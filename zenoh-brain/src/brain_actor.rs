use std::sync::Arc;
use std::time::Duration;

use log::*;

use serde::Deserialize;
use serde::Serialize;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;

use anyhow::Result;
use zenoh::Config;
use zenoh::Session;

use crate::actor::Actor;
use crate::actor::Timers;

use minicbor::{Decode, Encode, encode::Write};

#[derive(Debug, Serialize)]
pub enum BrainCmd {
    Connect,
    Disconnect,
    Publish { topic: String, payload: Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
}
#[derive(Debug)]
pub enum BrainEvent {
    Connected,
    Disconnected,
    Publish { topic: String, payload: Vec<u8> },
    Msg ( BrainMsg )
}

#[derive(Debug)]
pub struct BrainMsg {
    utc: Option<u64>,
}

impl<C> Encode<C> for BrainMsg {
    fn encode<W:minicbor::encode::Write,>(&self, e: &mut minicbor::Encoder<W>,ctx:&mut C) -> Result<(), minicbor::encode::Error<W::Error>> {
        e.begin_map()?;
        if let Some(utc) = self.utc {
            e.str("utc")?;
            e.u64(utc)?;
        };
        e.end()?;
        Ok(())
    }
    
    fn is_nil(&self) -> bool {
        false
    }
}

pub struct BrainActor {
    tx_cmd: Sender<BrainCmd>,
    rx_cmd: Receiver<BrainCmd>,
    event_handlers: Vec<Box<dyn FnMut(&BrainEvent) + Send>>,
    timers: Timers,
}

impl Actor for BrainActor {
    type Cmd = BrainCmd;
    type Event = BrainEvent;

    fn add_listener<FUNC: FnMut(&Self::Event) + 'static + Send>(&mut self, f: FUNC) {
        self.event_handlers.push(Box::new(f));
    }

    async fn run(&mut self) -> Result<()> {
        loop {
            select! {
                cmd = self.rx_cmd.recv() => {
                    info!("ActorBrain::run() cmd {:?}", cmd);
                    match cmd {
                        Some(BrainCmd::Publish { topic, payload }) => {
                            info!("BrainActor::run() Publish {} {} {:X?}", topic, minicbor::display(&payload) , &payload);

                        }
                        _ => {
                            info!("BrainActor::run() Unknown command");
                        }
                    }
                },
                expired_timers  = self.timers.expired_timers() => {
                    for timer_name in expired_timers {
                        match &timer_name[..] {
                            "heartbeat" => {
                                info!("Heartbeat");
                                for handler in &mut self.event_handlers {
                                    handler(&BrainEvent::Msg (BrainMsg {
                                        utc: Some(chrono::Utc::now().timestamp_millis() as u64),
                                    }));
                                }

                            }
                            _ => {
                                info!("Unknown timer {}", timer_name);
                            }
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

impl BrainActor {
    pub fn new() -> Self {
        let (tx_cmd, rx_cmd) = tokio::sync::mpsc::channel(100);
        let mut timers = Timers::new();
        timers.add_repeat_timer("heartbeat".to_string(), std::time::Duration::from_secs(1));
        BrainActor {
            tx_cmd,
            rx_cmd,
            event_handlers: Vec::new(),
            timers,
        }
    }
}
