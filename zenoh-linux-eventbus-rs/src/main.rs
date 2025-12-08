// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
#![allow(unused_variables)]
mod eventbus;
mod limero;
mod logger;
mod map_actor;
mod fsm_actor;
#[cfg(feature = "with-ps4")]
mod ps4_actor;
mod zenoh_actor;

use crate::eventbus::{ActorImpl, ActorStart, ActorStop, Bus, Eventbus, EventbusStop};
use anyhow::Result;
use basu::{async_trait, event};
use limero::*;
use log::{debug, info};
use std::{any::Any, sync::Arc};
use zenoh_linux_eventbus_rs::eventbus::on_message;

#[tokio::main]
async fn main() -> Result<()> {
    logger::init();
    let mut eventbus = Eventbus::new();
    let zenoh_actor = zenoh_actor::ZenohActor::new(eventbus.bus());
    let map_actor = map_actor::MapActor::new(eventbus.bus());

    eventbus.register_actor(Box::new(map_actor));
    eventbus.register_actor(Box::new(zenoh_actor));
    #[cfg(feature = "with-ps4")]
    {
        let ps4_actor = ps4_actor::Ps4Actor::new(eventbus.bus());
        eventbus.register_actor(Box::new(ps4_actor));
    }
    eventbus.run().await?;

    Ok(())
}
