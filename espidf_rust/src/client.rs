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
use embassy_time::{with_timeout, Duration};
use embedded_svc::ws::Receiver;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    embassy,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
    Uart,
};
use esp_println::println;
use log::info;
use minicbor::bytes::ByteVec;

use minicbor::decode::info;
use minicbor::{encode::write::EndOfSlice, Decode, Decoder, Encode, Encoder};

use crate::limero::{Sink, SinkRef, SinkTrait, Source, SourceTrait, Timer, Timers};
use crate::protocol::msg::{Flags, MqttSnMessage, ReturnCode};

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

trait Subscriber {
    fn on_message(&self, message: &MqttSnMessage);
}

struct Sub<'a, T> {
    topic_id: u16,
    sender: DynamicSender<'a, T>,
}

impl<'a, T> Sub<'a, T> {
    pub fn new(topic_id: u16, sender: DynamicSender<'a, T>) -> Self {
        Sub { topic_id, sender }
    }
}

impl<'a, T> Subscriber for Sub<'a, T>
where
    T: for<'b> Decode<'b, i32>,
{
    fn on_message(&self, message: &MqttSnMessage) {
        if let MqttSnMessage::Publish {
            flags,
            topic_id,
            msg_id,
            data,
        } = message
        {
            if *topic_id == self.topic_id {
                let mut cbor_decoder = Decoder::new(data); // Lifetime here is limited to this scope

                let res = T::decode(&mut cbor_decoder, &mut 1);

                if res.is_ok() {
                    let _ = self.sender.try_send(res.unwrap());
                }
            }
        }
    }
}
#[derive(Clone, Debug)]
pub enum SessionCmd {
    Publish { topic_id: u16, message: String },
    Subscribe { topic_id: u16 },
    Unsubscribe { topic_id: u16 },
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
}

pub struct ClientSession {
    command: Sink<SessionCmd, 3>,
    events: Source<SessionEvent>,
    txd_msg: SinkRef<MqttSnMessage, 4>,
    rxd_msg: Sink<MqttSnMessage, 3>,
    client_topics: BTreeMap<u16, String>,
    server_topics: BTreeMap<u16, String>,
    client_topics_registered: BTreeMap<u16, bool>,
    client_topics_sender: BTreeMap<u16, Box<dyn Subscriber>>,
    client_id: String,
    state: State,
    ping_timeouts: u32,
    msg_id: u16,
    timers: Timers,
}
impl ClientSession {
    pub fn new(txd_msg: SinkRef<MqttSnMessage, 4>) -> ClientSession {
        ClientSession {
            command: Sink::new(),
            events: Source::new(),
            txd_msg,
            rxd_msg: Sink::new(),
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            client_topics_registered: BTreeMap::new(),
            client_topics_sender: BTreeMap::new(),
            client_id: "ESP32_1".to_string(),
            state: State::Disconnected,
            ping_timeouts: 0,
            msg_id: 0,
            timers: Timers::new(),
        }
    }

