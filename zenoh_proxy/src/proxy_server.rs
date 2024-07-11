use bytes::BytesMut;
use log::*;
use std::collections::BTreeMap;
use std::io;
use std::io::Write;
use std::result::Result;

use minicbor::encode;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio::select;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};

use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::protocol;
use crate::protocol::decode_frame;
use crate::protocol::encode_frame;
use crate::SourceTrait;
use protocol::msg::Flags;
use protocol::msg::ProxyMessage;
use protocol::msg::ReturnCode;
use protocol::MessageDecoder;
use protocol::MTU_SIZE;

use crate::transport::*;
use crate::zenoh_pubsub::*;

use zenoh::open;
use zenoh::prelude::r#async::*;
use zenoh::subscriber::Subscriber;
use zenoh::Session;

const GREEN: &str = "\x1b[0;32m";
const RESET: &str = "\x1b[m";

#[derive(Clone)]
pub enum ProxyServerEvent {
    Connected,
    Disconnected,
    Publish { topic: String, message: Vec<u8> },
}

#[derive(Clone, Debug)]
pub enum ProxyServerCmd {
    Publish { topic: String, message: Vec<u8> },
    TransportEvent(TransportEvent),
    PubSubEvent(PubSubEvent),
    Disconnect,
}

struct TopicId {
    id: u16,
    name: String,
    acked: bool,
}

pub struct ProxySession {
    events: Source<ProxyServerEvent>,
    cmds: Sink<ProxyServerCmd>,
    transport_cmd: SinkRef<TransportCmd>,
    pubsub_cmd: SinkRef<PubSubCmd>,
    client_topics: BTreeMap<u16, String>,
    server_topics: BTreeMap<String, u16>,
    will_topic: Option<String>,
    will_message: Option<Vec<u8>>,
    topic_id_counter: u16,
    client_id: Option<String>,
}

impl ProxySession {
    pub fn new(pubsub_cmd: SinkRef<PubSubCmd>, transport_cmd: SinkRef<TransportCmd>) -> Self {
        let commands = Sink::new(100);
        let events = Source::new();

        ProxySession {
            events,
            cmds: commands,
            transport_cmd,
            pubsub_cmd,
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            will_topic: None,
            will_message: None,
            topic_id_counter: 0,
            client_id: None,
        }
    }

    pub fn sink_ref(&self) -> SinkRef<ProxyServerCmd> {
        self.cmds.sink_ref()
    }

    pub fn transport_send(&self, message: ProxyMessage) {
        self.transport_cmd
            .push(TransportCmd::SendMessage { message });
    }

    fn get_server_topic_from_string(&mut self, topic: &str) -> u16 {
        match self.server_topics.get(topic) {
            Some(topic_id) => *topic_id,
            None => {
                self.topic_id_counter += 1;
                let topic_id = self.topic_id_counter;
                self.server_topics.insert(topic.to_string(),topic_id,);
                topic_id
            }
        }
    }

    pub async fn run(&mut self) {
        self.pubsub_cmd.push(PubSubCmd::Connect);

        let _buf = vec![0u8; MTU_SIZE];
        loop {
            select! {
                cmd = self.cmds.next() => {
                    let cmd = cmd.unwrap();
                    debug!("ProxyServerCmd:: {:?}", &cmd);
                    match cmd{
                        ProxyServerCmd::Publish { topic, message } => {
                            debug!("Publishing message to zenoh");
                            self.pubsub_cmd.push(PubSubCmd::Publish { topic, message });
                        },
                        ProxyServerCmd::TransportEvent(event) => {
                            debug!("Received event from transport {:?}", event);
                             self.on_transport_event(event).await;
                            }
                        ProxyServerCmd::Disconnect => {
                        }
                        ProxyServerCmd::PubSubEvent(event) => {
                            self.on_pubsub_event(event);
                        }
                    }
                },
            }
        }
    }

