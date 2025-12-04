use log::*;
use std::any::Any;

use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;

use anyhow::Result;
use zenoh::handlers::FifoChannelHandler;
use zenoh::pubsub::Subscriber;
use zenoh::sample::Sample;
use zenoh::Config;
use zenoh::Session;

use crate::handle_type;
use crate::limero::Msg;
use crate::limero::MulticastEvent;
use crate::limero::WifiEvent;
use crate::limero::SysEvent;
use crate::limero::ZenohEvent;
use crate::MyHandler;
use crate::MyActor;

#[derive(Serialize, Debug, Clone)]
pub enum ZenohCmd {
    Publish { topic: String, payload: Vec<u8> },
    // Subscribe { topic: String },
}

pub struct ZenohActor {
    name : String,
    config: Option<zenoh::config::Config>,
    zenoh_session: Option<Session>,
    subscriber: Option<Subscriber<FifoChannelHandler<Sample>>>,
    connected: bool,
}

impl ZenohActor {
    pub fn new() -> Self {
        let r = Config::from_file("./zenoh.json5");
        let config = Some(r.unwrap_or_else(|e| {
            error!("ZenohActor::new() error {}", e);
            panic!("ZenohActor::new() error {}", e);
        }));

        //   let mut config = Config::default();
        //     config.insert_json5("mode", r#""client""#).unwrap();
        //     config.insert_json5("connect/endpoints",r#"["tcp/limero.ddns.net:7447"]"#).unwrap();

        ZenohActor {
            name: "ZenohActor".to_string(),
            config,
            zenoh_session: None,
            subscriber: None,
            connected: false,
        }
    }

    async fn on_cmd(&mut self, cmd: &ZenohCmd) {
        info!("ActorBrain::run() cmd {:?}", cmd);
        match cmd {
            ZenohCmd::Publish { topic, payload } => {
                info!(
                    "ZenohActor::run() Publish {}  {:X?}",
                    topic,
                    &payload
                );
                if let Some(session) = &self.zenoh_session {
                    let topic = topic.clone();
                    let payload = payload.clone();

                    let _ = session.put(&topic, &payload).await;
                } else {
                    error!("ZenohActor::run() zenoh_session is None");
                }
            }

            _ => {
                info!("BrainActor::run() Unknown command");
            }
        }
    }

    async fn on_timer(&mut self, timer_name: &str) {
        match &timer_name[..] {
            "heartbeat" => {}
            _ => {
                info!("Unknown timer {}", timer_name);
            }
        }
    }

    async fn open_zenoh(&mut self) -> Result<()> {
        let config = self.config.clone().unwrap();
        zenoh::init_log_from_env_or("debug");
        let session = zenoh::open(config).await.unwrap();
        info!("Zenoh session opened");
        self.subscriber = session.declare_subscriber("**").await.ok();
        self.zenoh_session = Some(session);
        Ok(())
    }
}

impl MyHandler for ZenohActor {
    async fn handle(&mut self, event: &Box<dyn Any + Send + 'static>) {
        handle_type(event, |s: &String| {
            self.name = s.clone();
            info!("{} received String event: {}", self.name, s);
        });
        handle_type::<i32, _>(event, |i| {
            info!("{} received i32 event: {}", self.name, i);
        });
        handle_type::<MulticastEvent, _>(event, |mi| {
            info!("{} received MulticastEvent event: {:?}", self.name, mi);
        });
    }
}

impl MyActor for ZenohActor {
    fn tell(&self, cmd: ZenohCmd) {
        self.actor.tell(cmd)
    }

    fn sender(&self) -> Sender<ZenohCmd> {
        self.actor.sender()
    }

    fn on_event<FUNC: FnMut(&ZenohEvent) + 'static + Send>(&mut self, f: FUNC) -> () {
        self.actor.on_event(f);
    }

    async fn run(&mut self) {
        self.open_zenoh().await.unwrap();

        loop {
            select! {
                cmd = self.actor.rx_cmd.recv() => {
                    info!("ActorZenoh::run() cmd {:?}", cmd);
                    if let Some(c) = cmd {
                            self.on_cmd(&c).await;
                    }

                },
                timers = self.actor.timers.expired_timers() => {
                    for timer in timers {
                        self.on_timer(timer.as_str()).await;
                    }
                },
                msg = self.subscriber.as_mut().unwrap().recv_async() => {
                    match msg {
                        Ok(msg) => {
                            let topic = msg.key_expr().to_string();
                            let payload = msg.payload().to_bytes();
                            debug!("From zenoh: {}:{}", topic,minicbor::display(&payload));
                            self.actor.emit(&ZenohEvent::Publish {
                                    topic: topic.clone(),
                                    payload: payload.to_vec(),
                                });
                            },
                        Err(e) => {
                            info!("PubSubActor::run() error {} ",e);
                        }
                    }
                }
            }
        }
    }
}
