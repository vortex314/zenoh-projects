#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
use actor::Actor;
use actor::ActorImpl;
use log::info;
use minicbor::Encode;
use minicbor::Encoder;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::Mutex;
use std::thread::Thread;
use std::time::Duration;

use log::debug;
use log::error;
use minicbor::decode::info;
use minicbor::display;

mod value;

use tokio::sync::mpsc::Sender;
use value::Value;
mod actor;
mod logger;
mod zenoh_actor;
use zenoh_actor::ZenohActor;
use zenoh_actor::ZenohCmd;
use zenoh_actor::*;
mod brain_actor;
use anyhow::Result;
use brain_actor::*;

#[tokio::main(flavor = "multi_thread", worker_threads = 3)]

async fn main() -> Result<()> {
    logger::init(); // Log to stderr (if you run with `RUST_LOG=debug`);

    let mut zenoh_actor: ZenohActor = ZenohActor::new();
    let mut brain_actor: BrainActor = BrainActor::new();

    zenoh_actor.on_event( |event| match event {
        ZenohEvent::Publish { topic, payload } => {
            info!("{} =>{}", topic, display(&payload));
        }
        _ => {}
    });

    let zenoh_sender = zenoh_actor.sender();

    brain_actor.on_event( move |event: &brain_actor::BrainEvent| match event {
        BrainEvent::Publish{topic,msg} => {
            let mut writer = Vec::<u8>::new();
            let mut encoder = Encoder::new(&mut writer);
            msg.encode(&mut encoder, &mut ()).unwrap();
            let _ = zenoh_sender.try_send(ZenohCmd::Publish {
                topic : topic.clone(),
                payload: writer,
            });
        }
        _ => {}
    });

    tokio::spawn(async move {
        zenoh_actor.run().await;
    });
    tokio::spawn(async move {
        brain_actor.run().await;
    });
    tokio::time::sleep(Duration::from_secs(100000)).await;
    Result::Ok(())
}
