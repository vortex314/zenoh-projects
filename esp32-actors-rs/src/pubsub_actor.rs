use alloc::boxed::Box;
use alloc::format;
use alloc::string::ToString;
use core::option::Option::Some;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};

use alloc::collections::BTreeMap;
use alloc::string::String;

use embassy_time::{Duration, Instant};
// use esp_backtrace as _;
use log::{debug, info};

use crate::proxy_message::{Flags, ProxyMessage, ReturnCode};
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::pubsub::PubSubCmd;
use msg::pubsub::PubSubEvent;

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
}

pub struct PubSubActor {
    cmds: CmdQueue<PubSubCmd>,
    events: EventHandlers<PubSubEvent>,
    timers: Timers,
    transport_cmd: Box<dyn Handler<ProxyMessage>>,
    rxd_msg: CmdQueue<ProxyMessage>,
    client_topics: BTreeMap<String, u16>,
    server_topics: BTreeMap<u16, String>,
    client_topics_registered: BTreeMap<u16, bool>,
    //    client_topics_sender: BTreeMap<u16, Box<dyn Subscriber>>,
    client_id: String,
    state: State,
    ping_timeouts: u32,
    msg_id: u16,
    topic_id_counter: u16,
}
impl PubSubActor {
    pub fn new(txd_msg: Box<dyn Handler<ProxyMessage>>) -> PubSubActor {
        PubSubActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            transport_cmd: txd_msg,
            rxd_msg: CmdQueue::new(5),
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

    pub fn transport_in(&mut self) -> Box<dyn Handler<ProxyMessage>> {
        self.rxd_msg.handler()
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
        self.transport_cmd.handle(&msg);
    }

    async fn on_timeout(&mut self, id: u32) {
        if id == TimerId::ConnectTimer as u32 {
            if self.state == State::Disconnected {
                self.txd(ProxyMessage::SearchGateway);
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
                    self.events.handle(&PubSubEvent::Disconnected);
                }
            }
        } else {
            info!("Unexpected timer id {}", id);
            self.ping_timeouts += 1;
            if self.ping_timeouts > 3 {
                self.transport_cmd
                    .handle(&ProxyMessage::Disconnect { duration: 0 });
            }
        }
    }

    async fn on_rxd_message(&mut self, msg: ProxyMessage) {
        match msg {
            ProxyMessage::ConnAck { return_code } => {
                info!("Connected code  {:?}", return_code);
                if self.state == State::Disconnected {
                    self.state = State::Connected;
                    self.events.handle(&PubSubEvent::Connected);
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
                    data: msg::cbor::encode::<u64>(&diff),
                });
            }
            ProxyMessage::Disconnect { duration: _ } => {
                info!("Disconnected");
                self.state = State::Disconnected;
                self.events.handle(&PubSubEvent::Disconnected);
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
                    self.events.handle(&PubSubEvent::Publish {
                        topic: self.server_topics.get(&topic_id).unwrap().clone(),
                        payload: data,
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
            PubSubCmd::Publish { topic, payload } => {
                let full_topic = format!("src/{}/{}", self.client_id, topic);
                let topic_id = self.get_client_topic_from_string(&full_topic);
                let mut flags = Flags(0);
                flags.set_qos(0);
                self.txd(ProxyMessage::Publish {
                    flags,
                    topic_id,
                    msg_id: self.msg_id,
                    data: payload,
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
                    .handle(&ProxyMessage::Disconnect { duration: 0 });
            }
        }
    }
}

impl Actor<PubSubCmd, PubSubEvent> for PubSubActor {
    async fn run(&mut self) {
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
            match select3(self.cmds.next(), self.timers.alarm(), self.rxd_msg.next()).await {
                First(msg) => match msg {
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
                Third(msg) => match msg {
                    Some(cmd) => {
                        self.on_rxd_message(cmd).await;
                    }
                    None => {
                        info!("Unexpected {:?}", msg);
                    }
                },
            }
        }
    }
    fn add_listener(&mut self, listener: Box<dyn Handler<PubSubEvent>>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<PubSubCmd>> {
        self.cmds.handler()
    }
}
