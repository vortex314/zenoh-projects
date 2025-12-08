use std::fmt::Debug;
use std::sync::Arc;

use log::*;

use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;

use anyhow::Result;
use async_trait::async_trait;
use zenoh::Config;
use zenoh::Session;
use zenoh::handlers::FifoChannelHandler;
use zenoh::pubsub::Subscriber;
use zenoh::sample;
use zenoh::sample::Sample;
use zenoh::session;

use crate::limero::LawnmowerManualCmd;
use crate::limero::Ps4Cmd;
use crate::limero::Ps4Event;
use crate::limero::WifiEvent;

use crate::eventbus::{ActorImpl, ActorStart, ActorStop, ActorTick, Bus, Eventbus, on_message};

#[derive(Debug, Serialize)]
pub enum ZenohCmd {
    Connect,
    Disconnect,
    Publish { topic: String, payload: Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
}
#[derive(Debug)]
pub enum ZenohEvent {
    Connected,
    Disconnected,
    Publish { topic: String, payload: Vec<u8> },
}

pub fn zenoh_to_bus<T>(bus: &Bus, pattern: &str, topic: &str, payload: &[u8])
where
    T: Send + Sync + Debug + 'static + serde::de::DeserializeOwned,
{
    if topic.ends_with(pattern) {
        if let Ok(msg) = serde_json::from_slice::<T>(payload) {
            info!("ZenohActor zenoh_to_bus() topic {} msg {:?}", topic, msg);
            bus.emit(msg);
        } else {
            error!(
                "ZenohActor zenoh_to_bus() failed to deserialize topic {}",
                topic
            );
        }
    } else {
        //   info!("ZenohActor zenoh_to_bus() topic {} does not match pattern {}", topic, pattern);
    }
}

async fn bus_to_zenoh<T>(session: &Session, topic: &str, msg: &T)
where
    T: Send + Sync + Debug + serde::Serialize,
{
    if let Ok(json) = serde_json::to_string(msg) {
        session.put(topic, json.as_bytes()).await;
    } else {
        error!(
            "ZenohActor bus_to_zenoh() failed to serialize msg {:?}",
            msg
        );
    }
}

pub struct ZenohActor {
    config: Option<zenoh::config::Config>,
    zenoh_session: Option<Session>,
    subscriber: Option<Subscriber<FifoChannelHandler<Sample>>>,
    connected: bool,
    bus: Bus,
    hostname: String,
}

impl ZenohActor {
    pub fn new(bus: Bus) -> Self {
        let r = Config::from_file("./zenoh.json5");
        let config = Some(r.unwrap_or_else(|e| {
            error!("ZenohActor::new() error {}", e);
            panic!("ZenohActor::new() error {}", e);
        }));

        //   let mut config = Config::default();
        //     config.insert_json5("mode", r#""client""#).unwrap();
        //     config.insert_json5("connect/endpoints",r#"["tcp/limero.ddns.net:7447"]"#).unwrap();

        ZenohActor {
            config,
            zenoh_session: None,
            subscriber: None,
            connected: false,
            bus,
            hostname: hostname::get().unwrap().to_str().unwrap().to_string(),
        }
    }

    async fn on_publish(&mut self, topic: &str, payload: &[u8]) {
        self.bus.emit(ZenohEvent::Publish {
            topic: topic.to_string(),
            payload: payload.to_vec(),
        });
    }

    fn on_cmd(&mut self, cmd: &ZenohCmd) {
        info!("ZenohActor::cmd() cmd {:?}", cmd);
        match cmd {
            ZenohCmd::Publish { topic, payload } => {
                if let Some(session) = &self.zenoh_session {
                    let topic = topic.clone();
                    let payload = payload.clone();

                    let _ = session.put(&topic, &payload);
                } else {
                    error!("ZenohActor::run() zenoh_session is None");
                }
            }

            _ => {
                info!("BrainActor::run() Unknown command");
            }
        }
    }

    async fn open_zenoh(&mut self) -> Result<()> {
        let config = self.config.clone().unwrap();
        zenoh::init_log_from_env_or("info");
        let session = zenoh::open(config).await.unwrap();
        info!("Zenoh session opened");
        self.zenoh_session = Some(session);
        self.start_subscriber().await?;
        Ok(())
    }

    async fn close_zenoh(&mut self) -> Result<()> {
        if let Some(session) = &self.zenoh_session {
            session.close();
            info!("Zenoh session closed");
        }
        self.zenoh_session = None;
        self.subscriber = None;
        Ok(())
    }

    async fn on_tick(&mut self) {
        info!("ZenohActor tick");
        // Perform periodic tasks here
    }

    async fn start_subscriber(&mut self) -> Result<()> {
        let bus_clone = self.bus.clone();
        if let Some(session) = &self.zenoh_session {
            info!("ZenohActor subscriber started");
            let session_clone = session.clone();
            tokio::spawn(async move {
                let subscriber = session_clone.declare_subscriber("**").await.unwrap();

                loop {
                    let sample = subscriber.recv_async().await;
                    match sample {
                        Ok(s) => {
                            let topic = s.key_expr().to_string();
                            let payload = s.payload().to_bytes();
                            std::str::from_utf8(&payload).map(|json| {
                                zenoh_to_bus::<WifiEvent>(
                                    &bus_clone,
                                    "WifiEvent",
                                    &topic,
                                    &payload,
                                );
                                zenoh_to_bus::<Ps4Cmd>(&bus_clone, "Ps4Cmd", &topic, &payload);
                            });

                            // Here you would typically send this to the event bus
                        }
                        Err(e) => {
                            // Subscriber error or closed
                            info!("ZenohActor subscriber error: {}", e);
                        }
                    }
                }
            });
        } else {
            error!("ZenohActor::start_subscriber() zenoh_session is None");
        }
        Ok(())
    }
}

#[async_trait]
impl ActorImpl for ZenohActor {
    async fn handle(&mut self, msg: &Arc<dyn std::any::Any + Send + Sync>) {
        if msg.is::<ActorStart>() {
            info!("ZenohActor received ActorStart");
            let _ = self.open_zenoh().await;
        } else if msg.is::<ActorStop>() {
            info!("ZenohActor received ActorStop");
            let _ = self.close_zenoh().await;
        } else if msg.is::<ZenohCmd>() {
            self.on_cmd(msg.downcast_ref::<ZenohCmd>().unwrap());
        } else if msg.is::<ActorTick>() {
            info!("ZenohActor received ActorTick");
            self.on_tick().await;
        } else if msg.is::<LawnmowerManualCmd>() {
            let cmd = msg.downcast_ref::<LawnmowerManualCmd>().unwrap();
            let topic = format!("src/{}/ps4/LawnmowerManualCmd", self.hostname);
            if let Some(session) = &self.zenoh_session {
                bus_to_zenoh(session, &topic, cmd).await;
            } else {
                error!("ZenohActor: no zenoh session to publish LawnmowerManualCmd");
            }
        } else if msg.is::<Ps4Event>() {
            let event = msg.downcast_ref::<Ps4Event>().unwrap();
            let topic = format!("src/{}/ps4/Ps4Event", self.hostname);
            if let Some(session) = &self.zenoh_session {
                // bus_to_zenoh(session, &topic, event).await;
            } else {
                error!("ZenohActor: no zenoh session to publish Ps4Event");
            }
        }
    }
}
