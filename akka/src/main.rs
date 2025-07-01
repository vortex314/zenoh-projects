// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
mod logger;
mod multicast;
mod value;
mod brain;
use actix::prelude::*;
use log::info;
use std::time::Duration;

use crate::{multicast::McActor, value::Value};

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
            multicast::McEvent::Received(value) => {
                if value["src"] == Value::from("esp1/sys") {
                    info!("Received value for esp1/sys: {}", value["pub"]["free_heap"]);
                } else {
                   // info!("Received value for other destination: {:?}", value);
                }
            }
            multicast::McEvent::ConnectionLost => {
                info!("Connection lost");
            }
            multicast::McEvent::ConnectionEstablished => {
                info!("Connection established");
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

#[actix::main]
 async fn main() {
 //   let _arbiter = Arbiter::new();
    logger::init();

    let listener = Listener::new().start();
    let _mc_addr: Addr<McActor> = McActor::new().start();

    let listener_clone = listener.clone();

    _mc_addr.do_send(multicast::McCmd::AddListener(listener.recipient()));

    tokio::time::sleep(Duration::from_millis(3000)).await;
    listener_clone.do_send(ListenerCmd);
    tokio::time::sleep(Duration::from_millis(30000000)).await;

    // let the actors all run until they've shut themselves down
    //   system.run().unwrap();
}
