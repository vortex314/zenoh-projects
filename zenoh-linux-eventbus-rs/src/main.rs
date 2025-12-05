// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
#![allow(unused_variables)]
mod limero;
mod logger;
mod eventbus;
mod zenoh_actor;

use anyhow::Result;
use basu::{async_trait, event};
use limero::*;
use log::{debug, info};
use zenoh_linux_eventbus_rs::eventbus::on_message;
use std::{any::Any, sync::Arc};
use crate::eventbus::{ActorImpl, Bus, Eventbus, ActorStart, ActorStop, EventbusStop};



struct Actor1 {
    name: String,
    bus : Option<Bus>,
    counter : usize,
}

impl Actor1 {
    fn new(name: String, bus : Bus) -> Self {
        Actor1 {
            name,
            bus : Some(bus),
            counter : 0,
        }
    }
    fn on_start(&mut self, msg: &ActorStart) {
        self.bus = Some(msg.bus.clone());
    }
    fn on_stop(&mut self, _msg: &ActorStop) {
        self.bus = None;
    }
}

#[async_trait]
impl ActorImpl for Actor1 {

    async fn handle(&mut self, _msg: &Arc<dyn Any + Send + Sync>)  {
        // test for type and downcast
        info!("Actor {} handling message", self.name);

        on_message::<ActorStart,_>(_msg, |msg| {
            self.counter += 1;
            info!("Actor {} received ActorStart #{}", self.name, self.counter);
            self.on_start(msg);
        });

        on_message::<ActorStop,_>(_msg, |msg| {
            self.on_stop(msg);
        });

        on_message::<Ps4Event,_>(_msg, |msg| {
            info!("Actor {} received Ps4Event: {:?}", self.name, msg);
        });

        on_message::<String,_>(_msg, |msg| {
            info!("Actor {} received String message: {}", self.name, msg);
        });
    }
}


#[tokio::main]
async fn main() -> Result<()> {
    logger::init();
    info!("Starting Limero example...");

    let mut eventbus = Eventbus::new();
    let  zenoh_actor = zenoh_actor::ZenohActor::new(eventbus.bus());
    /*let actor = Actor1 ::new("actor1".to_string(), eventbus.bus());

    let actor2 = Actor1 ::new("actor2".to_string(), eventbus.bus());
    eventbus.register_actor(Box::new(actor));
    eventbus.register_actor(Box::new(actor2));
    eventbus.emit("Hello, Limero!".to_string());*/
    eventbus.emit(Ps4Event::default());
    eventbus.register_actor(Box::new(zenoh_actor));
    eventbus.emit(zenoh_actor::ZenohCmd::Publish {
        topic: "src/ps4/Ps4Event".to_string(),
        payload: serde_json::to_vec(&Ps4Event::default()).unwrap(),
    });
    eventbus.run().await?;
    info!("Limero example finished.");

    Ok(())
}
