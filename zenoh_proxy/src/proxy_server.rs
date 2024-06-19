use crate::protocol;
use crate::SourceTrait;
use protocol::decode_frame;
use protocol::encode_frame;
use protocol::msg::ProxyMessage;
use protocol::MTU_SIZE;
use protocol::MessageDecoder;
use bytes::BytesMut;
use log::*;
use minicbor::encode;
use std::io;
use std::io::Write;
use std::result::Result;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};

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

struct ProxyServer {
    port_info: SerialPortInfo,
    events: Source<ProxyServerEvent>,
    commands: Sink<ProxyServerCmd>,
    transport: SinkRef<TransportCmd>,
}

impl ProxyServer {
    pub fn new(port_info: SerialPortInfo, transport: SinkRef<TransportCmd>) -> Self {
        let commands = Sink::new(100);
        let events = Source::new();
        ProxyServer {
            port_info,
            events,
            commands,
            transport,
        }
    }

    pub async fn run(&mut self) {
        loop {
            select! {
                cmd = self.commands.recv() => {
                    match cmd {
                        Some(ProxyServerCmd::Connect { protocol_id, duration, client_id }) => {
                            let connect = ProxyMessage::Connect { protocol_id, duration, client_id };
                            self.transport.send(TransportCmd::SendMessage { frame: connect }).await;
                        },
                        Some(ProxyServerCmd::WillTopic { topic }) => {
                            let will_topic = ProxyMessage::WillTopic { topic };
                            self.transport.send(TransportCmd::SendMessage { frame: will_topic }).await;
                        },
                        Some(ProxyServerCmd::WillMsg { message }) => {
                            let will_msg = ProxyMessage::WillMsg { message };
                            self.transport.send(TransportCmd::SendMessage { frame: will_msg }).await;
                        },
                        Some(ProxyServerCmd::Register { topic_id, topic_name }) => {
                            let register = ProxyMessage::Register { topic_id, topic_name };
                            self.transport.send(TransportCmd::SendMessage { frame: register }).await;
                        },
                        Some(ProxyServerCmd::Subscribe { topic, qos }) => {
                            let subscribe = ProxyMessage::Subscribe { topic, qos };
                            self.transport.send(TransportCmd::SendMessage { frame: subscribe }).await;
                        },
                        Some(ProxyServerCmd::Disconnect) => {
                            let disconnect = ProxyMessage::Disconnect;
                            self.transport.send(TransportCmd::SendMessage { frame: disconnect }).await;
                        },
                        None => {
                            break;
                        },
                    }
                },
                event = self.transport.recv() => {
                    match event {
                        Some(TransportEvent::RecvMessage { frame }) => {
                            match frame {
                                ProxyMessage::ConnAck { return_code } => {
                                    self.events.emit(ProxyServerEvent::Connected);
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