    pub async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(
            TimerId::ConnectTimer as u32,
            Duration::from_millis(1000),
        ));
        loop {
            let _res = select3(
                self.command.read(),
                self.rxd_msg.read(),
                self.timers.alarm(),
            )
            .await;
            match _res {
                First(msg) => {
                    self.on_cmd_message(msg.unwrap()).await;
                }
                Second(msg) => {
                    self.on_rxd_message(msg.unwrap()).await;
                }
                Third(idx) => {
                    info!("Timer expired {}", idx);
                    self.on_timeout(idx).await;
                }
            }
        }
    }

    async fn on_timeout(&mut self, id: u32) {
        if id == TimerId::ConnectTimer as u32 {
            if self.state == State::Disconnected {
                self.txd_msg.push(MqttSnMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: self.client_id.clone(),
                });
            }
        } else if id == TimerId::PingTimer as u32 {
            if self.state == State::Connected {
                self.txd_msg
                    .push(MqttSnMessage::PingReq { client_id: None });
                self.ping_timeouts += 1;
                if self.ping_timeouts > 3 {
                    self.txd_msg.push(MqttSnMessage::Disconnect { duration: 0 });
                }
            }
        } else {
            info!("Unexpected timer id {}", id);
        }
    }

    async fn on_cmd_message(&mut self, msg: SessionCmd) {
        match msg {
            SessionCmd::Publish {
                topic_id,
                message: _,
            } => {
                info!("Received message on topic {}", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.txd_msg.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id: self.msg_id,
                        return_code: ReturnCode::Accepted,
                    });
                    self.msg_id += 1;
                } else {
                    self.txd_msg.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id: self.msg_id,
                        return_code: ReturnCode::InvalidTopicId,
                    });
                    self.msg_id += 1;
                }
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    async fn on_rxd_message(&mut self, msg: MqttSnMessage) {
        match msg {
            MqttSnMessage::ConnAck { return_code } => {
                info!("Connected code  {:?}", return_code);
                if self.state == State::Disconnected {
                    self.state = State::Connected;
                }
            }
            MqttSnMessage::PingResp {} => {
                info!("Ping response");
            }
            MqttSnMessage::Disconnect { duration: _ } => {
                info!("Disconnected");
                self.state = State::Disconnected;
                self.server_topics.clear();
            }
            MqttSnMessage::Register {
                topic_id,
                msg_id,
                topic_name,
            } => {
                info!("Registering topic {} with id {}", topic_name, topic_id);
                self.server_topics.insert(topic_id, topic_name);
            }
            MqttSnMessage::Publish {
                flags,
                topic_id,
                msg_id,
                data: _,
            } => {
                info!("Received message on topic {} ", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.txd_msg.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::Accepted,
                    });
                } else {
                    self.txd_msg.push(MqttSnMessage::PubAck {
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
                    self.txd_msg.push(MqttSnMessage::Register {
                        topic_id,
                        msg_id,
                        topic_name: self.client_topics.get(&topic_id).unwrap().clone(),
                    });
                }
                info!(
                    "Received PubAck for topic {} with code {:?}",
                    topic_id, return_code
                );
            }

            MqttSnMessage::Publish {
                flags,
                topic_id,
                msg_id,
                data,
            } => {
                info!("Received message on topic {} ", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.txd_msg.push(MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::Accepted,
                    });
                } else {
                    self.txd_msg.push(MqttSnMessage::PubAck {
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
                    self.txd_msg.push(MqttSnMessage::Register {
                        topic_id,
                        msg_id,
                        topic_name: self.client_topics.get(&topic_id).unwrap().clone(),
                    });
                    self.client_topics_registered.insert(topic_id, true);
                }
                info!(
                    "Received PubAck for topic {} with code {:?}",
                    topic_id, return_code
                );
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    pub async fn on_cmd_msg(&mut self, cmd: SessionCmd) {
        match cmd {
            SessionCmd::Publish { topic_id, message } => {
                self.txd_msg.push(MqttSnMessage::Publish {
                    flags: Flags(0),
                    topic_id,
                    msg_id: self.msg_id,
                    data: message.as_bytes().to_vec(),
                });
            }
            SessionCmd::Subscribe { topic_id } => {
                self.txd_msg.push(MqttSnMessage::Register {
                    topic_id,
                    msg_id: 0,
                    topic_name: self.client_topics.get(&topic_id).unwrap().clone(),
                });
            }
            SessionCmd::Unsubscribe { topic_id } => {
                self.txd_msg.push(MqttSnMessage::PubAck {
                    topic_id,
                    msg_id: 0,
                    return_code: ReturnCode::Accepted,
                });
            }
            SessionCmd::Connect { client_id } => {
                self.txd_msg.push(MqttSnMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: client_id.clone(),
                });
            }
            SessionCmd::Disconnect => {
                self.txd_msg.push(MqttSnMessage::Disconnect { duration: 0 });
            }
        }
    }
}
