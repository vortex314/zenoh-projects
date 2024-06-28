use embassy::executor::Spawner;
use embassy::time::Duration;
use embassy::util::Forever;
use esp32_hal::prelude::*;
use esp32_hal::gpio::{Output, OutputConfig, Pin};

use limero::Sink;
use limero::SinkTrait;

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
} ;

pub struct Led {
    commands: Sink<LedCmd,2>,
    timers: Timers,
    state: LedState,
    pin: AnyOutputPin + Send,
    pin_level_high : bool,
}

impl Led {
    pub fn new(pin: AnyOutputPin) -> Self {
        Self {
            commands: Sink::new(),
            timers: Timers::new(),
            state: LedState::OFF,
            pin,
        }
    }
}

impl  Led {

    pub async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(0,Duration::from_millis(1_000));
        loop {
        select! {
            _ = self.commands.read() => {
                match msg {
                    LedMsg::On => {self.state = LedState::ON;},
                    LedMsg::Off => {self.state = LedState::OFF;},
                    LedMsg::Blink { duration } => {self.state = LedState::BLINK { duration };
                self.set_led_high(true);
                self.timers.set_interval(0, Duration::from_millis(duration as u64));
            },
            LedMsg::Pulse { duration } => {
                self.state = LedState::PULSE { duration };
                self.set_led_high(true);
                self.timers.set_interval(0, Duration::from_millis(duration as u64));
                }
            }}
            _ = self.timers.alarm() => {
                match self.state {
                    LedState::BLINK { duration } => {
                        self.pin_level_high = !self.pin_level_high;
                        self.set_led_high(self.pin_level_high);
                    }
                    LedState::PULSE { duration } => {
                        self.set_led_high(false);
                    }
                    _ =>{}
                }
            }
        }
    }
    }

    fn set_led_high(&mut self, high : bool ) {
        if high {
            self.pin.set_high().unwrap();
        } else {
             self.pin.set_low().unwrap();
        }
    }

}
