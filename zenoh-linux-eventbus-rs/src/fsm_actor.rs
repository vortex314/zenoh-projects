use basu::async_trait;
use basu::event;
// src/main.rs

use serde::{Deserialize, Serialize};
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;
use zenoh_linux_eventbus_rs::limero::LawnmowerManualCmd;

use log::info;
use log::warn;


use crate::eventbus::ActorStart;
use crate::eventbus::as_message;
use crate::eventbus::on_message;
use crate::eventbus::{ActorImpl, Bus};

pub struct FsmActor {
    bus: Bus,
}

impl FsmActor {
    pub fn new(bus: Bus) -> Self {
        FsmActor {
            bus,

        }
    }

    fn manual_cmd_handler(&mut self, cmd: &LawnmowerManualCmd) {
        info!("FsmActor received LawnmowerManualCmd: {:?}", cmd);
        // Handle the command and update FSM states as needed
    }

    pub fn on_start(&mut self, start: &ActorStart) {
        info!("FsmActor started");
    }
}

#[async_trait]
impl ActorImpl for FsmActor {
    async fn handle(&mut self, msg: &Arc<dyn std::any::Any + Send + Sync>) {
        as_message::<LawnmowerManualCmd>(msg).map(|cmd| self.manual_cmd_handler(cmd));
        as_message::<ActorStart>(msg).map(|ev| self.on_start(ev));
    }
}
