use core::result;

use alloc::boxed::Box;
use alloc::format;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::vec::Vec;
use cobs::CobsEncoder;
use embassy_executor::Spawner;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{DynamicReceiver, DynamicSender, Sender};
use embassy_sync::pubsub::publisher::Pub;
use embassy_sync::pubsub::PubSubChannel;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use alloc::collections::BTreeMap;
use alloc::string::String;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};
use embassy_time::{with_timeout, Duration, Instant};
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
};
use esp_println::println;
use log::{debug, info};

use limero::{ Actor,CmdQueue,EventHandlers,Handler};
use limero::{timer::Timer, timer::Timers};
use crate::proxy_message::{ Flags, ProxyMessage, ReturnCode, VecWriter};
use pubsub::PubSubEvent;
use pubsub::PubSubCmd;

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

pub enum PubSubCmd {}
pub enum PubSubEvent {}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct PubSubActor {
    cmds: CmdQueue<PubSubCmd>,
    events: EventHandlers<PubSubEvent>,
    timers: Timers,
    
}
impl PubSubActor {
    pub fn new(txd_msg: Box<dyn Handler<ProxyMessage>>) -> PubSubActor {
        PubSubActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),}
    }

   

    async fn on_timeout(&mut self, id: u32) {
       
    }



    async fn on_rxd_message(&mut self, msg: ProxyMessage) {
        match msg {
        };
    }

    pub async fn on_cmd_message(&mut self, cmd: PubSubCmd) {
        match cmd {
        }
    }
}

impl Actor<ProxyMessage, ProxyMessage> for EspNowActor {

    async fn run(&mut self) {
       loop {
           match select(self.cmds.next(), self.timers.alarm()).await {
               First(msg) => match msg {
                   _ => {}
               },
               Second(idx) => {
                   self.on_timeout(idx).await;
               }
           }
       }
   }
   fn add_listener(&mut self, listener: Box<dyn Handler<ProxyMessage>>) {
       self.events.add_listener(listener);
   }

   fn handler(&self) -> Box<dyn Handler<ProxyMessage>> {
       self.cmds.handler()
   }
   
}


