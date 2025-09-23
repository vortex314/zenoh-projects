// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
mod alive;
mod brain;
mod broker;
mod eventbus;
mod limero;
mod logger;
mod multicast;
mod udp;
mod value;
use actix::prelude::*;
use limero::*;
use local_ip_address::local_ip;
use log::{debug, info};
use std::{any::Any, collections::HashMap, hash::Hash, time::Duration};

use crate::{brain::Brain, multicast::McActor, value::Value};

#[derive(Message)]
#[rtype(result = "()")]
struct ListenerCmd;
struct Listener;

impl Actor for Listener {
    type Context = Context<Self>;
}

impl Listener {
    fn new() -> Self {
        Listener
    }
}

impl Handler<multicast::McEvent> for Listener {
    type Result = ();

    fn handle(&mut self, msg: multicast::McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            multicast::McEvent::ReceivedValue(value) => {
                if value["src"] == Value::from("esp1/sys") {
                    debug!("Received value for esp1/sys: {}", value["pub"]["free_heap"]);
                } else {
                    // info!("Received value for other destination: {:?}", value);
                }
            }
        }
    }
}

impl Handler<ListenerCmd> for Listener {
    type Result = ();

    fn handle(&mut self, _msg: ListenerCmd, _: &mut Self::Context) -> Self::Result {
        info!("Listener command received");
    }
}

const MULTICAST_IP: &str = "225.0.0.1";
const MULTICAST_PORT: u16 = 6502;
const BROKER_PORT: u16 = 6503;

trait MyHandler: Send {
    fn handle(&mut self, event: &Box<dyn Any + Send>);
}
struct MyEventBus {
    sender: tokio_mpmc::Sender<Box<dyn Any + Send>>,
    receiver: tokio_mpmc::Receiver<Box<dyn Any + Send>>,
    my_handlers: HashMap<std::any::TypeId, Vec<Box<dyn MyHandler + Send>>>,
}

impl MyEventBus {
    fn new() -> Self {
        let (sender, receiver) = tokio_mpmc::channel::<Box<dyn Any + Send>>(1024);
        MyEventBus {
            sender,
            receiver,
            my_handlers: HashMap::new(),
        }
    }

    async fn emit(&self, event: Box<dyn Any + Send>) {
        let _ = self.sender.send(event).await;
    }

    fn register_handler<T: 'static + Send>(&mut self, handler: Box<dyn MyHandler + Send>) {
        self.my_handlers
            .entry(std::any::TypeId::of::<T>())
            .or_default()
            .push(handler);
        for (key, handlers) in &self.my_handlers {
            info!("Registered handler for type {:?} ({} handlers)", key, handlers.len());
        }
    }

    async fn run(&mut self) {
        while let Ok(event) = self.receiver.recv().await {
            let event = event.unwrap();
            info!("Event received: {:?} ({} handlers registered)", event.type_id(), self.my_handlers.keys().count());
            for handler in self
                .my_handlers
                .get_mut(&event.type_id())
                .unwrap_or(&mut Vec::new())
            {
                info!("Handling event with handler");
                handler.handle(&event);
            }
        }
    }
}

struct MyActor {
    name: String,
}

fn handle_type<T, F>(event: &Box<dyn Any + Send + 'static>, mut f: F) -> bool
where
    T: 'static + Send,
    F: FnMut(&T),
{
    info!("Checking event type: {:?}", event.type_id());
   if event.is::<T>() {
        if let Some(value) = event.downcast_ref::<T>() {
            f(value);
            return true; // Downcast and processing succeeded
        }
    }
    false // Type mismatch or downcast failed
}

impl MyHandler for MyActor {
    fn handle(&mut self, event: &Box<dyn Any + Send + 'static>) {
        info!("{} handling event", self.name);
        handle_type(event, |s: &String| {
            self.name = s.clone();
            info!("{} received String event: {}", self.name, s);
        });
        handle_type::<i32, _>(event, |i| {
            info!("{} received i32 event: {}", self.name, i);
        });
        handle_type::<MulticastInfo, _>(event, |mi| {
            info!("{} received MulticastInfo event: {:?}", self.name, mi);
        });
    }
}

#[actix::main]
async fn main() {
    //   let _arbiter = Arbiter::new();
    logger::init();

    let mut event_bus = MyEventBus::new();

    let actor1 = MyActor {
        name: "Actor1".into(),
    };
    let actor2 = MyActor {
        name: "Actor2".into(),
    };

    event_bus.register_handler::<String>(Box::new(actor1));
    event_bus.register_handler::<MulticastInfo>(Box::new(actor2));

    event_bus.emit(Box::new(42)).await;
    event_bus.emit(Box::new("Hello, world!".to_string())).await;
    event_bus.emit(Box::new(MulticastInfo::default())).await; // Unknown type

    tokio::spawn(async move {
        event_bus.run().await;
    });

    let listener = Listener::new().start();
    let _mc_addr: Addr<McActor> = McActor::new(MULTICAST_IP, MULTICAST_PORT).start();
    let _brain = Brain::new().start();
    let _broker = broker::BrokerActor::new(BROKER_PORT).start();

    _mc_addr.do_send(multicast::McCmd::AddListener(listener.recipient()));

    loop {
        // Wait for a while before sending the announce message
        tokio::time::sleep(Duration::from_millis(1000)).await;

        // Send an announce message to the multicast group
        let mut announce = Value::object();
        announce["src"] = "broker".into();
        announce["ip"] = local_ip().unwrap().to_string().into();
        announce["port"] = (BROKER_PORT as i64).into();
        announce["type"] = "broker".into();
        _mc_addr.do_send(multicast::McCmd::SendValue(announce));
    }

    // let the actors all run until they've shut themselves down
    //   system.run().unwrap();
}
