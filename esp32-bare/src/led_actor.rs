use esp_hal::gpio::{AnyOutput, Output, Pin};
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler,Endpoint};
use embassy_time::Duration;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};

#[derive(Clone)]
pub enum LedCmd {
    On,
    Off,
    Blink { duration: u32 },
    Pulse { duration: u32 },
}

pub enum LedEvent {}

enum LedState {
    ON,
    OFF,
    BLINK { duration: u32 },
    PULSE { duration: u32 },
}

pub struct LedActor {
    cmds: CmdQueue<LedCmd>,
    events: EventHandlers<LedEvent>,
    timers: Timers,
    state: LedState,
    pin: AnyOutput<'static>,
    pin_level_high: bool,
}

impl LedActor {
    pub fn new(pin: AnyOutput<'static>) -> Self {
        Self {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            state: LedState::BLINK { duration: 2000 },
            pin,
            pin_level_high: false,
        }
    }
}

impl Actor<LedCmd, LedEvent> for LedActor {
    async fn run(&mut self) {
        self.timers
            .add_timer(Timer::new_repeater(0, Duration::from_millis(1_000)));
        loop {
            match select(self.cmds.next(), self.timers.alarm()).await {
                First(msg) => {
                    self.on_cmd(msg.unwrap());
                }
                Second(id) => {
                    self.on_timer(id);
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Endpoint<LedEvent>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Endpoint<LedCmd> {
        self.cmds.handler()
    }
}
impl LedActor {
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

}
