use anyhow::Result;
use futures::executor::block_on;
use log::debug;
use log::error;
use log::{info, logger};

use futures::future::join_all;
use serde::{Deserialize, Serialize};
use socket2::Domain;
use socket2::Protocol;
use socket2::SockAddr;
use socket2::Socket;
use socket2::Type;
use std::any::Any;
use std::str::FromStr;
use std::{
    collections::{HashMap, VecDeque},
    error::Error,
    net::{Ipv4Addr, SocketAddr, SocketAddrV4},
    sync::Arc,
    time::Duration,
};
use tokio::{
    net::UdpSocket,
    select,
    sync::{Mutex, mpsc},
    time::{self, Instant},
};

use crate::value::Value;
use actix::prelude::*;
// Configuration
const UDP_LISTEN_ADDR: &str = "0.0.0.0:6502";

const MAX_CACHED_MESSAGES: usize = 100;
const INTERFACE_ALL: &str = "0.0.0.0";

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    let x = Ipv4Addr::from_str(ip4_str)?;
    Ok(x)
}
pub trait CommonMessage {}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum McCmd {
    AddListener(Recipient<McEvent>),
    ReceivedInternal(Value),
    Send(Value),
    Reconnect,
}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum McEvent {
    Received(Value),
    ConnectionLost,
    ConnectionEstablished,
}

impl CommonMessage for McCmd {}

pub struct McActor {
    multicast_ip: String,
    multicast_port: u16,
    udp_socket: Option<Arc<UdpSocket>>,
    listeners: Vec<Recipient<McEvent>>,
}

impl McActor {
    pub fn new(multicast_ip:&str,multicast_port:u16) -> McActor {
        McActor {
            multicast_ip: multicast_ip.to_string(),
            multicast_port,
            udp_socket: None,
            listeners: Vec::new(),
        }
    }
}

impl Actor for McActor {
    type Context = Context<Self>;

    fn started(&mut self, ctx: &mut Context<Self>) {
        info!(" pre_start ");
        let udp_listen_addr = format!("0.0.0.0:{}", self.multicast_port);
        let addr = SocketAddr::from_str(&udp_listen_addr).unwrap();

        // Create a UDP socket with socket2
        let socket = Socket::new(Domain::IPV4, Type::DGRAM, Some(Protocol::UDP))
            .expect("Failed to create UDP socket");
        socket.set_reuse_address(true); // tokio cannot do this, so we do it here
        socket.set_reuse_port(true);
        socket
            .set_nonblocking(true)
            .expect("Failed to set non-blocking"); // needed folet udp_socketr tokio

        socket.bind(&SockAddr::from(addr));

        let udp_socket =
            UdpSocket::from_std(socket.into()).expect("Failed to create UdpSocket from socket2");

        udp_socket
            .join_multicast_v4(
                Ipv4Addr::from_str(&self.multicast_ip).unwrap(),
                Ipv4Addr::from_str(INTERFACE_ALL).unwrap(),
            )
            .expect("Failed to join multicast group");
        // reuse addr to allow multiple instances to bind to the same port

        let udp_socket = Arc::new(udp_socket);
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
                        self_ref.send(McCmd::ReceivedInternal(value.unwrap())).await;


                    },
                    Err(e) => {
                        error!("UDP receive error: {}", e);
                        self_ref.send(McCmd::Reconnect).await;
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
                self.listeners.iter().for_each(|l| {
                    debug!("Sending to listener: {:?}", l);
                    l.do_send(McEvent::Received(v.clone()));
                });
            }
            McCmd::Send(_v) => {
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
