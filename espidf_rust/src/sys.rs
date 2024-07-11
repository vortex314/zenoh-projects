use crate::limero::timer::Timer;
use crate::limero::timer::Timers;
use crate::limero::ActorTrait;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::limero::SourceTrait;
use crate::protocol::msg::ProxyMessage;
use crate::pubsub::PubSubActor;
use crate::pubsub::PubSubCmd;
use crate::pubsub::PubSubEvent;
use crate::pubsub::payload_decode;
use crate::pubsub::payload_encode;

use alloc::boxed::Box;
use alloc::fmt::format;
use alloc::string::String;
use alloc::string::ToString;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self, select3, Either3};
use embassy_time::Duration;
use embassy_time::Instant;
use log::info;
use minicbor::Decode;
use minicbor::Encode;

#[derive(Clone)]
pub enum SysCmd {
    PubSubEvent(PubSubEvent),
}
#[derive(Clone)]
pub enum SysEvent {
    PubSubCmd(PubSubCmd),
}

#[derive(Clone, Decode)]

struct EchoReq {
    #[n(0)]
    timestamp: u64,
    #[n(1)]
    reply_to: String,
}

pub struct Sys {
    events: Source<SysEvent>,
    cmds: Sink<SysCmd, 3>,
    timers: Timers,
}

impl Sys {
    pub fn new() -> Sys {
        Sys {
            events: Source::new(),
            cmds: Sink::new(),
            timers: Timers::new(),
        }
    }
    async fn on_cmd(&mut self, cmd: SysCmd) {
        match cmd {
            SysCmd::PubSubEvent (event) => {
                self.on_pubsub(event);
            }
        }
    }
    fn on_pubsub(&mut self, event: PubSubEvent) {
        info!("Event: {:?}", event);
        match event {
            PubSubEvent::Publish { topic, message } => {
                if topic == "sys/echo" {
                    if let Ok(echo_req) = payload_decode::<EchoReq>(&message) {
                        self.events.emit(SysEvent::PubSubCmd(PubSubCmd::Publish {
                            topic: echo_req.reply_to,
                            message: payload_encode(echo_req.timestamp),
                        }))
                    } else {
                        info!("Failed to decode echo request");
                    }
                } else {
                    info!("Unknown topic: {}", topic);  
                }
            }
            _ => {}
        }
    }
    async fn on_timer(&mut self, _timer_id: u32) {
        info!("Timer fired");
        self.events.emit(SysEvent::PubSubCmd(PubSubCmd::Publish {
            topic: "sys/uptime".to_string(),
            message: payload_encode(Instant::now().as_millis() as u64),
        }));
    }
}


impl ActorTrait<SysCmd, SysEvent> for Sys {
    async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(1, Duration::from_millis(1000)) );
        loop {
            match select(self.cmds.next(), self.timers.alarm()).await {
                First(cmd) => {
                    self.on_cmd(cmd.unwrap()).await;
                }
                Second(timer) => {
                    self.on_timer(timer).await;
                }
            }
        }
    }

    fn sink_ref(&self) -> SinkRef<SysCmd> {
        self.cmds.sink_ref()
    }

    fn add_listener(&mut self, sink_ref: SinkRef<SysEvent>) {
        self.events.add_listener(sink_ref);
    }
}

impl SourceTrait<SysEvent> for Sys {
    fn add_listener(&mut self, sink_ref: SinkRef<SysEvent>) {
        self.events.add_listener(sink_ref);
    }
}
