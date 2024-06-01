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
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_time::{with_timeout, Duration, Timer};
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

use crate::protocol::msg::ProxyMessage;
use crate::limero::{Actor, Source, SourceTrait};

static CMD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
static PUB_MSG: PubSubChannel<CriticalSectionRawMutex, ProxyMessage, 10, 10, 10> =
    PubSubChannel::new();
const RC_ACCEPTED: u8 = 0;
const RC_REJECTED_CONGESTION: u8 = 1;
const RC_REJECTED_INVALID_TOPIC_ID: u8 = 2;
const RC_REJECTED_NOT_SUPPORTED: u8 = 3;

static RXD_DATA: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

trait Subscriber {
    fn on_message(&self, message: & ProxyMessage);
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
    T: for<'b> Decode<'b, i32> , 
{
    fn on_message(&self, message: & ProxyMessage) {
        if let ProxyMessage::Publish { topic_id, message } = message {
            if *topic_id == self.topic_id {
                let mut cbor_decoder = Decoder::new(message); // Lifetime here is limited to this scope

                let res = T::decode(&mut cbor_decoder, &mut 1);

                if res.is_ok() {
                    let _ = self.sender.try_send(res.unwrap());
                }
            }
        }
        
    }
}

pub struct ClientSession {
    pub actor : Actor<ProxyMessage,ProxyMessage>,
    client_topics: BTreeMap<u16, String>,
    server_topics: BTreeMap<u16, String>,
    client_topics_registered: BTreeMap<u16, bool>,
    client_topics_sender: BTreeMap<u16, Box<dyn Subscriber>>,
    client_id: String,
    state: State,
    ping_timeouts: u32,
    txd_msg: Source<ProxyMessage>,
    rxd_msg: DynamicReceiver<'static, ProxyMessage>,
    msg_handler: Source< ProxyMessage>,
}
impl ClientSession {
    pub fn new() -> ClientSession {
        ClientSession {
            actor: Actor::<ProxyMessage,ProxyMessage>::new(Box::new(|_actor,msg| {
                info!("ClientSession received message: {:?}", msg);
            })),
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            client_topics_registered: BTreeMap::new(),
            client_topics_sender: BTreeMap::new(),
            client_id: "ESP32_1".to_string(),
            state: State::Disconnected,
            ping_timeouts: 0,
            txd_msg: Source::new(),
            rxd_msg: RXD_DATA.dyn_receiver(),
            msg_handler: Source::new(),
        }
    }

    async fn publish<T>(&mut self, topic_id: u16, _message: T)
    where
        T: Encode<String>,
    {
        let mut buffer = Vec::<u8>::new();
        let mut encoder = Encoder::new(&mut buffer);
        let mut s = String::new();
        _message.encode(&mut encoder, &mut s).unwrap();
        let msg = ProxyMessage::Publish {
            topic_id,
            message: buffer,
        };
        self.txd_msg.emit(&msg);
    }

    async fn handler(&mut self) {
        loop {
            let _res = select(CMD_MSG.receive(), self.rxd_msg.receive()).await;
            match _res {
                First(msg) => {
                    self.on_cmd_message(msg).await;
                }
                Second(msg) => {
                    self.on_rxd_message(msg).await;
                }
            }
        }
    }

