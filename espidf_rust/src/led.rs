use crate::limero::timer::{Timer, Timers};
use crate::limero::Flow;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::SourceTrait;
use alloc::boxed::Box;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_time::Duration;
use embedded_hal::digital::v2::OutputPin;
use esp_hal::gpio::any_pin::AnyPin;
use esp_hal::prelude::*;
use esp_hal::{
    clock::ClockControl,
    gpio::{AnyOutput, Output, Pin},
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, Uart, UartRx, UartTx},
};
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

pub struct Led {
    commands: Sink<LedCmd, 2>,
    timers: Timers,
    state: LedState,
    pin: AnyOutput<'static>,
    pin_level_high: bool,
}

impl Led {
    pub fn new(pin: AnyOutput<'static>) -> Self {
        Self {
            commands: Sink::new(),
            timers: Timers::new(),
            state: LedState::BLINK { duration: 2000 },
            pin,
            pin_level_high: false,
        }
    }
}

impl Led {
    pub async fn run(&mut self) {
        self.timers
            .add_timer(Timer::new_repeater(0, Duration::from_millis(1_000)));
        loop {
            match select(self.commands.next(), self.timers.alarm()).await {
                First(msg) => {
                    self.on_cmd(msg.unwrap());
                }
                Second(id) => {
                    self.on_timer(id);
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
            self.pin.set_high()
        } else {
            self.pin.set_low()
        }
    }
    pub fn sink_ref(&self) -> SinkRef<LedCmd> {
        self.commands.sink_ref()
    }
}
