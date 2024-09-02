use limero::timer::Timer;
use limero::timer::Timers;

use limero::Handler;
use limero::Actor;
use limero::Endpoint;
use limero::CmdQueue;
use limero::EventHandlers;

use crate::ProxyMessage;
use pubsub::PubSubCmd;
use pubsub::PubSubEvent;
use pubsub::payload_decode;
use pubsub::payload_encode;
use pubsub::Codec;
use crate::ALLOCATOR;

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
use serde::{Deserialize, Serialize};

use serdes::Cbor;
use serdes::PayloadCodec;

#[derive(Clone)]
pub enum SysCmd {
    PubSubEvent(PubSubEvent),
}
#[derive(Clone)]
pub enum SysEvent {
    PubSubCmd(PubSubCmd),
}

#[derive(Clone, Deserialize, Serialize)]

struct EchoReq {
    timestamp: u64,
    reply_to: String,
}

pub struct SysActor {
    cmds: CmdQueue<SysCmd>,
    events: EventHandlers<SysEvent>,
    timers: Timers,
}

impl SysActor {
    pub fn new() -> SysActor {
        SysActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
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
            PubSubEvent::Publish { topic, payload } => {
                if topic == "sys/echo" {
                    if let Ok(echo_req) = Cbor::decode::<EchoReq>(&payload) {
                        self.events.handle(&SysEvent::PubSubCmd(PubSubCmd::Publish {
                            topic: echo_req.reply_to,
                            payload: payload_encode(&echo_req.timestamp, Codec::Cbor),
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
        self.events.handle(&SysEvent::PubSubCmd(PubSubCmd::Publish {
            topic: "sys/uptime".to_string(),
            payload: payload_encode(&(Instant::now().as_millis() as u64),Codec::Cbor),
        }));
        self.events.handle(&SysEvent::PubSubCmd(PubSubCmd::Publish {
            topic: "sys/heap_free".to_string(),
            payload: payload_encode(&(ALLOCATOR.free() as u64),Codec::Cbor),
        }));
        self.events.handle(&SysEvent::PubSubCmd(PubSubCmd::Publish {
            topic: "sys/heap_used".to_string(),
            payload: payload_encode(&(ALLOCATOR.used() as u64),Codec::Cbor),
        }));
    }
}


impl Actor<SysCmd, SysEvent> for SysActor {
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

    fn add_listener(&mut self, handler : Endpoint<SysEvent>) {
        self.events.add_listener(handler);
    }

    fn handler(&self) -> Endpoint<SysCmd> {
        self.cmds.handler()
    }
}

