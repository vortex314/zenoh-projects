use anyhow::Result;
use futures::executor::block_on;
use log::debug;
use log::error;
use log::info;

use socket2::Domain;
use socket2::Protocol;
use socket2::SockAddr;
use socket2::Socket;
use socket2::Type;
use std::str::FromStr;
use std::{
    net::{Ipv4Addr, SocketAddr},
    sync::Arc,
    time::Duration,
};
use tokio::{
    net::UdpSocket,
    select,
    time::{self},
};

use crate::value::Value;
use actix::prelude::*;
// Configuration
const INTERFACE_ALL: &str = "0.0.0.0";

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    let x = Ipv4Addr::from_str(ip4_str)?;
    Ok(x)
}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum UdpCmd {
    AddListener(Recipient<UdpEvent>),
    ReceivedInternal(Value),
    SendValue {
        socket_addr: SocketAddr,
        value: Value,
    },
    Reconnect,
}

#[derive(Debug, Message, Clone)]
#[rtype(result = "()")]
pub enum UdpEvent {
    ReceivedValue(Value),
}

pub struct UdpActor {
    udp_ip: String,
    udp_port: u16,
    udp_socket: Option<Arc<UdpSocket>>,
    listeners: Vec<Recipient<UdpEvent>>,
}

impl UdpActor {
    pub fn new(udp_ip: &str, udp_port: u16) -> UdpActor {
        UdpActor {
            udp_ip: udp_ip.to_string(),
            udp_port,
            udp_socket: None,
            listeners: Vec::new(),
        }
    }
    pub fn create_udp_socket(udp_ip: &str, udp_port: u16) -> Result<Arc<UdpSocket>> {
        let addr = SocketAddr::from_str(&format!("{}:{}", udp_ip, udp_port))?;
        let socket = Socket::new(Domain::IPV4, Type::DGRAM, Some(Protocol::UDP))?;
        socket.set_reuse_address(true)?;
        socket.set_reuse_port(true)?;
        socket.set_nonblocking(true)?;
        socket.bind(&SockAddr::from(addr))?;

        let udp_socket = UdpSocket::from_std(socket.into())?;

        Ok(Arc::new(udp_socket))
    }

    pub fn emit(&self, event: UdpEvent) {
        for listener in &self.listeners {
            listener.do_send(event.clone());
        }
    }
}

impl Actor for UdpActor {
    type Context = Context<Self>;

    fn started(&mut self, ctx: &mut Context<Self>) {
        info!(" pre_start ");
        let udp_socket = Self::create_udp_socket(&self.udp_ip, self.udp_port)
            .expect("Failed to create multicast socket");

        // Clone for UDP receiver task
        let udp_receiver_socket = udp_socket.clone();

        self.udp_socket = Some(udp_socket);

        let self_ref = ctx.address();

        let receiver = async move {
            let mut buf = [0; 1024];
            loop {
                select! {
                    r = udp_receiver_socket.recv_from(&mut buf) => {
                        match r {
                    Ok((len, src)) => {
                         let message = String::from_utf8_lossy(&buf[..len]);
                            info!("MC recv {} => {}", src, message);

                        let value = Value::from_json(&message[..]);
                        self_ref.send(UdpCmd::ReceivedInternal(value.unwrap())).await.expect("Failed to send ReceivedInternal message");


                    },
                    Err(e) => {
                        error!("UDP receive error: {}", e);
                        self_ref.send(UdpCmd::Reconnect).await.expect("Failed to send Reconnect message");
                    },
                }
                    },
                    _ = time::sleep(Duration::from_secs(100)) => {
                        info!("timeout.");
                    }
                };
            }
        };
        let success = Arbiter::current().spawn(receiver);
        info!("Spawn returned {}", success);
    }
}

impl Handler<UdpCmd> for UdpActor {
    // type Result = Result<(),dyn std::error::Error +'static >;
    type Result = ();

    fn handle(&mut self, msg: UdpCmd, _: &mut Self::Context) {
        match msg {
            UdpCmd::ReceivedInternal(v) => {
                debug!(
                    "UDP received internal {} for {} listeners",
                    v.to_json(),
                    self.listeners.len()
                );
                // broadcast
                self.emit(UdpEvent::ReceivedValue(v.clone()));
            }
            UdpCmd::SendValue { socket_addr, value } => {
                info!("MC send {}", value.to_json());
                let buf = value.to_json().into_bytes();
                self.udp_socket
                    .as_mut()
                    .map(|s| {
                        let _ = s.try_send_to(&buf, socket_addr);
                    })
                    .expect("Failed to send UDP message");
            }
            UdpCmd::AddListener(ar) => {
                info!("Adding listener: {:?}", ar);
                self.listeners.push(ar.clone())
            }
            _ => {}
        }
    }
}
