use core::result;

use alloc::boxed::Box;
use alloc::format;
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
use embassy_futures::select::{self};
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

use minicbor::decode::{self, info};
use minicbor::{encode::write::EndOfSlice, Decode, Decoder, Encode, Encoder};

use crate::limero::{timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::protocol::msg::{ Flags, ProxyMessage, ReturnCode, VecWriter};

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

#[derive(Clone, Debug)]
pub enum PubSubCmd {
    Publish { topic: String, message : Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
    Connect { client_id: String },
    Rxd(ProxyMessage),
    Disconnect,
}
#[derive(Clone, Debug)]
pub enum PubSubEvent {
    Publish { topic: String, message: Vec<u8> },
    Connected,
    Disconnected,
}

pub fn payload_encode<X>( v: X) -> Vec<u8>
where
    X: Encode<()>,
{
    let mut buffer = Vec::<u8>::new();
    let mut encoder = Encoder::new(&mut buffer);
    let _x = encoder.encode(v);
    _x.unwrap().writer().to_vec()
}

pub fn payload_decode<'a,T>(v: &'a Vec<u8>) -> Result<T, decode::Error>
where T : Decode<'a,()>
{
    let mut decoder = Decoder::new(v);
    decoder.decode::<T>()
}


pub fn payload_display(v: &Vec<u8>) -> String {
    let line:String  = v.iter().map(|b| format!("{:02X} ", b)).collect();
    let s = format!("{}", minicbor::display(v.as_slice()));
    if s.len() == 0 {
        line
    } else {
        s
    }
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct PubSubActor {
    command: Sink<PubSubCmd, 3>,
    events: Source<PubSubEvent>,
    transport_cmd: SinkRef<ProxyMessage>,
    rxd_msg: Sink<ProxyMessage, 3>,
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
impl PubSubActor {
    pub fn new(txd_msg: SinkRef<ProxyMessage>) -> PubSubActor {
        PubSubActor {
            command: Sink::new(),
            events: Source::new(),
            transport_cmd: txd_msg,
            rxd_msg: Sink::new(),
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            client_topics_registered: BTreeMap::new(),
            //           client_topics_sender: BTreeMap::new(),
            client_id: "esp32".to_string(),
            state: State::Disconnected,
            ping_timeouts: 0,
            msg_id: 0,
            timers: Timers::new(),
            topic_id_counter: 0,
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.command.sink_ref()
    }

    pub fn transport_sink_ref(&self) -> SinkRef<ProxyMessage> {
        self.rxd_msg.sink_ref()
    }

    fn register_topic(&mut self, topic: &str) {
        let topic_id = self.get_client_topic_from_string(topic);
        self.txd(ProxyMessage::Register {
            topic_id,
            msg_id: 0,
            topic_name: topic.to_string(),
        });
    }

    fn get_client_topic_from_string(&mut self, topic: &str) -> u16 {
        match self.client_topics.get(topic) {
            Some(topic_id) => *topic_id,
            None => {
                self.topic_id_counter += 1;
                let topic_id = self.topic_id_counter;
                self.client_topics.insert(topic.to_string(), topic_id);
                topic_id
            }
        }
    }

    fn txd(&mut self, msg: ProxyMessage) {
        self.transport_cmd.send(msg);
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

        self.txd(ProxyMessage::Connect {
            flags: Flags(0),
            duration: 100,
            client_id: self.client_id.clone(),
        });

        loop {
            match select(self.command.next(), self.timers.alarm()).await {
                First(msg) => match msg {
                    Some(PubSubCmd::Rxd(m)) => {
                        self.on_rxd_message(m).await;
                    }
                    Some(cmd) => {
                        self.on_cmd_message(cmd).await;
                    }
                    None => {
                        info!("Unexpected {:?}", msg);
                    }
                },
                Second(idx) => {
                    self.on_timeout(idx).await;
                }
            }
        }
    }

    async fn on_timeout(&mut self, id: u32) {
        if id == TimerId::ConnectTimer as u32 {
            if self.state == State::Disconnected {
                self.txd(ProxyMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: self.client_id.clone(),
                });
            }
        } else if id == TimerId::PingTimer as u32 {
            if self.state == State::Connected {
                self.txd(ProxyMessage::PingReq {
                    timestamp: Instant::now().as_millis() as u64,
                });
                self.ping_timeouts += 1;
                if self.ping_timeouts > 3 {
                    info!("Ping timeout >3 disconnecting    ");
                    self.txd(ProxyMessage::Disconnect { duration: 0 });
                    self.state = State::Disconnected;
                    self.events.emit(PubSubEvent::Disconnected);
                }
            }
        } else {
            info!("Unexpected timer id {}", id);
            self.ping_timeouts += 1;
            if self.ping_timeouts > 3 {
                self.transport_cmd
                    .send(ProxyMessage::Disconnect { duration: 0 });
            }
        }
    }



    async fn on_rxd_message(&mut self, msg: ProxyMessage) {
        match msg {
            ProxyMessage::ConnAck { return_code } => {
                info!("Connected code  {:?}", return_code);
                if self.state == State::Disconnected {
                    self.state = State::Connected;
                    self.events.emit(PubSubEvent::Connected);
                }
                self.txd(ProxyMessage::Subscribe {
                    flags: Flags(0),
                    msg_id: 1,
                    topic: "dst/esp32/**".to_string(),
                    qos: 0,
                });
                self.register_topic("src/esp32/sys/uptime");
                self.register_topic("src/esp32/sys/latency");
            }
            ProxyMessage::PingResp { timestamp } => {
                debug!("Ping response {:?}", timestamp);
                self.ping_timeouts = 0;
                let now: u64 = Instant::now().as_millis() as u64;
                let diff: u64 = now - timestamp;
                debug!("Ping response time {}", diff);
                let topic_id = self.get_client_topic_from_string("src/esp32/sys/latency");
                self.txd(ProxyMessage::Publish {
                    topic_id, // 1 is the topic id for the ping response
                    msg_id: self.msg_id,
                    flags: Flags(0),
                    data: payload_encode::<u64>(diff),
                });
            }
            ProxyMessage::Disconnect { duration: _ } => {
                info!("Disconnected");
                self.state = State::Disconnected;
                self.events.emit(PubSubEvent::Disconnected);
                self.server_topics.clear();
            }
            ProxyMessage::Register {
                topic_id,
                msg_id: _,
                topic_name,
            } => {
                info!("Registering topic {} with id {}", topic_name, topic_id);
                self.server_topics.insert(topic_id, topic_name);
            }
            ProxyMessage::Publish {
                flags: _,
                topic_id,
                msg_id,
                data,
            } => {
                info!("Received message on topic {} ", topic_id);
                if self.server_topics.contains_key(&topic_id) {
                    self.events.emit(PubSubEvent::Publish {
                        topic: self.server_topics.get(&topic_id).unwrap().clone(),
                        message: data,
                    });
                    self.txd(ProxyMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::Accepted,
                    });
                } else {
                    self.txd(ProxyMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code: ReturnCode::InvalidTopicId,
                    });
                }
            }
            ProxyMessage::PubAck {
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
                    self.txd(ProxyMessage::Register {
                        topic_id,
                        msg_id,
                        topic_name: topic_name.clone(),
                    });
                }
            }
            ProxyMessage::RegAck {
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

    pub async fn on_cmd_message(&mut self, cmd: PubSubCmd) {
        match cmd {
            PubSubCmd::Publish { topic, message } => {
                let full_topic = format!("src/{}/{}", self.client_id,topic);
                let topic_id = self.get_client_topic_from_string(&full_topic);
                let mut flags = Flags(0);
                flags.set_qos(0);
                self.txd(ProxyMessage::Publish {
                    flags,
                    topic_id,
                    msg_id: self.msg_id,
                    data: message,
                });
                self.msg_id += 1;
            }
            PubSubCmd::Subscribe { topic } => {
                self.msg_id += 1;
                let topic_id = self.get_client_topic_from_string(topic.as_str());
                self.txd(ProxyMessage::Register {
                    topic_id,
                    msg_id: self.msg_id,
                    topic_name: topic,
                });
            }
            PubSubCmd::Unsubscribe { topic } => {
                self.msg_id += 1;
                let topic_id = self.get_client_topic_from_string(topic.as_str());
                self.txd(ProxyMessage::PubAck {
                    topic_id,
                    msg_id: self.msg_id,
                    return_code: ReturnCode::Accepted,
                });
            }
            PubSubCmd::Connect { client_id } => {
                self.txd(ProxyMessage::Connect {
                    flags: Flags(0),
                    duration: 100,
                    client_id: client_id.clone(),
                });
            }
            PubSubCmd::Disconnect => {
                self.transport_cmd
                    .send(ProxyMessage::Disconnect { duration: 0 });
            }
            _ => {
                info!("Unexpected command {:?}", cmd);
            }
        }
    }
}

impl SourceTrait<PubSubEvent> for PubSubActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink);
    }
}
