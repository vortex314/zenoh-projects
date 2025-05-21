use log::*;

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

use crate::actor::Actor;
use crate::actor::ActorImpl;

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
    actor: Actor<ZenohCmd, ZenohEvent>,
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
            actor: Actor::new(),
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
                    "ZenohActor::run() Publish {} {} {:X?}",
                    topic,
                    minicbor::display(&payload),
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

impl ActorImpl<ZenohCmd, ZenohEvent> for ZenohActor {
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
