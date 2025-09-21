// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
mod alive;
mod brain;
mod broker;
mod logger;
mod multicast;
mod value;
mod udp;
mod eventbus;
mod limero;
use limero::*;
use actix::prelude::*;
use local_ip_address::local_ip;
use log::{debug, info};
use std::time::Duration;


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


#[actix::main]
async fn main() {
    //   let _arbiter = Arbiter::new();
    logger::init();

    let listener = Listener::new().start();
    let _mc_addr: Addr<McActor> = McActor::new(MULTICAST_IP,MULTICAST_PORT).start();
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
