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
use msg::MetaPropertyId;
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

struct ObjectInfo {
    id: ObjectId,
    name: String,
    desc: String,
    prop: BTreeMap<i8, PropInfo>,
}

struct PropInfo {
    id: i8,
    name: String,
    desc: String,
    prop_type: Option<PropType>,
    prop_mode: Option<PropMode>,
}

pub struct ProxySession {
    event_handlers: EventHandlers<ProxyServerEvent>,
    cmds: CmdQueue<ProxyServerCmd>,
    transport_cmd: CmdQueue<TransportCmd>,
    pubsub_cmd: Box<dyn Handler<PubSubCmd>>,
    objects: BTreeMap<ObjectId, ObjectInfo>,
    client_id: Option<String>,
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
            objects: BTreeMap::new(),
            client_id: None,
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
                            debug!("Publishing message to zenoh");
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
                    debug!("Error handling message {:?}", r.err().unwrap().to_string());
                }
            }
            TransportEvent::ConnectionLost {} => {
                info!("Connection lost");
                self.event_handlers.handle(&ProxyServerEvent::Disconnected);
            }
        }
        Ok(())
    }

    fn id_to_topic(&self, object_id: ObjectId, prop_id: PropertyId) -> Result<String> {
        if self.objects.contains_key(&object_id) {
            let object_info = self.objects.get(&object_id).unwrap();
            if object_info.prop.contains_key(&prop_id) {
                let prop_info = object_info.prop.get(&prop_id).unwrap();
                Ok(format!("{}/{}", object_info.name, prop_info.name))
            } else {
                Err(anyhow::Error::msg(format!(
                    "Property {} not found for {} ",
                    prop_id, object_info.name
                )))
            }
        } else if prop_id == MetaPropertyId::RetCode as i8 {
            Ok("retcode".to_string())
        } else if prop_id == MetaPropertyId::MsgId as i8 {
            Ok("msg_id".to_string())
        } else {
            Err(anyhow::Error::msg(format!(
                "Object {} not found",
                object_id
            )))
        }
    }

    async fn on_transport_rxd(&mut self, payload: Vec<u8>) -> Result<()> {
        debug!(" CBOR : {}", minicbor::display(&payload));
        let mut decoder = minicbor::Decoder::new(&payload);
        let msg_header = decoder.decode::<MsgHeader>()?;
        let object_id = msg_header.src.unwrap();

        match msg_header.msg_type {
            MsgType::Pub => {
                debug!("Publish message");
                let tokens = decoder.tokens().collect();
                
                let _ = decoder.map()?;
                let mut unknown=0;
                let mut known=0;
                loop {
                    let dt = decoder.datatype()?;
                    if dt == Type::Break {
                        break;
                    }
                    let key: i8 = decoder.decode()?;
                    match self.id_to_topic(object_id, key) {
                        Ok(topic) => {
                            known+=1;
                            let t = decoder.datatype()?;
                            let message: Option<Vec<u8>> = match t {
                                Type::I32 => {
                                    let value = decoder.decode::<i32>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::U32 => {
                                    let value = decoder.decode::<u32>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::U16 => {
                                    let value = decoder.decode::<u16>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::I16 => {
                                    let value = decoder.decode::<i16>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::U8 => {
                                    let value = decoder.decode::<u8>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::I8 => {
                                    let value = decoder.decode::<i8>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                Type::String => {
                                    let value = decoder.decode::<String>()?;
                                    Some(minicbor::to_vec(&value)?)
                                }
                                _ => {
                                    decoder.skip()?;
                                    error!("Unexpected type {:?} for {}", t, topic);
                                    None
                                }
                            };
                            self.pubsub_cmd.handle(&PubSubCmd::Publish {
                                topic: format!("src/{}",topic.clone()),
                                payload: message.clone().unwrap(),
                            });
                        }
                        Err(e) => {
                            unknown+=1;
                            debug!("Uknown topic id  {} {}", e, key);
                            decoder.skip()?;
                        }
                    }
                }
                debug!("Known {} Unknown {}", known, unknown);
            }
            MsgType::Sub => {
                info!("Subscribe message");
            }
            MsgType::Alive => {
                debug!("Alive message");
            }
            MsgType::Info => {
                let msg_info = decoder.decode::<InfoMsg>()?;

                let obj_id = msg_header.src.unwrap();

                if !self.objects.contains_key(&msg_header.src.unwrap()) {
                    let object_info = ObjectInfo {
                        id: msg_header.src.unwrap(),
                        name: "".to_string(),
                        desc: "".to_string(),
                        prop: BTreeMap::new(),
                    };
                    self.objects.insert(msg_header.src.unwrap(), object_info);
                }
                if msg_info.id < 0 {
                    debug!("Object info");
                    let object_info = self.objects.get_mut(&obj_id).unwrap();
                    if object_info.name.is_empty() {
                        object_info.name = msg_info.name;
                        object_info.desc = msg_info.desc;
                        info!("Discovered object {}", object_info.name);
                    }
                } else {
                    let object_info = self.objects.get_mut(&obj_id).unwrap();
                    if !object_info.prop.contains_key(&msg_info.id) {
                        info!(
                            "Discovered property {} for object {}",
                            msg_info.name, object_info.name
                        );
                        let prop_info = PropInfo {
                            id: msg_info.id,
                            name: msg_info.name,
                            desc: msg_info.desc,
                            prop_type: msg_info.prop_type,
                            prop_mode: msg_info.prop_mode,
                        };
                        object_info.prop.insert(msg_info.id, prop_info);
                    }
                }
            }
        }
        Ok(())
    }
}
