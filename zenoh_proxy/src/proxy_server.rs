
use bytes::BytesMut;
use log::*;
use std::io;
use std::io::Write;
use std::result::Result;
use std::collections::BTreeMap;

use tokio::select;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};
use minicbor::encode;

use crate::protocol;
use crate::SourceTrait;
use crate::protocol::decode_frame;
use crate::protocol::encode_frame;
use protocol::msg::ProxyMessage;
use protocol::MTU_SIZE;
use protocol::MessageDecoder;
use crate::limero::Sink;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::limero::SinkRef;

use crate::transport::*;

use zenoh::prelude::r#async::*;


const GREEN : &str = "\x1b[0;32m";
const RESET : &str = "\x1b[m";

#[derive(Clone)]
pub enum ProxyServerEvent {
    Connected ,
    Disconnected,
    Publish { topic_id: u16, message: Vec<u8> },
}

#[derive(Clone) ]
pub enum ProxyServerCmd {
    Connect { protocol_id: u8, duration: u16, client_id: String },
    WillTopic { topic: String },
    WillMsg { message: Vec<u8> },
    
    Disconnect,
}

struct TopicId {
    id: u16,
    name: String,
    acked: bool,
}

pub struct ProxyServer {
    port_info: SerialPortInfo,
    events: Source<ProxyServerEvent>,
    commands: Sink<ProxyServerCmd>,
    transport_cmd: SinkRef<TransportCmd>,
    transport_event:Sink<TransportEvent>,
    zenoh_session: Option<zenoh::Session>,
    client_topics : BTreeMap <u16,TopicId>,
    server_topics : BTreeMap <u16,TopicId>,
}

impl ProxyServer {
    pub fn new(port_info: SerialPortInfo, transport: SinkRef<TransportCmd>) -> Self {
        let commands = Sink::new(100);
        let events = Source::new();
        ProxyServer {
            port_info,
            events,
            commands,
            transport_cmd: transport,
            transport_event: Sink::new(100),
            zenoh_session: None,
            client_topics: Vec::new(),
            server_topics: Vec::new(),
        }
    }

    pub async fn run(&mut self) {
        self.zenoh_session = Some(zenoh::open(config::default()).res().await.unwrap());
        let mut zenoh_subscriber = Some(self.zenoh_session.as_ref().unwrap().declare_subscriber("/esp32/*").res().await.unwrap());
        let mut buf = vec![0u8; MTU_SIZE];
        loop {
            select! {
                msg = self.zenoh_subscriber.as_ref().unwrap().recv_async() => {
                    match msg {
                        Some((topic, payload)) => {
                            let topic_id = 0;
                            let message = payload.to_vec();
                            let publish = ProxyMessage::Publish { topic_id, message };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: publish });
                        },
                        None => {
                            break;
                        },
                    }
                },
                cmd = self.commands.read() => {
                    match cmd {
                        Some(ProxyServerCmd::Connect { protocol_id, duration, client_id }) => {
                            let connect = ProxyMessage::Connect { protocol_id, duration, client_id };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: connect });
                        },
                        Some(ProxyServerCmd::WillTopic { topic }) => {
                            let will_topic = ProxyMessage::WillTopic { topic };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: will_topic });
                        },
                        Some(ProxyServerCmd::WillMsg { message }) => {
                            let will_msg = ProxyMessage::WillMsg { message };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: will_msg });
                        },
                        Some(ProxyServerCmd::Register { topic_id, topic_name }) => {
                            let register = ProxyMessage::Register { topic_id, topic_name };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: register });
                        },
                        Some(ProxyServerCmd::Subscribe { topic, qos }) => {
                            let subscribe = ProxyMessage::Subscribe { topic, qos };
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: subscribe });
                        },
                        Some(ProxyServerCmd::Disconnect) => {
                            let disconnect = ProxyMessage::Disconnect;
                            self.transport_cmd.push(TransportCmd::SendMessage { msg: disconnect });
                        },
                        None => {
                            break;
                        },
                    }
                },
                event = self.transport_event.read() => {
                    match event {
                        Some(TransportEvent::RecvMessage { msg }) => {
                            match msg {
                                ProxyMessage::Connect { .. } => {
                                    self.zenoh_session = Some(zenoh::open(config::default()).res().await.unwrap());
                                },
                                ProxyMessage::ConnAck { return_code } => {
                                    if return_code == 0 {
                                        self.events.emit(ProxyServerEvent::Connected);
                                    } else {
                                        self.events.emit(ProxyServerEvent::Disconnected);
                                    }
                                },
                                ProxyMessage::Publish { topic_id, message } => {
                                    self.events.emit(ProxyServerEvent::Publish { topic_id, message });
                                    self.zenoh_session.as_ref().unwrap().write(&topic_id.to_string(), &message).await.unwrap();
                                },
                                ProxyMessage::Disconnect => {
                                    self.zenoh_session.as_ref().unwrap().close();
                                    self.zenoh_session = None;
                                    self.events.emit(ProxyServerEvent::Disconnected);
                                },
                                ProxyMessage::Subscribe { topic , qos } => {
                                    let _ = self.zenoh_session.as_ref().unwrap().declare(&topic, whatami::PUBLISHER).await;
                                    let subscriber = self.zenoh_session.declare_subscriber(topic).res().await.unwrap();

                                },
                                _ => {
                                    // Ignore
                                },
                            }
                        },
                        None => {
                            break;
                        },
                    }
                },
            }
        }
    }
}

impl SinkTrait<ProxyServerCmd> for ProxyServer {
    fn push(&self, message: ProxyServerCmd) {
        self.commands.push(message);
    }
}

impl SourceTrait<ProxyServerEvent> for ProxyServer {
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<ProxyServerEvent>>) {
        self.events.subscribe(sink);
    }
}

