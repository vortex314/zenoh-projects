use core::result;

use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::vec::Vec;
use cobs::CobsEncoder;
use embassy_executor::Spawner;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{DynamicReceiver, DynamicSender, Sender};
use embassy_sync::pubsub::publisher::Pub;
use embassy_sync::pubsub::PubSubChannel;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use alloc::collections::BTreeMap;
use alloc::string::String;
use embassy_futures::select::Either3::{First, Second, Third};
use embassy_futures::select::{self, select3, Either3};
use embassy_time::{with_timeout, Duration, Instant};
use embedded_svc::ws::Receiver;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
};
use esp_println::println;
use log::{debug, info};
use minicbor::bytes::ByteVec;

use minicbor::decode::info;
use minicbor::{encode::write::EndOfSlice, Decode, Decoder, Encode, Encoder};

use crate::limero::{timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::protocol::msg::{Flags, MqttSnMessage, ReturnCode, VecWriter};

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

#[derive(Clone, Debug)]
pub enum SessionCmd {
    Publish { topic: String, message: String },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
    Connect { client_id: String },
    Disconnect,
}
#[derive(Clone, Debug)]
pub enum SessionEvent {
    Publish { topic_id: u16, message: String },
    Connected,
    Disconnected,
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct ClientSession {
    command: Sink<SessionCmd, 3>,
    events: Source<SessionEvent>,
    transport_cmd: SinkRef<MqttSnMessage>,
    rxd_msg: Sink<MqttSnMessage, 3>,
    client_topics: BTreeMap<String, u16>,
    server_topics: BTreeMap<u16, String>,
    client_topics_registered: BTreeMap<u16, bool>,
    //    client_topics_sender: BTreeMap<u16, Box<dyn Subscriber>>,
    client_id: String,
    state: State,
    ping_timeouts: u32,
    msg_id: u16,
    timers: Timers,
    topic_id_counter: u16,
}
impl ClientSession {
    pub fn new(txd_msg: SinkRef<MqttSnMessage>) -> ClientSession {
        ClientSession {
            command: Sink::new(),
            events: Source::new(),
            transport_cmd: txd_msg,
            rxd_msg: Sink::new(),
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            client_topics_registered: BTreeMap::new(),
            //           client_topics_sender: BTreeMap::new(),
            client_id: "ESP32_1".to_string(),
            state: State::Disconnected,
            ping_timeouts: 0,
            msg_id: 0,
            timers: Timers::new(),
            topic_id_counter: 0,
        }
    }

    pub fn sink_ref(&self) -> SinkRef<SessionCmd> {
        self.command.sink_ref()
    }

    pub fn transport_sink_ref(&self) -> SinkRef<MqttSnMessage> {
        self.rxd_msg.sink_ref()
    }

    fn register_topic(&mut self, topic: &str) -> u16 {
        let topic_id = self.topic_id_counter;
        self.topic_id_counter += 1;
        self.transport_cmd.push(MqttSnMessage::Register {
            topic_id: self.topic_id_counter,
            msg_id: 0,
            topic_name: topic.to_string(),
        });

        self.client_topics.insert(topic.to_string(), topic_id);
        self.client_topics_registered.insert(topic_id, true);
        self.topic_id_counter += 1;
        topic_id
    }

    fn get_client_topic_from_string(&mut self, topic: &str) -> u16 {
        let topic_id = match self.client_topics.get(topic) {
            Some(topic_id) => *topic_id,
            None => self.register_topic(topic),
        };
        topic_id
    }

    pub async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(
            TimerId::ConnectTimer as u32,
            Duration::from_millis(5_000),
        ));
        self.timers.add_timer(Timer::new_repeater(
            TimerId::PingTimer as u32,
            Duration::from_millis(1_000),
        ));

        loop {
            match select3(
                self.command.read(),
                self.rxd_msg.read(),
                self.timers.alarm(),
            )
            .await
            {
                First(msg) => {
                    self.on_cmd_message(msg.unwrap()).await;
                }
                Second(msg) => {
                    self.on_rxd_message(msg.unwrap()).await;
                }
                Third(idx) => {
                    self.on_timeout(idx).await;
                }
            }
        }
    }

    async fn on_timeout(&mut self, id: u32) {
        if id == TimerId::ConnectTimer as u32 {
            if self.state == State::Disconnected {
                self.transport_cmd.push(MqttSnMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: self.client_id.clone(),
                });
            }
        } else if id == TimerId::PingTimer as u32 {
            if self.state == State::Connected {
                self.transport_cmd.push(MqttSnMessage::PingReq {
                    timestamp: Instant::now().as_millis() as u64,
                });
                self.ping_timeouts += 1;
                if self.ping_timeouts > 3 {
                    info!("Ping timeout >3 disconnecting    ");
                    self.transport_cmd
                        .push(MqttSnMessage::Disconnect { duration: 0 });
                    self.state = State::Disconnected;
                    self.events.emit(SessionEvent::Disconnected);
                }
            }
        } else {
            info!("Unexpected timer id {}", id);
            self.ping_timeouts += 1;
            if self.ping_timeouts > 3 {
                self.transport_cmd
                    .push(MqttSnMessage::Disconnect { duration: 0 });
            }
        }
    }

    fn encode<X>(&self, v: X) -> Vec<u8>
    where
        X: Encode<()>,
    {
        let mut buffer = VecWriter::new();
        let mut encoder = Encoder::new(&mut buffer);
        let _x = encoder.encode(v);
        _x.unwrap().writer().to_vec()
    }

    async fn on_rxd_message(&mut self, msg: MqttSnMessage) {
        match msg {
            MqttSnMessage::ConnAck { return_code } => {
                info!("Connected code  {:?}", return_code);
                if self.state == State::Disconnected {
                    self.state = State::Connected;
                    self.events.emit(SessionEvent::Connected);
                }
                self.transport_cmd.push(MqttSnMessage::Register {
                    topic_id: 0,
                    msg_id: 0,
                    topic_name: "ESP32".to_string(),
                });
                self.transport_cmd.push(MqttSnMessage::Register {
                    topic_id: 1,
                    msg_id: 0,
                    topic_name: "latency".to_string(),
                });
            }
            MqttSnMessage::PingResp { timestamp } => {
                debug!("Ping response {:?}", timestamp);
                self.ping_timeouts = 0;
                let now: u64 = Instant::now().as_millis() as u64;
                let diff: u64 = now - timestamp;
                debug!("Ping response time {}", diff);
                let topic_id = self.get_client_topic_from_string("latency");
                self.transport_cmd.push(MqttSnMessage::Publish {
                    topic_id, // 1 is the topic id for the ping response
                    msg_id: self.msg_id,
                    flags: Flags(0),
                    data: self.encode(diff),
                });
            }
            MqttSnMessage::Disconnect { duration: _ } => {
                info!("Disconnected");
                self.state = State::Disconnected;
                self.events.emit(SessionEvent::Disconnected);
                self.server_topics.clear();
            }
            MqttSnMessage::Register {
                topic_id,
                msg_id: _,
                topic_name,
            } => {
                info!("Registering topic {} with id {}", topic_name, topic_id);
                self.server_topics.insert(topic_id, topic_name);
            }
            MqttSnMessage::Publish {
                flags: _,
                topic_id,
                msg_id,
                data: _,
            } => {
                info!("Received message on topic {} ", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.transport_cmd.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::Accepted,
                    });
                } else {
                    self.transport_cmd.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::InvalidTopicId,
                    });
                }
            }
            MqttSnMessage::PubAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                if return_code == ReturnCode::Accepted {
                    info!(
                        "Received PubAck for topic {} with code {:?}",
                        topic_id, return_code
                    );
                } else {
                    let topic_name = self.client_topics.iter().find(|(_, v)| **v == topic_id);
                    if topic_name.is_none() {
                        info!(
                            "Received PubAck for topic {} with code {:?}",
                            topic_id, return_code
                        );
                        return;
                    }
                    let topic_name = topic_name.unwrap().0;
                    self.transport_cmd.push(MqttSnMessage::Register {
                        topic_id,
                        msg_id,
                        topic_name: topic_name.clone(),
                    });
                }
            }
            MqttSnMessage::RegAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                if return_code == ReturnCode::Accepted {
                    self.client_topics_registered.insert(topic_id, true);
                } else {
                    info!(
                        "Received RegAck Failure  for topic {} msg_id {} with code {:?}",
                        topic_id, msg_id, return_code
                    );
                }
            }

            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    pub async fn on_cmd_message(&mut self, cmd: SessionCmd) {
        match cmd {
            SessionCmd::Publish { topic, message } => {
                let topic_id = self.get_client_topic_from_string(&topic);
                let mut flags = Flags(0);
                flags.set_qos(0);
                self.transport_cmd.push(MqttSnMessage::Publish {
                    flags,
                    topic_id,
                    msg_id: self.msg_id,
                    data: message.as_bytes().to_vec(),
                });
                self.msg_id += 1;
            }
            SessionCmd::Subscribe { topic } => {
                self.msg_id += 1;
                let topic_id = self.get_client_topic_from_string(topic.as_str());
                self.transport_cmd.push(MqttSnMessage::Register {
                    topic_id,
                    msg_id: self.msg_id,
                    topic_name: topic,
                });
            }
            SessionCmd::Unsubscribe { topic } => {
                self.msg_id += 1;
                let topic_id = self.get_client_topic_from_string(topic.as_str());
                self.transport_cmd.push(MqttSnMessage::PubAck {
                    topic_id,
                    msg_id: self.msg_id,
                    return_code: ReturnCode::Accepted,
                });
            }
            SessionCmd::Connect { client_id } => {
                self.transport_cmd.push(MqttSnMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: client_id.clone(),
                });
            }
            SessionCmd::Disconnect => {
                self.transport_cmd
                    .push(MqttSnMessage::Disconnect { duration: 0 });
            }
        }
    }
}

impl SourceTrait<SessionEvent> for ClientSession {
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<SessionEvent>>) {
        self.events.subscribe(sink);
    }
}
