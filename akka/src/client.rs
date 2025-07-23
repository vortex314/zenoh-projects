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
};
use tokio::
    net::UdpSocket
;

use crate::multicast;
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
pub enum ClientCmd {
    AddListener(Recipient<ClientEvent>),
    ReceivedUdpInternal(Value),
    ReceivedMcInternal(Value),
    SendUdp(Value), // unnecessary
    SendMc(Value), //..
    Reconnect,
    Publish(Value),
    Subscribe { dst:Option<String>,src:Option<String> }, // Subscribe to a specific destination
}

#[derive(Debug, Message, Clone)]
#[rtype(result = "()")]
pub enum ClientEvent {
    ReceivedMc(Value), //..
    ReceivedUdp(Value), //..
    Publish(Value),
}

pub struct ClientActor {
    object_name: String,
    multicast_ip: String,
    multicast_port: u16,
    udp_ip: String,
    udp_port: u16,
    broker_ip: Option<String>,
    broker_port: Option<u16>,
    udp_socket: Option<Arc<UdpSocket>>,
    multicast_socket: Option<Arc<UdpSocket>>,
    broker_addr : Option<SocketAddr>,
    listeners: Vec<Recipient<ClientEvent>>,
}

impl ClientActor {
    pub fn new(
        object_name: String,
        multicast_ip: &str,
        multicast_port: u16,
        udp_ip: &str,
        udp_port: u16,
    ) -> ClientActor {
        ClientActor {
            object_name,
            multicast_ip: multicast_ip.to_string(),
            multicast_port,
            udp_ip: udp_ip.to_string(),
            udp_port,
            broker_ip: None,
            broker_port: None,
            udp_socket: None,
            multicast_socket: None,
            broker_addr:None,
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

    pub fn emit(&self, event: ClientEvent) {
        for listener in &self.listeners {
            listener.do_send(event.clone());
        }
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

        let udp_receiver = async move  {
            let mut buf = [0; 1024];
            loop {
                let res = udp_receiver_socket.recv_from(&mut buf).await;
                if let Ok((size, socket_addr)) = res {
                    if size > 0 {
                        Value::from_json(&String::from_utf8_lossy(&buf[..size]))
                            .iter()
                            .for_each( |v: &Value| {
                                info!("Received UDP value: {} from {}", v.to_json(),socket_addr);
                                self_ref2
                                    .do_send(ClientCmd::ReceivedUdpInternal(v.clone()));
                            });
                    }
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
                    if size > 0 {
                        Value::from_json(&String::from_utf8_lossy(&buf[..size]))
                            .iter()
                            .for_each(
                                 | value:&Value | {
                                    info!("MC {} => {}", socket_addr,value.to_json());
                                    self_ref
                                        .do_send(ClientCmd::ReceivedMcInternal(value.clone()));
                                },
                            );
                    }
                } else {
                    error!("Multicast receive error: {}", r.unwrap_err());
                    break;
                }
            }
        };
        let success = Arbiter::current().spawn(udp_receiver);
        let success_mc = Arbiter::current().spawn(mc_receiver);

        info!("Spawn returned {}", success);
    }
}

impl Handler<ClientCmd> for ClientActor {
    // type Result = Result<(),dyn std::error::Error +'static >;
    type Result = ();

    fn handle(&mut self, msg: ClientCmd, _: &mut Self::Context) {
        match msg {
            ClientCmd::ReceivedMcInternal(value) => {
                let dev_type = value["type"].as_::<String>();
                let broker_ip = value["ip"].as_::<String>();
                let broker_port = value["port"].as_::<i64>();
                if dev_type.is_some() && broker_ip.is_some() && broker_port.is_some() {
                    if dev_type.unwrap() =="broker" {
                        info!("Received broker announcement: {}:{}", broker_ip.unwrap(), broker_port.unwrap());
                        self.broker_addr = Some( SocketAddr::from_str(&format!("{}:{}", broker_ip.unwrap(), broker_port.unwrap())).unwrap());
                    }
                }
            }
            ClientCmd::ReceivedUdpInternal(v) => {
                debug!(
                    "UDP received internal {} for {} listeners",
                    v.to_json(),
                    self.listeners.len()
                );
                // broadcast
                self.emit(ClientEvent::ReceivedUdp(v.clone()));
            }
            ClientCmd::Publish(_v) => {
                info!("MC send {}", _v.to_json());
                let buf = _v.to_json().into_bytes();
                self.udp_socket.as_mut().map(|s| s.send_to(&buf, self.broker_addr.unwrap()));
            }
            ClientCmd::AddListener(ar) => {
                info!("Adding listener: {:?}", ar);
                self.listeners.push(ar.clone())
            }
            _ => {}
        }
    }
}
