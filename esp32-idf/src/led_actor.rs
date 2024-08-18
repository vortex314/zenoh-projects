use std::borrow::BorrowMut;
use std::time::Duration;

use crate::limero::timer::{Timer, Timers};
use crate::limero::Flow;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::SourceTrait;
use esp_idf_hal::gpio::*; 

use futures::FutureExt;
use log::info;
use minicbor::decode::info;

#[derive(Clone)]
pub enum LedCmd {
    On,
    Off,
    Blink { duration: u32 },
    Pulse { duration: u32 },
}

enum LedState {
    ON,
    OFF,
    BLINK { duration: u32 },
    PULSE { duration: u32 },
}

pub struct LedActor {
    commands: Sink<LedCmd>,
    timers: Timers,
    state: LedState,
    pin_driver: PinDriver<'static,AnyOutputPin,Output>,
    pin_level_high: bool,
}

impl LedActor {
    pub fn new(pin: AnyOutputPin) -> Self {
        let pin_driver = PinDriver::output(pin).unwrap();
        Self {
            commands: Sink::new(),
            timers: Timers::new(),
            state: LedState::BLINK { duration: 50 },
            pin_driver,
            pin_level_high: false,
        }
    }
}

impl LedActor {
    pub async fn run(&mut self) {
        self.timers
            .add_timer(Timer::new_repeater(0, Duration::from_millis(1_000)));
        loop {
            futures::select!{
                cmd = self.commands.next().fuse() => {
                    if let Some(cmd) = cmd {
                        self.on_cmd(cmd);
                    }
                }
                _ = self.timers.alarm().fuse() => {
                    self.on_timer(0);
                }
            }
        }
    }

    fn on_cmd(&mut self, msg: LedCmd) {
        match msg {
            LedCmd::On => {
                self.state = LedState::ON;
            }
            LedCmd::Off => {
                self.state = LedState::OFF;
            }
            LedCmd::Blink { duration } => {
                self.state = LedState::BLINK { duration };
                self.set_led_high(true);
                self.timers
                    .set_interval(0, Duration::from_millis(duration as u64));
            }
            LedCmd::Pulse { duration } => {
                self.state = LedState::PULSE { duration };
                self.set_led_high(true);
                self.timers
                    .set_interval(0, Duration::from_millis(duration as u64));
            }
        }
    }

    fn on_timer(&mut self, _id: u32) {
        match self.state {
            LedState::BLINK { duration: _ } => {
                self.pin_level_high = !self.pin_level_high;
                self.set_led_high(self.pin_level_high);
            }
            LedState::PULSE { duration: _ } => {
                self.set_led_high(false);
            }
            _ => {}
        }
    }

    fn set_led_high(&mut self, high: bool) {
        if high {
            let _ = self.pin_driver.set_high();
        } else {
            let _ = self.pin_driver.set_low();
        }
    }
    pub fn sink_ref(&self) -> SinkRef<LedCmd> {
        self.commands.sink_ref()
    }
}
