use anyhow::Result;
use indexmap::IndexMap;
use log::debug;
use log::error;
use log::info;

use socket2::Domain;
use socket2::Protocol;
use socket2::SockAddr;
use socket2::Socket;
use socket2::Type;
use std::collections::HashSet;
use std::str::FromStr;
use std::{
    net::{Ipv4Addr, SocketAddr},
    sync::Arc,
};
use tokio::net::UdpSocket;
use tokio::time::Duration;

use crate::Value;
use actix::prelude::*;
// Configuration
const INTERFACE_ALL: &str = "0.0.0.0";

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    let x = Ipv4Addr::from_str(ip4_str)?;
    Ok(x)
}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum ClientCmd {
    AddListener {
        src: String,
        recipient: Recipient<ClientEvent>,
    },
    ReceivedUdpInternal {
        src: SocketAddr,
        value: Value,
    },
    ReceivedMcInternal {
        src: SocketAddr,
        value: Value,
    },
    SendUdp {
        dst: SocketAddr,
        value: Value,
    },
    SendMc {
        value: Value,
    }, //..
    Publish {
        src: String,
        value: Value,
    },
    PublishSubscriptions,
}

#[derive(Debug, Message, Clone)]
#[rtype(result = "()")]
pub enum ClientEvent {
    ReceivedMc(Value),  //..
    ReceivedUdp(Value), //..
    Publish(Value),
}

struct ObjectInfo {
    name: String,
    addr: SocketAddr,
    timestamp: std::time::SystemTime,
}

enum Destination {
    Local(Recipient<ClientEvent>),
    Remote(ObjectInfo),
}

#[derive(Debug, Clone, Hash)]
enum Pattern {
    Src(String),
    Dst(String),
}

impl Pattern {
    fn src(src: String) -> Self {
        Pattern::Src(src)
    }
    fn dst(dst: String) -> Self {
        Pattern::Dst(dst)
    }
    fn matches(&self, value: &Value) -> bool {
        match self {
            Pattern::Src(src) => value["src"] == Value::from(src.to_string()),
            Pattern::Dst(dst) => value["dst"] == Value::from(dst.to_string()),
        }
    }
    fn to_string(&self) -> String {
        match self {
            Pattern::Src(src) => format!("{}", src),
            Pattern::Dst(dst) => format!("{}", dst),
        }
    }
}

impl PartialEq for Pattern {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Pattern::Src(src1), Pattern::Src(src2)) => src1 == src2,
            (Pattern::Dst(dst1), Pattern::Dst(dst2)) => dst1 == dst2,
            _ => false,
        }
    }
}

impl Eq for Pattern {}

#[derive(Debug)]
pub struct Subscription {
    dst: HashSet<String>,
    expiration: std::time::SystemTime,
}

pub struct ClientActor {
    multicast_ip: String,
    multicast_port: u16,
    udp_ip: String,
    udp_port: u16,
    udp_socket: Option<Arc<UdpSocket>>,
    multicast_socket: Option<Arc<UdpSocket>>,
    destinations: IndexMap<String, Destination>,
    subscriptions: IndexMap<Pattern, Vec<String>>,
}

