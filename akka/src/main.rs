// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
mod logger;
mod multicast;
mod value;
use actix::prelude::*;
use std::time::Duration;

use crate::multicast::McActor;

#[actix::main]
 async fn main() {
 //   let _arbiter = Arbiter::new();
    logger::init();

    let _mc_addr = McActor::new().start();
    tokio::time::sleep(Duration::from_millis(3000)).await;

    // let the actors all run until they've shut themselves down
    //   system.run().unwrap();
}
