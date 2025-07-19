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
/*const UDP_LISTEN_ADDR: &str = "0.0.0.0:6502";
const UDP_LISTEN_PORT: u16 = 6502;
const UDP_MULTICAST_ADDR: &str = "225.0.0.1";
const UDP_SEND_ADDR: &str = "225.0.0.1:6502"; // Example multicast address
const HTTP_SERVER_ADDR: &str = "0.0.0.0:3000";*/
const MAX_CACHED_MESSAGES: usize = 100;

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    let x = Ipv4Addr::from_str(ip4_str)?;
    Ok(x)
}
pub trait CommonMessage {}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum BrokerCmd {
    AddListener(Recipient<BrokerEvent>),
    ReceivedInternal(Value),
    Send(Value),
    Reconnect,
    Subscribe {
        pattern: Pattern,
        endpoint: Endpoint,
    },
}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum BrokerEvent {
    Received(Value),
    ConnectionLost,
    ConnectionEstablished,
}

#[derive(Debug)]
struct Endpoint {
    ip: Ipv4Addr,
    port: u16,
    object: String,
}

#[derive(Eq, Hash, PartialEq,Debug)]
struct Pattern {
    src_pattern: Option<String>,
    dst_pattern: Option<String>,
    verb_pattern: Option<String>,
}

impl Pattern {
    fn new() -> Self {
        Pattern {
            src_pattern: None,
            dst_pattern: None,
            verb_pattern: None,
        }
    }
    fn set_src_pattern(&mut self, pattern: String) {
        self.src_pattern = Some(pattern);
    }
    fn set_dst_pattern(&mut self, pattern: String) {
        self.dst_pattern = Some(pattern);
    }
    fn set_verb_pattern(&mut self, pattern: String) {
        self.verb_pattern = Some(pattern);
    }

    fn matches(&self, value: &Value) -> bool {
        let src_matches = match &self.src_pattern {
            Some(pattern) => {
                let mut found = false;
                value["src"].handle::<String, _>(|src| {
                    if src.contains(pattern) {
                        found = true;
                    }
                });
                found
            }
            None => true,
        };
        let dst_matches = match &self.dst_pattern {
            Some(pattern) => {
                let mut found = false;
                value["dst"].handle::<String, _>(|dst| {
                    if dst.contains(pattern) {
                        found = true;
                    }
                });
                found
            }
            None => true,
        };
        let verb_matches = match &self.verb_pattern {
            Some(pattern) => {
                let mut found = false;
                value["verb"].handle::<String, _>(|verb| {
                    if verb.contains(pattern) {
                        found = true;
                    }
                });
                found
            }
            None => true,
        };
        src_matches && dst_matches && verb_matches
    }
}

impl CommonMessage for BrokerCmd {}

pub struct BrokerActor {
    udp_port: u16,
    udp_socket: Option<Arc<UdpSocket>>,
    listeners: Vec<Recipient<BrokerEvent>>,
    subscribers: Arc<Mutex<HashMap<Pattern, Vec<Endpoint>>>>, // changed
}

impl BrokerActor {
    pub fn new(port: u16) -> BrokerActor {
        BrokerActor {
            udp_port: port,
            udp_socket: None,
            listeners: Vec::new(),
            subscribers: Arc::new(Mutex::new(HashMap::new())), // changed
        }
    }

        fn add_subscriber(&mut self , pattern: Pattern, endpoint: Endpoint) {
        let mut subs = self.subscribers.lock().unwrap(); // lock for access
        subs.entry(pattern).or_insert_with(Vec::new).push(endpoint);
    }
}

impl Actor for BrokerActor {
    type Context = Context<Self>;



    fn started(&mut self, ctx: &mut Context<Self>) {
        info!(" pre_start ");
        let udp_listen_addr = format!("0.0.0.0:{}", self.udp_port);
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

        // reuse addr to allow multiple instances to bind to the same port

        let udp_socket = Arc::new(udp_socket);
        // Clone for UDP receiver task
        let udp_receiver_socket = udp_socket.clone();

        self.udp_socket = Some(udp_socket);

        let self_ref = ctx.address();
        let subscribers = self.subscribers.clone(); // clone Arc for async move
        let mut pattern = Pattern::new();
        pattern.set_src_pattern("pclenovo/tester".to_string());
        let mut endpoint = Endpoint {
            ip: str_to_ip4_addr("192.168.0.148").unwrap(),
            port: 6504,
            object: "pclenovo/tester".to_string(),
        };
        {
            let mut subs = subscribers.try_lock().unwrap(); // lock for access
            subs.insert(pattern, endpoint);
            if subs.find(&pattern).is_none() {
                error!("Pattern not found in subscribers");
            } else {
                info!("Pattern found in subscribers");
            }
        }

        let receiver = async move {
            let mut buf = [0; 1024];
            loop {
                select! {
                    r = udp_receiver_socket.recv_from(&mut buf) => {
                        match r {
                            Ok((len, src)) => {
                                let message = String::from_utf8_lossy(&buf[..len]);
                                info!("BRK recv {} => {}", src, message);

                                let value = Value::from_json(&message[..]).unwrap();
                                let subs = subscribers.lock().await; // lock for access
                                for (pattern, endpoint) in subs.iter() {
                                    if pattern.matches(&value) {
                                        info!("Matched pattern: {:?}", &pattern);
                                        info!("Sending to endpoint: {:?}", &endpoint);
                                        udp_receiver_socket.send_to(
                                            value.to_json().as_bytes(),
                                            SocketAddr::new(endpoint.ip.into(), endpoint.port),
                                        ).await.expect("Failed to send UDP message");
                                    }
                                }
                            },
                    Err(e) => {
                        error!("UDP receive error: {}", e);
                        self_ref.send(BrokerCmd::Reconnect).await;
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

impl Handler<BrokerCmd> for BrokerActor {
    // type Result = Result<(),dyn std::error::Error +'static >;
    type Result = ();

    fn handle(&mut self, msg: BrokerCmd, _: &mut Self::Context) {
        match msg {
            BrokerCmd::ReceivedInternal(v) => {
                debug!(
                    "UDP received internal {} for {} listeners",
                    v.to_json(),
                    self.listeners.len()
                );
                // broadcast
                self.listeners.iter().for_each(|l| {
                    debug!("Sending to listener: {:?}", l);
                    l.do_send(BrokerEvent::Received(v.clone()));
                });
            }
            BrokerCmd::Send(_v) => {
                info!("UDP send {}", _v.to_json());
                let buf = _v.to_json().into_bytes();
                self.udp_socket.as_mut().map(|s| s.send(&buf));
            }
            BrokerCmd::AddListener(ar) => {
                info!("Adding listener: {:?}", ar);
                self.listeners.push(ar.clone())
            }
            BrokerCmd::Subscribe { pattern, endpoint } => {
                info!("Adding subscriber: {:?} to endpoint: {:?}", pattern, endpoint);
                let mut subs = self.subscribers.lock().unwrap(); // lock for access
                subs.insert(pattern, endpoint);
            }
            _ => {}
        }
    }
}