impl ClientActor {
    pub fn new(
        multicast_ip: &str,
        multicast_port: u16,
        udp_ip: &str,
        udp_port: u16,
    ) -> ClientActor {
        ClientActor {
            multicast_ip: multicast_ip.to_string(),
            multicast_port,
            udp_ip: udp_ip.to_string(),
            udp_port,

            udp_socket: None,
            multicast_socket: None,
            destinations: IndexMap::new(),
            subscriptions: IndexMap::new(),
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
}

impl Actor for ClientActor {
    type Context = Context<Self>;

    fn started(&mut self, ctx: &mut Context<Self>) {
        info!(" pre_start ");
        let multicast_socket =
            Self::create_multicast_socket(&self.multicast_ip, self.multicast_port)
                .expect("Failed to create multicast socket");
        let udp_socket = Self::create_udp_socket(&self.udp_ip, self.udp_port)
            .expect("Failed to create UDP socket");

        // Clone for UDP receiver task
        let udp_receiver_socket = udp_socket.clone();
        let multicast_receiver_socket = multicast_socket.clone();

        self.udp_socket = Some(udp_socket);
        self.multicast_socket = Some(multicast_socket);

        let self_ref = ctx.address();
        let self_ref2 = ctx.address();

        let udp_receiver = async move {
            let mut buf = [0; 1024];
            loop {
                let res = udp_receiver_socket.recv_from(&mut buf).await;
                if let Ok((size, socket_addr)) = res {
                    Value::from_json(&String::from_utf8_lossy(&buf[..size]))
                        .iter()
                        .for_each(|v: &Value| {
                            info!("Received UDP value: {} from {}", v.to_json(), socket_addr);
                            self_ref2.do_send(ClientCmd::ReceivedUdpInternal {
                                src: socket_addr,
                                value: v.clone(),
                            });
                        });
                } else {
                    error!("UDP receive error: {}", res.unwrap_err());
                    break;
                }
            }
        };
        let mc_receiver = async move {
            let mut buf = [0; 1024];
            loop {
                let r = multicast_receiver_socket.recv_from(&mut buf).await;
                if let Ok((size, socket_addr)) = r {
                    Value::from_json(&String::from_utf8_lossy(&buf[..size]))
                        .iter()
                        .for_each(|value: &Value| {
                            info!("MC {} => {}", socket_addr, value.to_json());
                            self_ref.do_send(ClientCmd::ReceivedMcInternal {
                                src: socket_addr,
                                value: value.clone(),
                            });
                        });
                } else {
                    error!("Multicast receive error: {}", r.unwrap_err());
                    break;
                }
            }
        };
        let subscriptions_sender = async move {
            loop {
                tokio::time::sleep(Duration::from_millis(1000)).await;
            }
        };
        Arbiter::current().spawn(udp_receiver);
        Arbiter::current().spawn(mc_receiver);
        Arbiter::current().spawn(subscriptions_sender);
    }
}

impl Handler<ClientCmd> for ClientActor {
    // type Result = Result<(),dyn std::error::Error +'static >;
    type Result = ();

    fn handle(&mut self, msg: ClientCmd, ctx: &mut Self::Context) {
        match msg {
            ClientCmd::ReceivedMcInternal {
                src: src_addr,
                value,
            } => {
                value["src"].handle::<String, _>(|_key| {
                    self.destinations.insert(
                        _key.clone(),
                        Destination::Remote(ObjectInfo {
                            name: _key.clone(),
                            addr: src_addr,
                            timestamp: std::time::SystemTime::now(),
                        }),
                    );
                });
            }
            ClientCmd::ReceivedUdpInternal { src, value } => {}

            ClientCmd::Publish { src, value } => {
                info!("Publishing value: {} from {}", value.to_json(), src);
                self.subscriptions
                    .iter()
                    .filter(|(pattern, _)| pattern.matches(&value))
                    .for_each(|(_, dsts)| {
                        for dst in dsts {
                            match self.destinations.get(dst) {
                                Some(Destination::Local(recipient)) => {
                                    recipient.do_send(ClientEvent::Publish(value.clone()));
                                }
                                Some(Destination::Remote(object_info)) => {
                                    let mut msg = Value::object();
                                    msg["src"] = src.clone().into();
                                    msg["pub"] = value.clone();
                                    ctx.address().do_send(ClientCmd::SendUdp {
                                        dst: object_info.addr,
                                        value: msg,
                                    });
                                }
                                None => {
                                    error!("No destination found for {}", dst);
                                }
                            }
                        }
                    });
            }

            ClientCmd::AddListener { src, recipient } => {
                let src1 = src.clone();
                let src2 = src.clone();
                self.destinations
                    .insert(src1, Destination::Local(recipient));
                self.subscriptions
                    .entry(Pattern::Dst(src2))
                    .or_default()
                    .push(src.clone());
            }
            ClientCmd::PublishSubscriptions => {
                for (pattern, dsts) in &self.subscriptions {
                    for dst in dsts {
                        if let Some(Destination::Remote(object_info)) = self.destinations.get(dst) {
                            let mut value = Value::object();
                            value["src"] = pattern.to_string().into();
                            value["dst"] = object_info.name.clone().into();
                            value["sub"] = Value::object();

                            ctx.address().do_send(ClientCmd::SendUdp {
                                dst: object_info.addr,
                                value,
                            });
                        }
                    }
                }
            }
            _ => {}
        }
    }
}
