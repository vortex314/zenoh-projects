use log::*;

use minicbor::display;
use minicbor::encode;

use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;

use zenoh::config;
use zenoh::open;

use anyhow::Result;
use zenoh::Config;
use zenoh::Session;

trait Handler<T>: Send + Sync {
    fn handle(&self, event: &T);
}

pub trait Actor {
    type Cmd;
    type Event;
    async fn run(&mut self);
    fn sender(&self) -> Result<Sender<Self::Cmd>>;
    fn add_listener(&mut self, listener: Box<dyn Handler<Self::Event> + 'static>);
}

#[derive(Debug, Serialize)]
pub enum ZenohCmd {
    Connect,
    Disconnect,
    Publish { topic: String, payload: Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
}

pub enum ZenohEvent {
    Connected,
    Disconnected,
    Publish { topic: String, payload: Vec<u8> },
}

pub struct ActorZenoh {
    tx_cmd: Sender<ZenohCmd>,
    rx_cmd: Receiver<ZenohCmd>,
    event_handlers: Vec<Box<dyn Handler<ZenohEvent>>>,
    config: Option<zenoh::config::Config>,
    zenoh_session: Option<Session>,
}

impl Actor for ActorZenoh {
    type Cmd = ZenohCmd;
    type Event = ZenohEvent;

     fn add_listener(&mut self, listener: Box<dyn Handler<Self::Event> + 'static>) {
        self.event_handlers.push(listener);
    }

    async fn run(&mut self) {
        let config = self.config.clone().unwrap();
        zenoh::init_log_from_env_or("info");
        let zenoh_session = zenoh::open(config).await.unwrap();


        let subscriber = zenoh_session.declare_subscriber("**").await.unwrap();
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
                            info!("From zenoh: {}:{}", topic,minicbor::display(&payload));
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

impl ActorZenoh {
    pub fn new() -> Self {
        let config = Config::from_file("./zenoh.json5").ok().unwrap();

        let (tx_cmd, rx_cmd) = tokio::sync::mpsc::channel(100);
        ActorZenoh {
            tx_cmd,
            rx_cmd,
            event_handlers: Vec::new(),
            config: Some(config),
            zenoh_session: None,
        }
    }
}
