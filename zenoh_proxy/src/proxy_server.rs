
use bytes::BytesMut;
use log::*;
use std::io;
use std::io::Write;
use std::result::Result;

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
    Register { topic_id: u16, topic_name: String },
    Subscribe { topic: String, qos: u8 },
    Disconnect,
}

pub struct ProxyServer {
    port_info: SerialPortInfo,
    events: Source<ProxyServerEvent>,
    commands: Sink<ProxyServerCmd>,
    transport_cmd: SinkRef<TransportCmd>,
    transport_event:Sink<TransportEvent>,
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
        }
    }

    pub async fn run(&mut self) {
        loop {
            select! {
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
                                ProxyMessage::ConnAck { return_code } => {
                                    if return_code == 0 {
                                        self.events.emit(ProxyServerEvent::Connected);
                                    } else {
                                        self.events.emit(ProxyServerEvent::Disconnected);
                                    }
                                },
                                ProxyMessage::Publish { topic_id, message } => {
                                    self.events.emit(ProxyServerEvent::Publish { topic_id, message });
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
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<ProxyServerEvent>>) {
        self.events.add_listener(sink);
    }
}

