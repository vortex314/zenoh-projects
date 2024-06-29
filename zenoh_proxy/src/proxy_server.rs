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
use protocol::msg::MqttSnMessage;
use protocol::msg::ReturnCode;
use protocol::MessageDecoder;
use protocol::MTU_SIZE;

use crate::transport::*;

use zenoh::prelude::r#async::*;

const GREEN: &str = "\x1b[0;32m";
const RESET: &str = "\x1b[m";

#[derive(Clone)]
pub enum ProxyServerEvent {
    Connected,
    Disconnected,
    Publish { topic: String, message: Vec<u8> },
}

#[derive(Clone)]
pub enum ProxyServerCmd {
    Connect {
        protocol_id: u8,
        duration: u16,
        client_id: String,
    },
    WillTopic {
        topic: String,
    },
    WillMsg {
        message: Vec<u8>,
    },

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
    transport_event: Sink<TransportEvent>,
    zenoh_session: Option<zenoh::Session>,
    client_topics: BTreeMap<u16, TopicId>,
    server_topics: BTreeMap<u16, TopicId>,
    will_topic: Option<String>,
    will_message: Option<Vec<u8>>,
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
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            will_topic: None,
            will_message: None,
        }
    }

    pub fn transport_sink_ref(&self) -> SinkRef<TransportEvent> {
        self.transport_event.sink_ref()
    }

    pub fn transport_send(&self, message: MqttSnMessage) {
        self.transport_cmd
            .push(TransportCmd::SendMessage { message });
    }

    pub async fn run(&mut self) {
        let zenoh_session = zenoh::open(config::default()).res().await.unwrap();
        let  zenoh_subscriber = zenoh_session
            .declare_subscriber("esp32/*")
            .res()
            .await
            .unwrap();
        let  _buf = vec![0u8; MTU_SIZE];
        loop {
            select! {
                _msg = zenoh_subscriber.recv_async() => {

                },
                cmd = self.commands.read() => {
                    match cmd {
                       _ => {
                           info!("Received command from client ");
                          },

                    }
                },
                event = self.transport_event.read() => {
                    match event {
                        Some(TransportEvent::ConnectionLost {}) => {
                            info!("Connection lost");
                            self.events.emit(ProxyServerEvent::Disconnected);
                        },
                        Some(TransportEvent::RecvMessage { message }) => {
                            info!("Received transport event from client ");
                            match message {
                                MqttSnMessage::Connect { flags:_,duration:_,client_id:_ }=> {
                                    info!("Received Connect message");
                                    self.transport_send(MqttSnMessage::ConnAck { return_code: ReturnCode::Accepted });
                                    self.client_topics.clear();
                                },
                                MqttSnMessage::WillTopic { flags:_,topic } => {
                                    info!("Received WillTopic message");
                                    self.will_topic = Some(topic);
                                    self.transport_send(MqttSnMessage::WillMsgReq { });
                                },
                                MqttSnMessage::WillMsg { message } => {
                                    self.will_message = Some(message);
                                    info!("Received WillMsg message");
                                },
                                MqttSnMessage::Publish { flags:_,topic_id, msg_id,data:_ } => {
                                    match self.client_topics.get(&topic_id) {
                                        Some(_topic) => {
                                            info!("Received Publish message");
                                           // zenoh_session.put(&topic.name, message).res().await.unwrap();
                                            self.transport_send(MqttSnMessage::PubAck { topic_id,msg_id, return_code: ReturnCode::Accepted } );
                                        },
                                        None => {
                                            info!("Received Publish message for unknown topic");
                                            self.transport_send(MqttSnMessage::PubAck { topic_id,msg_id, return_code : ReturnCode::InvalidTopicId });
                                        },
                                    }
                                },
                                MqttSnMessage::Subscribe { flags,msg_id,topic_id, topic:_,qos:_} => {
                                    /*zenoh_subscriber
                                        .with_subscriber(topic.clone())
                                        .res()
                                        .await
                                        .unwrap();*/
                                    self.transport_send(MqttSnMessage::SubAck { flags,msg_id,topic_id:topic_id.unwrap(),return_code : ReturnCode::Accepted });

                                },
                                MqttSnMessage::Register { topic_id,msg_id,topic_name, } => {
                                    info!("Received Unsubscribe message");
                                    self.client_topics.insert(topic_id, TopicId { id: topic_id, name: topic_name, acked: false });
                                    self.transport_send(MqttSnMessage::RegAck { topic_id,msg_id, return_code: ReturnCode::Accepted });
                                },

                                MqttSnMessage::PingReq { timestamp }=> {
                                    info!("Received PingReq message with {:?} ",timestamp);
                                    self.transport_send(MqttSnMessage::PingResp{timestamp});
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
