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
mod ps4_actor;
mod map_actor;

use anyhow::Result;
use basu::{async_trait, event};
use limero::*;
use log::{debug, info};
use zenoh_linux_eventbus_rs::eventbus::on_message;
use std::{any::Any, sync::Arc};
use crate::eventbus::{ActorImpl, Bus, Eventbus, ActorStart, ActorStop, EventbusStop};

#[tokio::main]
async fn main() -> Result<()> {
    logger::init();
    let mut eventbus = Eventbus::new();
    let zenoh_actor = zenoh_actor::ZenohActor::new(eventbus.bus());
    let ps4_actor = ps4_actor::Ps4Actor::new(eventbus.bus());
    let map_actor = map_actor::MapActor::new(eventbus.bus());

    eventbus.register_actor(Box::new(map_actor));
    eventbus.register_actor(Box::new(zenoh_actor));
    eventbus.register_actor(Box::new(ps4_actor));
    eventbus.run().await?;
    
    Ok(())
}
