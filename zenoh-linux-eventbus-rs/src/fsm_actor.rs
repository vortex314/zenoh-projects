use basu::async_trait;
use basu::event;
// src/main.rs

use serde::{Deserialize, Serialize};
use zenoh_linux_eventbus_rs::limero::LawnmowerManualCmd;
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;

use log::info;
use log::warn;
use statig::state_machine;

use crate::eventbus::ActorStart;
use crate::eventbus::{ActorImpl, Bus};
use crate::eventbus::as_message;
use crate::eventbus::on_message;

    enum MainState {
        #[initial]
        Idle,
        ManualControl,
        Autonomous,
    }

    enum FsmManual {
    RemoteControl,
    StraightLine,
}

enum FsmEvent {
    StartManualControl,
    StopManualControl,
    StartAutonomous,
    StopAutonomous,
    EmergencyStop,
}

#[derive(Default)]
struct Fsm {}



#[state_machine(initial = "Idle")]
impl Fsm {
    #[state]
    fn idle(event : &Event) -> Option<MainState> {
        match event {
            Event::StartManualControl => Some(MainState::ManualControl),
            Event::StartAutonomous => Some(MainState::Autonomous),
            _ => None,
        }
    }

    #[state]
    fn manual_control(event : &Event) -> Option<MainState> {
        match event {
            Event::StopManualControl => Some(MainState::Idle),
            Event::StartAutonomous => Some(MainState::Autonomous),
            _ => None,
        }
    }
    #[state]
    fn autonomous(event : &Event) -> Option<MainState> {
        match event {
            Event::StopAutonomous => Some(MainState::Idle),
            Event::StartManualControl => Some(MainState::ManualControl),
            _ => None,
        }
    }

}



enum FsmEvent {
    StartManualControl,
    StopManualControl,
    StartAutonomous,
    StopAutonomous,
}


pub struct FsmActor {
    bus: Bus,
    main_state : FsmMain,
    manual_state : FsmManual,
}

impl FsmActor {
    pub fn new(bus: Bus) -> Self {
        FsmActor {
            bus,
            main_state : FsmMain::Idle,
            manual_state : FsmManual::RemoteControl,
        }
    }

    fn manual_cmd_handler(&mut self, cmd : LawnmowerManualCmd) {
        info!("FsmActor received LawnmowerManualCmd: {:?}", cmd);
        // Handle the command and update FSM states as needed

    }

    pub fn on_start(&mut self, start : ActorStart)  {
        info!("FsmActor started");

        Ok(())
    }
}

#[async_trait]
impl ActorImpl for FsmActor {
    async fn handle(&mut self, msg: &Arc<dyn std::any::Any + Send + Sync>) {

        as_message::<LawnmowerManualCmd>(msg)
            .map(|cmd| self.manual_cmd_handler(cmd));

        as_message::<ActorStart>(msg)
            .map(|ev| self.on_start(ev) );
    }
}
