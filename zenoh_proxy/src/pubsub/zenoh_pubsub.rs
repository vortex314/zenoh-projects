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

use crate::limero::ActorTrait;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::limero::SourceTrait;

use crate::pubsub::payload_display;
use crate::pubsub::{PubSubCmd, PubSubEvent};
use minicbor::display;
use zenoh::open;
use zenoh::prelude::r#async::*;
use zenoh::subscriber::Subscriber;

pub struct ZenohPubSubActor {
    cmds: Sink<PubSubCmd>,
    events: Source<PubSubEvent>,
    config: zenoh::config::Config,
}

impl ZenohPubSubActor {
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
        ZenohPubSubActor {
            cmds: Sink::new(100),
            events: Source::new(),
            config: config.unwrap(),
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for ZenohPubSubActor {
    async fn run(&mut self) {
        let static_session: &'static mut Session =
            Session::leak(zenoh::open(config::default()).res().await.unwrap());
        let subscriber = static_session.declare_subscriber("**").res().await.unwrap();
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
                        Some(PubSubCmd::Publish { topic, payload}) => {
                            info!("To zenoh: {}:{}", topic,payload_display(&payload));
                            let _res = static_session
                                .put(&topic,payload.as_slice())
                                .encoding(KnownEncoding::AppOctetStream)
                                .res().await;
                        }
                        Some(PubSubCmd::Subscribe { topic }) => {
                            info!("Subscribing to zenoh");
                            let subscriber = static_session.declare_subscriber(&topic).res().await;
                            match subscriber {
                                Ok(sub) => {

                                }
                                Err(e) => {
                                    error!("Error subscribing to zenoh: {}", e);
                                }
                            }
                        }
                        Some(PubSubCmd::Unsubscribe { topic }) => {
                            info!("Unsubscribing from zenoh");
                           // let _res = static_session.remove_subscriber(&topic).res().await;
                        }
                        None => {
                            info!("PubSubActor::run() None");
                        }
                    }
                },
                msg = subscriber.recv_async() => {
                    match msg {
                        Ok(msg) => {
                            let topic = msg.key_expr.to_string();
                            let payload = msg.payload.contiguous().to_vec();
                            info!("From zenoh: {}:{}", topic,payload_display(&payload));
                            let event = PubSubEvent::Publish { topic, payload };
                            self.events.emit(event);
                        }
                        Err(e) => {
                            info!("PubSubActor::run() error {} ",e);
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

impl SourceTrait<PubSubEvent> for ZenohPubSubActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink);
    }
}
