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
use crate::actor::ActorImpl;
use crate::actor::Timers;

use minicbor::{encode::Write, Decode, Encode};

#[derive(Debug, Serialize, Clone)]
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
    Msg(BrainMsg),
}

#[derive(Debug)]
pub struct BrainMsg {
    utc: Option<u64>,
}

impl<C> Encode<C> for BrainMsg {
    fn encode<W: minicbor::encode::Write>(
        &self,
        e: &mut minicbor::Encoder<W>,
        ctx: &mut C,
    ) -> Result<(), minicbor::encode::Error<W::Error>> {
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
    pub actor: Actor<BrainCmd, BrainEvent>,
}

impl BrainActor {
    pub fn new() -> Self {
        BrainActor {
            actor: Actor::new(),
        }
    }

    async fn on_cmd(&mut self, cmd: &BrainCmd) {
        info!("ActorBrain::run() cmd {:?}", cmd);
        match cmd {
            BrainCmd::Publish { topic, payload } => {
                info!(
                    "BrainActor::run() Publish {} {} {:X?}",
                    topic,
                    minicbor::display(&payload),
                    &payload
                );
            }
            _ => {
                info!("BrainActor::run() Unknown command");
            }
        }
    }

    async fn on_timer(&mut self, timer_name: &str) {
        match &timer_name[..] {
            "heartbeat" => {
                self.actor.emit(&BrainEvent::Msg(BrainMsg {
                    utc: Some(chrono::Utc::now().timestamp_millis() as u64),
                }));
            }
            _ => {
                info!("Unknown timer {}", timer_name);
            }
        }
    }
}

impl ActorImpl<BrainCmd, BrainEvent> for BrainActor {
    async fn run(&mut self) {
        loop {
            select! {
                cmd = self.actor.rx_cmd.recv() => {
                    cmd.iter().for_each(|cmd| {
                        self.on_cmd(&cmd);
                })},
                timers = self.actor.timers.expired_timers() => {
                    for timer in timers {
                        self.on_timer(timer.as_str());
                    }
                }
            }
        }
    }
    fn tell(&self,cmd:BrainCmd) {
        self.actor.tell(cmd)
    }

    fn sender(&self)->Sender<BrainCmd>{
        self.actor.sender()
    }

    fn on_event<FUNC: FnMut(&BrainEvent) + 'static + Send>(&mut self, f: FUNC) -> () {
        self.actor.on_event(f);
    }


}
