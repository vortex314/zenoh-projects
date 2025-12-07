use basu::async_trait;
// src/main.rs
use hidapi::{DeviceInfo, HidApi, HidDevice};
use serde::{Deserialize, Serialize};
use std::sync::Arc;
use std::time::Duration;
use std::time::Instant;
use zenoh_linux_eventbus_rs::eventbus::on_message;
use zenoh_linux_eventbus_rs::limero::LawnmowerManualCmd;

use log::info;
use log::warn;

use crate::eventbus::ActorStart;
use crate::eventbus::{ActorImpl, Bus};
use crate::limero::Ps4Cmd;
use crate::limero::Ps4Event;

// Moving average for float values
struct MovingAverage {
    values: Vec<f32>,
    window_size: usize,
}

impl MovingAverage {
    pub fn new(window_size: usize) -> Self {
        MovingAverage {
            values: Vec::with_capacity(window_size),
            window_size,
        }
    }

    pub fn add_value(&mut self, value: f32) {
        if self.values.len() == self.window_size {
            self.values.remove(0);
        }
        self.values.push(value);
    }

    pub fn average(&self) -> f32 {
        if self.values.is_empty() {
            return 0.0;
        }
        let sum: f32 = self.values.iter().sum();
        sum / self.values.len() as f32
    }
}

struct Ps4EventHandler {
     first_event_received: bool,
    last_cmd: Option<LawnmowerManualCmd>,
    last_event_time: std::time::Instant,
    interval: Duration,
    moving_average_steer: MovingAverage,
    moving_average_speed: MovingAverage,
}

impl Ps4EventHandler {
    pub fn new() -> Self {
        Ps4EventHandler {
            first_event_received: false,
            last_cmd: None,
            last_event_time: Instant::now(),
            interval: Duration::from_millis(1000),
            moving_average_steer: MovingAverage::new(10),
            moving_average_speed: MovingAverage::new(10),
        }
    }

    fn map_range(
        &self,
        value: i8,
        in_min: i8,
        in_max: i8,
        out_min: f32,
        out_max: f32,
    ) -> f32 {
        (value as f32 - in_min as f32) * (out_max - out_min) / (in_max as f32 - in_min as f32)
            + out_min
    }
    

    fn event_to_cmd(&self, event: &Ps4Event) -> LawnmowerManualCmd {
        let mut cmd = LawnmowerManualCmd::default();
        // Here you would implement the logic to convert Ps4Event to LawnmowerManualCmd
        cmd.steer = Some(event.axis_lx.unwrap() as f32 / 128.0); // Example conversion
        cmd.speed = Some(event.axis_ly.unwrap() as f32 / 128.0); // Example
        cmd.blade = event.button_circle;
        cmd
    }

    fn handle_event(&mut self, event: &Ps4Event) -> Option<LawnmowerManualCmd> {
        if !self.first_event_received {
            self.first_event_received = true;
            self.last_event_time = Instant::now();
            self.last_cmd = Some(self.event_to_cmd(&event));
            None
        } else {
            let cmd = self.event_to_cmd(&event);
            self.moving_average_steer
                .add_value(cmd.steer.unwrap_or(0.0));
            self.moving_average_speed
                .add_value(cmd.speed.unwrap_or(0.0));
            // Here you would implement the logic to convert Ps4Event to LawnmowerManualCmd

            if self.last_event_time.elapsed() < self.interval {
                return None;
            }
            let smoothed_cmd = LawnmowerManualCmd {
                steer: Some(self.moving_average_steer.average()),
                speed: Some(self.moving_average_speed.average()),
                blade: cmd.blade,
            };
            self.last_event_time = Instant::now();
            self.last_cmd = Some(smoothed_cmd.clone());
            Some(smoothed_cmd)
        }
    }
}

pub struct MapActor {
    bus: Bus,
    ps4_handler: Ps4EventHandler,
}

impl MapActor {
    pub fn new(bus: Bus) -> Self {
        MapActor {
            bus,
            ps4_handler: Ps4EventHandler::new(),
        }
    }

    pub fn on_start(&mut self) -> anyhow::Result<()> {
        info!("MapActor started");

        Ok(())
    }
}

#[async_trait]
impl ActorImpl for MapActor {
    async fn handle(&mut self, msg: &Arc<dyn std::any::Any + Send + Sync>) {
        if msg.is::<ActorStart>() {
            let _ = self.on_start();
        }
        if msg.is::<Ps4Event>() {
            let ps4_event = msg.downcast_ref::<Ps4Event>().unwrap();
            if let Some(cmd) = self.ps4_handler.handle_event(ps4_event) {
                info!("MapActor emitting LawnmowerManualCmd: {:?}", cmd);
                self.bus.emit(cmd);
                info!("MapActor emitted LawnmowerManualCmd");
            }
        }
    }
}
