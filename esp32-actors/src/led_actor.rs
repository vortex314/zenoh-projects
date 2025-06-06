use esp_hal::gpio::Output;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers,Endpoint};
use embassy_time::Duration;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};

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
    BLINK ,
    PULSE ,
}

pub struct LedActor {
    cmds: CmdQueue<LedCmd>,
    events: EventHandlers<LedEvent>,
    timers: Timers,
    state: LedState,
    pin: Output<'static>,
    pin_level_high: bool,
}

impl LedActor {
    pub fn new(pin: Output<'static>) -> Self {
        Self {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            state: LedState::ON ,
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
                self.state = LedState::BLINK ;
                self.set_led_high(true);
                self.timers
                    .set_interval(0, Duration::from_millis(duration as u64));
            }
            LedCmd::Pulse { duration } => {
                self.state = LedState::PULSE ;
                self.set_led_high(true);
                self.timers
                    .set_interval(0, Duration::from_millis(duration as u64));
                self.timers.start(0);
            }
        }
    }

    fn on_timer(&mut self, _id: u32) {
        match self.state {
            LedState::BLINK  => {
                self.pin_level_high = !self.pin_level_high;
                self.set_led_high(self.pin_level_high);
            }
            LedState::PULSE  => {
                self.set_led_high(false);
                let _ = self.timers.with_timer(0, |t| t.stop());
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
