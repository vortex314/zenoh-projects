use egui::ahash::HashMap;
use log::*;


use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;


use anyhow::Result;
use zenoh::Config;
use zenoh::Session;

trait Handler<T>: Send + Sync {
    fn handle(&self, event: &T);
}

pub trait Actor {
    type Cmd;
    type Event;
    async fn run(&mut self)-> Result<()>;
    fn sender(&self) -> Result<Sender<Self::Cmd>>;
    fn add_listener<FUNC: FnMut(&Self::Event) + 'static + Send >(&mut self, f:FUNC ) -> ();
}

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
    tx_cmd: Sender<ZenohCmd>,
    rx_cmd: Receiver<ZenohCmd>,
    event_handlers: Vec<Box<dyn FnMut(&ZenohEvent) + Send >>,
    config: Option<zenoh::config::Config>,
    zenoh_session: Option<Session>,
}

impl Actor for ZenohActor {
    type Cmd = ZenohCmd;
    type Event = ZenohEvent;

     fn add_listener<FUNC: FnMut(&Self::Event) + 'static + Send >(&mut self, f:FUNC) {
        self.event_handlers.push(Box::new(f));
    }

    async fn run(&mut self) -> Result<()> {
        let config = self.config.clone().unwrap();
        zenoh::init_log_from_env_or("debug");
        let zenoh_session = zenoh::open(config).await.map_err(|e| anyhow::anyhow!(e))?;

        let subscriber = zenoh_session.declare_subscriber("**")
        .await.unwrap();

        self.zenoh_session = Some(zenoh_session);

        loop {
            select! {
                cmd = self.rx_cmd.recv() => {
                    info!("ActorZenoh::run() cmd {:?}", cmd);
                },
                msg = subscriber.recv_async() => {
                    match msg {
                        Ok(msg) => {
                            let topic = msg.key_expr().to_string();
                            let payload = msg.payload().to_bytes();
                            debug!("From zenoh: {}:{}", topic,minicbor::display(&payload));
                            for handler in self.event_handlers.iter_mut() {
                                handler(&ZenohEvent::Publish {
                                    topic: topic.clone(),
                                    payload: payload.to_vec()   ,
                                });
                            }       
                        }
                        Err(e) => {
                            info!("PubSubActor::run() error {} ",e);
                        }
                    }
                }
            }
        }
    }

    fn sender(&self) -> Result<Sender<Self::Cmd>> {
        Ok(self.tx_cmd.clone())
    }
}

impl ZenohActor {
    pub fn new() -> Self {
        //let config = Config::from_file("./zenoh.json5").ok().unwrap();
        let mut config = Config::default();
        config.insert_json5("mode", r#""client""#).unwrap();
        config.insert_json5("connect/endpoints",r#"["tcp/limero.ddns.net:7447"]"#).unwrap();

        let (tx_cmd, rx_cmd) = tokio::sync::mpsc::channel(100);
        ZenohActor {
            tx_cmd,
            rx_cmd,
            event_handlers: Vec::new(),
            config: Some(config),
            zenoh_session: None,
        }
    }
}
