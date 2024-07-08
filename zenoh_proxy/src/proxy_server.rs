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
use protocol::msg::MqttSnMessage;
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
    zenoh_session: Option<zenoh::Session>,
    client_topics: BTreeMap<u16, TopicId>,
    server_topics: BTreeMap<u16, TopicId>,
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
            zenoh_session: None,
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

    pub fn transport_send(&self, message: MqttSnMessage) {
        self.transport_cmd
            .push(TransportCmd::SendMessage { message });
    }

    fn get_server_topic_from_string(&mut self, topic: &str) -> u16 {
        for (id, topic_id) in self.server_topics.iter() {
            if topic_id.name == topic {
                return *id;
            }
        }
        let topic_id = self.topic_id_counter;
        self.topic_id_counter += 1;
        self.server_topics.insert(
            topic_id,
            TopicId {
                id: topic_id,
                name: topic.to_string(),
                acked: false,
            },
        );
        topic_id
    }

    pub async fn run(&mut self) {
        self.pubsub_cmd.push(PubSubCmd::Connect);

        let _buf = vec![0u8; MTU_SIZE];
        loop {
            select! {
                cmd = self.cmds.next() => {
                    let cmd = cmd.unwrap();
                    info!("ProxyServerCmd:: {:?}", &cmd);
                    match cmd{
                        ProxyServerCmd::Publish { topic, message } => {
                            info!("Publishing message to zenoh");
                            self.zenoh_session.as_ref().unwrap().put(&topic, message).res().await.unwrap();
                        },
                        ProxyServerCmd::TransportEvent(event) => {
                            info!("Received event from transport {:?}", event);
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
                let topic_id = self.get_server_topic_from_string(&topic);
                self.transport_send(MqttSnMessage::Publish {
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

    async fn on_transport_rxd(&mut self, event: MqttSnMessage) {
        match event {
            MqttSnMessage::Connect {
                flags: _,
                duration: _,
                client_id: _,
            } => {
                self.transport_send(MqttSnMessage::ConnAck {
                    return_code: ReturnCode::Accepted,
                });
                self.client_topics.clear();
            }
            MqttSnMessage::WillTopic { flags: _, topic } => {
                info!("Received WillTopic message");
                self.will_topic = Some(topic);
                self.transport_send(MqttSnMessage::WillMsgReq {});
            }
            MqttSnMessage::WillMsg { message } => {
                self.will_message = Some(message);
                info!("Received WillMsg message");
            }
            MqttSnMessage::Publish {
                flags,
                topic_id,
                msg_id,
                data,
            } => match self.client_topics.get(&topic_id) {
                Some(_topic) => {
                    info!("Received Publish message for known topic");
                    let x = self.client_topics.get_mut(&topic_id).unwrap().name.clone();
                    self.pubsub_cmd.push(PubSubCmd::Publish {
                        topic: x,
                        message: data,
                    });
                    if flags.qos() == 1 {
                        self.transport_send(MqttSnMessage::PubAck {
                            topic_id,
                            msg_id,
                            return_code: ReturnCode::Accepted,
                        });
                    }
                }
                None => {
                    info!("Received Publish message for unknown topic");
                    self.transport_send(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::InvalidTopicId,
                    });
                }
            },
            MqttSnMessage::Subscribe {
                flags,
                msg_id,
                topic_id,
                topic: _,
                qos: _,
            } => {
                /*zenoh_subscriber
                .with_subscriber(topic.clone())
                .res()
                .await
                .unwrap();*/
                self.transport_send(MqttSnMessage::SubAck {
                    flags,
                    msg_id,
                    topic_id: topic_id.unwrap(),
                    return_code: ReturnCode::Accepted,
                });
            }
            MqttSnMessage::Register {
                topic_id,
                msg_id,
                topic_name,
            } => {
                info!("Received Unsubscribe message");
                self.client_topics.insert(
                    topic_id,
                    TopicId {
                        id: topic_id,
                        name: topic_name,
                        acked: false,
                    },
                );
                self.transport_send(MqttSnMessage::RegAck {
                    topic_id,
                    msg_id,
                    return_code: ReturnCode::Accepted,
                });
            }

            MqttSnMessage::PingReq { timestamp } => {
                self.transport_send(MqttSnMessage::PingResp { timestamp });
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
