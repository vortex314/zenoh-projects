use bytes::BytesMut;
use log::*;
use std::collections::BTreeMap;
use std::io;
use std::io::Write;
use std::result::Result;
use zenoh::buffers::ZSliceBuffer;

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
use crate::ActorTrait;
use crate::SourceTrait;
use protocol::msg::Flags;
use protocol::msg::MqttSnMessage;
use protocol::msg::ReturnCode;
use protocol::MessageDecoder;
use protocol::MTU_SIZE;

use crate::transport::*;

use minicbor::display;
use zenoh::open;
use zenoh::prelude::r#async::*;
use zenoh::subscriber::Subscriber;

#[derive(Clone)]
pub enum PubSubCmd {
    Publish { topic: String, message: Vec<u8> },
    Disconnect,
    Connect,
    Subscribe { topic: String },
}

#[derive(Clone, Debug)]
pub enum PubSubEvent {
    Connected,
    Disconnected,
    Publish { topic: String, message: Vec<u8> },
}

pub struct PubSubActor {
    cmds: Sink<PubSubCmd>,
    events: Source<PubSubEvent>,
    config: zenoh::config::Config,
}

impl PubSubActor {
    pub fn new() -> Self {
        let mut config = Config::from_file("./zenohd.json5");
        if config.is_err() {
            error!(
                "Error reading zenohd.json5 file, using default config {}",
                config.err().unwrap()
            );
            config = Ok(config::default());
        } else {
            info!("Using zenohd.json5 file");
        }
        PubSubActor {
            cmds: Sink::new(100),
            events: Source::new(),
            config: config.unwrap(),
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for PubSubActor {
    async fn run(&mut self) {
        let static_session: &'static mut Session =
            Session::leak(zenoh::open(config::default()).res().await.unwrap());
        loop {
            select! {
                cmd = self.cmds.next() => {
                    match cmd {
                        Some(PubSubCmd::Connect) => {
                            info!("Connecting to zenoh");
                            self.events.emit(PubSubEvent::Connected);
                        }
                        Some(PubSubCmd::Disconnect) => {
                            info!("Disconnecting from zenoh");
                            self.events.emit(PubSubEvent::Disconnected);
                        }
                        Some(PubSubCmd::Publish { topic, message}) => {
                            let s = format!("{}", minicbor::display(message.as_slice()));
                            let s :&str = s.as_str();
                            let v:Value = s.into();
                            info!("Publishing to zenoh: {}", v);
                            let _res = static_session
                                .put(&topic,v)
                                .encoding(KnownEncoding::TextPlain)
                                .res().await;
                        }
                        Some(PubSubCmd::Subscribe { topic }) => {
                            info!("Subscribing to zenoh");
                            let subscriber = static_session.declare_subscriber(&topic).res().await;
                            match subscriber {
                                Ok(sub) => {
                                    let emitter =  self.events.clone();
                                    tokio::spawn(async move {
                                        while let Ok(sample) = sub.recv_async().await {
                                            info!("Received: {}", sample);
                                            emitter.emit(PubSubEvent::Publish {
                                                topic: topic.clone(),
                                                message: vec![1,2,3],
                                            });
                                        };
                                    });
                                }
                                Err(e) => {
                                    error!("Error subscribing to zenoh: {}", e);
                                }
                            }
                        }
                        None => {
                            info!("PubSubActor::run() None");
                        }
                    }
                }
            }
        }
    }

    fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }
}

impl SourceTrait<PubSubEvent> for PubSubActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink);
    }
}
