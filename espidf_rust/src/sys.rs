use crate::client::ClientSession;
use crate::client::SessionInput;
use crate::client::SessionEvent;
use crate::limero::timer::Timers;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::limero::SourceTrait;
use crate::protocol::msg::MqttSnMessage;
use alloc::boxed::Box;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self, select3, Either3};

pub struct Sys {
    pub_sub_events: Sink<SessionEvent, 4>,
    pub_sub_cmds: SinkRef<SessionInput>,
    timers: Timers,
}

impl Sys {
    pub fn new(client: SinkRef<SessionInput>) -> Sys {
        let pub_sub_events = Sink::new();
        let pub_sub_cmds = client;
        Sys {
            pub_sub_events,
            pub_sub_cmds,
            timers: Timers::new(),
        }
    }

    pub fn on_session_event(&self) -> SinkRef<SessionEvent> {
        self.pub_sub_events.sink_ref()
    }

    pub async fn run(&mut self) {
        loop {
            match select(self.pub_sub_events.read(), self.timers.alarm()).await {
                First(event) => {
                    self.on_event(event.unwrap()).await;
                }
                Second(timer) => {
                    self.on_timer(timer).await;
                }
            }
        }
    }

    async fn on_event(&mut self, event: SessionEvent) {
        match event {
            SessionEvent::Connected => {

            }
            SessionEvent::Disconnected => {
            }
            _ => {}
        }
    }

    async fn on_timer(&mut self, _timer_id: u32) {}
}
