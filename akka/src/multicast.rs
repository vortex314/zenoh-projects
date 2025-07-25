use anyhow::Result;
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
pub enum McCmd {
    AddListener(Recipient<McEvent>),
    ReceivedInternal(Value),
    SendValue(Value),
    Reconnect,
}

#[derive(Debug, Message, Clone)]
#[rtype(result = "()")]
pub enum McEvent {
    ReceivedValue(Value),
}


pub struct McActor {
    multicast_ip: String,
    multicast_port: u16,
    udp_socket: Option<Arc<UdpSocket>>,
    listeners: Vec<Recipient<McEvent>>,
}

impl McActor {
    pub fn new(multicast_ip: &str, multicast_port: u16) -> McActor {
        McActor {
            multicast_ip: multicast_ip.to_string(),
            multicast_port,
            udp_socket: None,
            listeners: Vec::new(),
        }
    }
    pub fn create_multicast_socket(
        multicast_ip: &str,
        multicast_port: u16,
    ) -> Result<Arc<UdpSocket>> {
        let addr = SocketAddr::from_str(&format!("{}:{}", multicast_ip, multicast_port))?;
        let socket = Socket::new(Domain::IPV4, Type::DGRAM, Some(Protocol::UDP))?;
        socket.set_reuse_address(true)?;
        socket.set_reuse_port(true)?;
        socket.set_nonblocking(true)?;
        socket.bind(&SockAddr::from(addr))?;

        let udp_socket = UdpSocket::from_std(socket.into())?;
        udp_socket.join_multicast_v4(
            Ipv4Addr::from_str(multicast_ip)?,
            Ipv4Addr::from_str(INTERFACE_ALL)?,
        )?;

        Ok(Arc::new(udp_socket))
    }

    pub fn emit(&self, event: McEvent) {
        for listener in &self.listeners {
            listener.do_send(event.clone());
        }
    }
}

impl Actor for McActor {
    type Context = Context<Self>;

    fn started(&mut self, ctx: &mut Context<Self>) {
        info!(" pre_start ");
        let udp_socket = Self::create_multicast_socket(&self.multicast_ip, self.multicast_port)
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
                        self_ref.send(McCmd::ReceivedInternal(value.unwrap())).await.expect("Failed to send ReceivedInternal message");


                    },
                    Err(e) => {
                        error!("UDP receive error: {}", e);
                        self_ref.send(McCmd::Reconnect).await.expect("Failed to send Reconnect message");
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

impl Handler<McCmd> for McActor {
    // type Result = Result<(),dyn std::error::Error +'static >;
    type Result = ();

    fn handle(&mut self, msg: McCmd, _: &mut Self::Context) {
        match msg {
            McCmd::ReceivedInternal(v) => {
                debug!(
                    "UDP received internal {} for {} listeners",
                    v.to_json(),
                    self.listeners.len()
                );
                // broadcast
                self.emit(McEvent::ReceivedValue(v.clone()));
            }
            McCmd::SendValue(_v) => {
                info!("MC send {}", _v.to_json());
                let buf = _v.to_json().into_bytes();
                self.udp_socket.as_mut().map(|s| s.send(&buf));
            }
            McCmd::AddListener(ar) => {
                info!("Adding listener: {:?}", ar);
                self.listeners.push(ar.clone())
            }
            _ => {}
        }
    }
}