    async fn on_cmd_message(&mut self, msg: ProxyMessage) {
        match msg {
            ProxyMessage::Publish {
                topic_id,
                message: _,
            } => {
                info!("Received message on topic {}", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.txd_msg.emit(&ProxyMessage::PubAck {
                        topic_id,
                        return_code: RC_ACCEPTED,
                    });
                } else {
                    self.txd_msg.emit(&ProxyMessage::PubAck {
                        topic_id,
                        return_code: RC_REJECTED_INVALID_TOPIC_ID,
                    });
                }
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    async fn on_rxd_message(&mut self, msg: ProxyMessage) {
        match msg {
            ProxyMessage::Register {
                topic_id,
                topic_name,
            } => {
                info!("Registering topic {} with id {}", topic_name, topic_id);
                self.server_topics.insert(topic_id, topic_name);
            }
            ProxyMessage::Publish {
                topic_id,
                message: _,
            } => {
                info!("Received message on topic {} ", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.txd_msg.emit(&ProxyMessage::PubAck {
                        topic_id,
                        return_code: RC_ACCEPTED,
                    });
                } else {
                    self.txd_msg.emit(&ProxyMessage::PubAck {
                        topic_id,
                        return_code: RC_REJECTED_INVALID_TOPIC_ID,
                    });
                }
            }
            ProxyMessage::PubAck {
                topic_id,
                return_code,
            } => {
                if return_code == RC_ACCEPTED {
                    info!(
                        "Received PubAck for topic {} with code {}",
                        topic_id, return_code
                    );
                } else {
                    self.txd_msg.emit(&ProxyMessage::Register {
                        topic_id,
                        topic_name: self.client_topics.get(&topic_id).unwrap().clone(),
                    });
                    self.client_topics_registered.insert(topic_id, true);
                }
                info!(
                    "Received PubAck for topic {} with code {}",
                    topic_id, return_code
                );
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    pub async fn run(&mut self) {
        loop {
            info!("Client running");
            let result = with_timeout(Duration::from_secs(5), self.rxd_msg.receive()).await;
            match result {
                Ok(msg) => match msg {
                    ProxyMessage::ConnAck { return_code } => {
                        info!("Connected code  {}", return_code);
                        if self.state == State::Disconnected {
                            self.state = State::Connected;
                        }
                    }
                    ProxyMessage::PingResp {} => {
                        info!("Ping response");
                    }
                    ProxyMessage::Disconnect {} => {
                        info!("Disconnected");
                        self.state = State::Disconnected;
                        self.server_topics.clear();
                    }
                    ProxyMessage::Register {
                        topic_id,
                        topic_name,
                    } => {
                        info!("Registering topic {} with id {}", topic_name, topic_id);
                        self.server_topics.insert(topic_id, topic_name);
                    }
                    ProxyMessage::Publish {
                        topic_id,
                        message: _,
                    } => {
                        info!("Received message on topic {} ", topic_id);
                        if self.server_topics.contains_key(&topic_id) {
                            self.txd_msg.emit(&ProxyMessage::PubAck {
                                topic_id,
                                return_code: RC_ACCEPTED,
                            });
                        } else {
                            self.txd_msg.emit(&ProxyMessage::PubAck {
                                topic_id,
                                return_code: RC_REJECTED_INVALID_TOPIC_ID,
                            });
                        }
                    }
                    ProxyMessage::PubAck {
                        topic_id,
                        return_code,
                    } => {
                        if return_code == RC_ACCEPTED {
                            info!(
                                "Received PubAck for topic {} with code {}",
                                topic_id, return_code
                            );
                        } else {
                            self.txd_msg.emit(&ProxyMessage::Register {
                                topic_id,
                                topic_name: self.client_topics.get(&topic_id).unwrap().clone(),
                            });
                        }
                        info!(
                            "Received PubAck for topic {} with code {}",
                            topic_id, return_code
                        );
                    }

                    _ => {
                        info!("Unexpected message {:?}", msg);
                    }
                },
                Err(_) => match self.state {
                    // on timeout
                    State::Disconnected => {
                        self.txd_msg.emit(&ProxyMessage::Connect {
                            protocol_id: 1,
                            duration: 100,
                            client_id: self.client_id.clone(),
                        });
                    }
                    State::Connected => {
                        if self.ping_timeouts > 5 {
                            self.txd_msg.emit(&ProxyMessage::Disconnect {});
                            self.state = State::Disconnected;
                            self.server_topics.clear();
                        } else {
                            self.ping_timeouts += 1;
                            self.txd_msg.emit(&ProxyMessage::PingReq {});
                        }
                    }
                },
            }
        }
    }
}
