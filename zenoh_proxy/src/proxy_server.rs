use anyhow::Result;
use bytes::BytesMut;
use limero::Actor;
use limero::CmdQueue;
use limero::EventHandlers;
use limero::Handler;
use log::*;
use minicbor::data::Type;
use minicbor::decode::info;
use minicbor::encode;
use msg::EspNowMessage;
use msg::InfoMsg;
use msg::Msg;
use msg::MsgHeader;
use msg::MsgType;
use msg::ObjectId;
use msg::PropMode;
use msg::PropType;
use msg::PropertyId;
use std::collections::BTreeMap;
use std::io;
use std::io::Write;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio::select;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};

use crate::pubsub::PubSubCmd;
use crate::pubsub::PubSubEvent;

use crate::transport::*;
use crate::zenoh_pubsub::*;
use crate::Translator;

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

pub struct ProxySession {
    event_handlers: EventHandlers<ProxyServerEvent>,
    cmds: CmdQueue<ProxyServerCmd>,
    transport_cmd: CmdQueue<TransportCmd>,
    pubsub_cmd: Box<dyn Handler<PubSubCmd>>,
    client_id: Option<String>,
    translator: Translator,
}

fn bytes_to_string(bytes: &[u8]) -> String {
    let mut s = String::new();
    for b in bytes {
        s.push_str(&format!("{:02X} ", b));
    }
    s
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
            client_id: None,
            translator: Translator::new(),
        }
    }

    pub fn transport_send(&self, message: Vec<u8>) {
        self.transport_cmd
            .handle(&TransportCmd::SendMessage(message));
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
                            info!("Publishing message to zenoh");
                            self.pubsub_cmd.handle(&PubSubCmd::Publish { topic, payload: message });
                        },
                        ProxyServerCmd::TransportEvent(event) => {
                             let _ = self.on_transport_event(event).await;
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
            PubSubEvent::Publish {
                topic: _,
                payload: _,
            } => {

                /*self.transport_send(&EspNowMessage::Publish {
                    flags: Flags(0),
                    topic_id,
                    msg_id: 0,
                    data: payload,
                });*/
            }
        }
    }

    async fn on_transport_event(&mut self, event: TransportEvent) -> Result<()> {
        match event {
            TransportEvent::RecvMessage(message) => {
                let r = self.on_transport_rxd(message).await;
                if r.is_err() {
                    info!("Error handling message {:?}", r.err().unwrap().to_string());
                }
            }
            TransportEvent::ConnectionLost {} => {
                info!("Connection lost");
                self.event_handlers.handle(&ProxyServerEvent::Disconnected);
            }
        }
        Ok(())
    }

    async fn on_transport_rxd(&mut self, binary_msg: Vec<u8>) -> Result<()> {
        debug!(" CBOR : {}", minicbor::display(&binary_msg));
        let mut decoder = minicbor::Decoder::new(&binary_msg);
        let msg = decoder.decode::<Msg>()?;
        self.translator.analyze(&msg);
        if msg.publish.is_some() {
       //     let (key, object) = self.translator.translate_to_object(&msg)?;
            self.translator.translate_to_array(&msg)?.iter().for_each(|(key, object)| {
                self.pubsub_cmd.handle(&PubSubCmd::Publish {
                    topic: key.clone(),
                    payload : object.clone(),
                });
            });
        };
        

        Ok(())
    }
}
