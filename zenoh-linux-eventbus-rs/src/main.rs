// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
mod limero;
mod logger;
mod zenoh_actor;
mod eventbus;
use actix::prelude::*;
use limero::*;
use local_ip_address::local_ip;
use log::{debug, info};
use std::{any::Any, collections::HashMap, hash::Hash, time::Duration};
use eventbus::*;

struct Actor {
    name: String,
}

impl Handler for Actor {
    async fn handle(&mut self, event: &Box<dyn Any + Send + 'static>) {
        handle_type(event, |s: &String| {
            info!("{} received String event: {}", self.name, s);
        });
        handle_type::<i32, _>(event, |i| {
            info!("{} received i32 event: {}", self.name, i);
        });
        handle_type::<MulticastEvent, _>(event, |mi| {
            info!("{} received MulticastEvent event: {:?}", self.name, mi);
        });
    }
}



#[actix::main]
async fn main() {
    //   let _arbiter = Arbiter::new();
    logger::init();

    let mut event_bus = EventBus::new();

    let actor1 = Actor {
        name: "Actor1".into(),
    };
    let actor2 = Actor {
        name: "Actor2".into(),
    };

    event_bus.register_handler::<String>(Box::new(actor1));
    event_bus.register_handler::<MulticastEvent>(Box::new(actor2));

    event_bus.emit(Box::new(42)).await;
    event_bus.emit(Box::new("Hello, world!".to_string())).await;
    event_bus.emit(Box::new(MulticastEvent::default())).await; // Unknown type
    event_bus.emit(Box::new(Ps4Event::default())).await; // Unknown type    

    tokio::spawn(async move {
        event_bus.run().await;
    });

}
