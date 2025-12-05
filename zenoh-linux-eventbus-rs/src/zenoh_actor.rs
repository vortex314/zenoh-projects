use std::sync::Arc;

use log::*;

use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;

use anyhow::Result;
use zenoh::handlers::FifoChannelHandler;
use zenoh::pubsub::Subscriber;
use zenoh::sample;
use zenoh::sample::Sample;
use zenoh::Config;
use zenoh::Session;
use async_trait::async_trait;
use zenoh::session;

use crate::eventbus::{on_message, ActorImpl, ActorStart, ActorStop, ActorTick,Bus, Eventbus};

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

pub struct ZenohActor {
    config: Option<zenoh::config::Config>,
    zenoh_session: Option<Session>,
    subscriber: Option<Subscriber<FifoChannelHandler<Sample>>>,
    connected: bool,
    bus: Bus,
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
        zenoh::init_log_from_env_or("debug");
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
                            info!("ZenohActor received sample on topic {} with payload size {}", topic, payload.len());
                                // Here you would typically send this to the event bus
                        },
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
        on_message::<ActorStart, _> (msg, |m| async {
            info!("ZenohActor received ActorStart");
            let _ = self.open_zenoh().await;
        }.await);
        on_message::<ActorTick, _>(msg, | _ | async {
            info!("ZenohActor received ActorTick");
            self.on_tick().await;
        }.await);
        on_message::<ActorStop, _>(msg, | _ | async {
            info!("ZenohActor received ActorStop");
            let _ = self.close_zenoh().await;
        }.await);
        on_message::<ZenohCmd, _>(msg, |cmd| self.on_cmd(cmd) );
    }
}
