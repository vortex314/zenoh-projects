use anyhow::Result;
use log::debug;
use log::error;
use log::{info, logger};

use serde::{Deserialize, Serialize};
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
    sync::{mpsc, Mutex},
    time::{self, Instant},
};

use crate::actor::Actor;
use crate::actor::ActorContext;
use crate::actor::ActorRef;
use crate::value::Value;
// Configuration
const UDP_LISTEN_ADDR: &str = "0.0.0.0:6502";
const UDP_LISTEN_PORT: u16 = 6502;
const UDP_MULTICAST_ADDR: &str = "225.0.0.1";
const UDP_SEND_ADDR: &str = "225.0.0.1:6502"; // Example multicast address
const HTTP_SERVER_ADDR: &str = "0.0.0.0:3000";
const MAX_CACHED_MESSAGES: usize = 100;

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    let x = Ipv4Addr::from_str(ip4_str)?;
    Ok(x)
}
pub trait CommonMessage {}

#[derive(Debug)]
pub enum McMessage {
    AddListener(ActorRef),
    Received(Value),
    ReceivedInternal(Value),
    Send(Value),
    ConnectionLost,
    ConnectionEstablished,
}

impl CommonMessage for McMessage {}

pub struct McActor {
    udp_socket: Arc<UdpSocket>,
    listeners: Vec<ActorRef>,
}

impl McActor {
    pub fn new() -> McActor {
        McActor {
            udp_socket: None,
            listeners: Vec::new(),
        }
    }
}

#[async_trait::async_trait]
impl Actor for McActor {
    async fn receive(&mut self, msg: Box<dyn Any + Send>, _ctx: &ActorContext) {
        if let Some(m) = msg.downcast_ref::<McMessage>() {
            match m {
                McMessage::Received(_v) => {} // only as output
                McMessage::ReceivedInternal(v) => {
                    // broadcast
                    self.listeners
                        .iter()
                        .map(|l| l.tell(McMessage::Received(v.clone())))
                        .map(async |f| f.await);
                }
                McMessage::Send(_v) => {
                    info!("UDP send {}", _v.to_json());
                    let buf = _v.to_json().into_bytes();
                    self.udp_socket.send(&buf);
                }
                McMessage::AddListener(ar) => self.listeners.push(ar.clone()),
                _ => {}
            }
        }
    }

    async fn post_stop(&mut self, _ctx: &ActorContext) {}

    // and (optionally) internal state

    async fn pre_start(&mut self, _ctx: &ActorContext) {
        info!(" pre_start ");
        let udp_socket = UdpSocket::bind(UDP_LISTEN_ADDR)
            .await
            .expect("Failed to create std UDP socket");
        udp_socket
            .join_multicast_v4(
                Ipv4Addr::from_str("225.0.0.1").unwrap(),
                Ipv4Addr::from_str("0.0.0.0").unwrap(),
            )
            .expect("Failed to join multicast group");
        // reuse addr to allow multiple instances to bind to the same port

        let udp_socket = Arc::new(udp_socket);
        // Clone for UDP receiver task
        let udp_receiver_socket = udp_socket.clone();

        self.udp_socket=udp_socket;

        let self_ref = _ctx.self_ref.clone();

        tokio::spawn(async move {
            let mut buf = [0; 1024];
            loop {
                info!("UDP recv...");
                select! {
                    r = udp_receiver_socket.recv_from(&mut buf) => {
                        match r {
                    Ok((len, src)) => {
                         let message = String::from_utf8_lossy(&buf[..len]);
                            info!("Received UDP from {}: {}", src, message);

                        let value = Value::from_json(&message[..]);
                        self_ref.tell(McMessage::ReceivedInternal(value.unwrap())).await;


                    },
                    Err(e) => {
                        error!("UDP receive error: {}", e);
                        self_ref.tell(McMessage::ConnectionLost).await;
                    },
                }
                    },
                    _ = time::sleep(Duration::from_secs(1)) => {
                        info!("timeout.");
                    }
                };
            }
        });
    }
}