    fn on_pubsub_event(&mut self, event: PubSubEvent) {
        match event {
            PubSubEvent::Connected => {
                info!("Connected to zenoh");
                self.events.emit(ProxyServerEvent::Connected);
            }
            PubSubEvent::Disconnected => {
                info!("Disconnected from zenoh");
                self.events.emit(ProxyServerEvent::Disconnected);
            }
            PubSubEvent::Publish { topic, message } => {
                if !self.server_topics.contains_key(&topic) {
                    let topic_id = self.get_server_topic_from_string(&topic);
                    self.transport_send(ProxyMessage::Register {
                        topic_id,
                        msg_id: 0,
                        topic_name: topic.clone(),
                    });
                }
                let topic_id = self.get_server_topic_from_string(&topic);
                self.transport_send(ProxyMessage::Publish {
                    flags: Flags(0),
                    topic_id,
                    msg_id: 0,
                    data: message,
                });
            }
        }
    }

    async fn on_transport_event(&mut self, event: TransportEvent) {
        match event {
            TransportEvent::RecvMessage { message } => {
                self.on_transport_rxd(message).await;
            }
            TransportEvent::ConnectionLost {} => {
                info!("Connection lost");
                self.events.emit(ProxyServerEvent::Disconnected);
            }
        }
    }

    async fn on_transport_rxd(&mut self, event: ProxyMessage) {
        match event {
            ProxyMessage::Connect {
                flags: _,
                duration: _,
                client_id: _,
            } => {
                self.transport_send(ProxyMessage::ConnAck {
                    return_code: ReturnCode::Accepted,
                });
                self.client_topics.clear();
            }
            ProxyMessage::WillTopic { flags: _, topic } => {
                info!("Received WillTopic message");
                self.will_topic = Some(topic);
                self.transport_send(ProxyMessage::WillMsgReq {});
            }
            ProxyMessage::WillMsg { message } => {
                self.will_message = Some(message);
                info!("Received WillMsg message");
            }
            ProxyMessage::Publish {
                flags,
                topic_id,
                msg_id,
                data,
            } => match self.client_topics.get(&topic_id) {
                Some(_topic) => {
                    let topic = self.client_topics.get(&topic_id).unwrap().clone();
                    self.pubsub_cmd.push(PubSubCmd::Publish {
                        topic,
                        message: data,
                    });
                    if flags.qos() == 1 {
                        self.transport_send(ProxyMessage::PubAck {
                            topic_id,
                            msg_id,
                            return_code: ReturnCode::Accepted,
                        });
                    }
                }
                None => {
                    info!("Received Publish message for unknown topic");
                    self.transport_send(ProxyMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::InvalidTopicId,
                    });
                }
            },
            ProxyMessage::Subscribe {
                flags,
                msg_id,
                topic,
                qos: _,
            } => {
                self.pubsub_cmd.push(PubSubCmd::Subscribe { topic });
                self.transport_send(ProxyMessage::SubAck {
                    flags,
                    msg_id,
                    topic_id: 1,
                    return_code: ReturnCode::Accepted,
                });
            }
            ProxyMessage::Register {
                topic_id,
                msg_id,
                topic_name,
            } => {
                self.client_topics.insert(topic_id, topic_name);
                self.transport_send(ProxyMessage::RegAck {
                    topic_id,
                    msg_id,
                    return_code: ReturnCode::Accepted,
                });
            }

            ProxyMessage::PingReq { timestamp } => {
                self.transport_send(ProxyMessage::PingResp { timestamp });
            }
            _ => {
                // Ignore
            }
        }
    }
}

impl SinkTrait<ProxyServerCmd> for ProxySession {
    fn push(&self, message: ProxyServerCmd) {
        self.cmds.push(message);
    }
}

impl SourceTrait<ProxyServerEvent> for ProxySession {
    fn add_listener(&mut self, sink: SinkRef<ProxyServerEvent>) {
        self.events.add_listener(sink);
    }
}
