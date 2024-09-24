use bytes::BytesMut;
use limero::Actor;
use limero::CmdQueue;
use limero::EventHandlers;
use limero::Handler;
use log::*;
use msg::EspNowMessage;
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

use crate::pubsub::PubSubCmd;
use crate::pubsub::PubSubEvent;

use crate::transport::*;
use crate::zenoh_pubsub::*;

use zenoh::open;
use zenoh::prelude::r#async::*;
use zenoh::subscriber::Subscriber;
use zenoh::Session;

const GREEN: &str = "\x1b[0;32m";
const RESET: &str = "\x1b[m";
const MTU_SIZE: usize = 1023;

#[derive(Clone)]
pub enum ProxyServerEvent {
    Connected,
    Disconnected,
    Publish { topic: String, message: Vec<u8> },
}

#[derive(Clone)]
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
    event_handlers: EventHandlers<ProxyServerEvent>,
    cmds: CmdQueue<ProxyServerCmd>,
    transport_cmd: CmdQueue<TransportCmd>,
    pubsub_cmd: Box<dyn Handler<PubSubCmd>>,
    client_topics: BTreeMap<u16, String>,
    server_topics: BTreeMap<String, u16>,
    will_topic: Option<String>,
    will_message: Option<Vec<u8>>,
    topic_id_counter: u16,
    client_id: Option<String>,
}

impl ProxySession {
    pub fn new(
        pubsub_cmd: Box<dyn Handler<PubSubCmd>>,
        _transport_cmd: Box<dyn Handler<TransportCmd>>,
    ) -> Self {
        let commands = CmdQueue::new(100);
        let events = EventHandlers::new();

        ProxySession {
            event_handlers: events,
            cmds: commands,
            transport_cmd: CmdQueue::new(100),
            pubsub_cmd,
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            will_topic: None,
            will_message: None,
            topic_id_counter: 0,
            client_id: None,
        }
    }

    pub fn transport_send(&self, message: EspNowMessage) {
        self.transport_cmd
            .handle(&TransportCmd::SendMessage(message));
    }

    fn get_server_topic_from_string(&mut self, topic: &str) -> u16 {
        match self.server_topics.get(topic) {
            Some(topic_id) => *topic_id,
            None => {
                self.topic_id_counter += 1;
                let topic_id = self.topic_id_counter;
                self.server_topics.insert(topic.to_string(), topic_id);
                topic_id
            }
        }
    }
}

impl Actor<ProxyServerCmd, ProxyServerEvent> for ProxySession {
    async fn run(&mut self) {
        self.pubsub_cmd.handle(&PubSubCmd::Connect);

        let _buf = vec![0u8; MTU_SIZE];
        loop {
            select! {
                cmd = self.cmds.next() => {
                    let cmd = cmd.unwrap();
                    match cmd{
                        ProxyServerCmd::Publish { topic, message } => {
                            debug!("Publishing message to zenoh");
                            self.pubsub_cmd.handle(&PubSubCmd::Publish { topic, payload: message });
                        },
                        ProxyServerCmd::TransportEvent(event) => {
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

    fn handler(&self) -> Box<dyn Handler<ProxyServerCmd>> {
        self.cmds.handler()
    }

    fn add_listener(&mut self, handler: Box<dyn Handler<ProxyServerEvent>>) {
        self.event_handlers.add_listener(handler);
    }
}

impl ProxySession {
    fn on_pubsub_event(&mut self, event: PubSubEvent) {
        match event {
            PubSubEvent::Connected => {
                info!("Connected to zenoh");
                self.event_handlers.handle(&ProxyServerEvent::Connected);
            }
            PubSubEvent::Disconnected => {
                info!("Disconnected from zenoh");
                self.event_handlers.handle(&ProxyServerEvent::Disconnected);
            }
            PubSubEvent::Publish { topic, payload:_ } => {
                if !self.server_topics.contains_key(&topic) {
                    let _topic_id = self.get_server_topic_from_string(&topic);
                    /*self.transport_send(&EspNowMessage::Register {
                        topic_id,
                        msg_id: 0,
                        topic_name: topic.clone(),
                    });*/
                }
                let _topic_id = self.get_server_topic_from_string(&topic);
                /*self.transport_send(&EspNowMessage::Publish {
                    flags: Flags(0),
                    topic_id,
                    msg_id: 0,
                    data: payload,
                });*/
            }
        }
    }

    async fn on_transport_event(&mut self, event: TransportEvent) {
        match event {
            TransportEvent::RecvMessage ( message ) => {
                self.on_transport_rxd(message).await;
            }
            TransportEvent::ConnectionLost {} => {
                info!("Connection lost");
                self.event_handlers.handle(&ProxyServerEvent::Disconnected);
            }
        }
    }

    async fn on_transport_rxd(&mut self, _event: EspNowMessage) {}
}
