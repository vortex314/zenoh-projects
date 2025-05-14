#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
use actor::Actor;
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

use shared::on_shared;
use shared::update_with_value;
use shared::FieldInfo;
use tokio::sync::mpsc::Sender;
use value::Value;
mod actor;
mod logger;
mod zenoh_actor;
use zenoh_actor::ZenohActor;
mod brain_actor;
use anyhow::Result;
use brain_actor::BrainActor;
mod shared;
use shared::SHARED;

#[tokio::main(flavor = "multi_thread", worker_threads = 3)]

async fn main() -> Result<()> {
    logger::init(); // Log to stderr (if you run with `RUST_LOG=debug`);

    let mut zenoh_actor: ZenohActor = ZenohActor::new();
    let mut brain_actor: BrainActor = BrainActor::new();


    zenoh_actor.add_listener(move |_event| match _event {
        zenoh_actor::ZenohEvent::Publish { topic, payload } => {
            info!("{} =>{}", topic, display(&payload));
            let r = Value::from_cbor(payload.to_vec());
            if let Ok(value) = r {
                let s: String = value.to_string();
                if s.len() > 100 {
                    debug!(" RXD {} :{} ", topic, &s[0..100]);
                } else {
                    debug!(" RXD {} :{} ", topic, s);
                }
                update_with_value(topic, &value);
                // update widgets with received value
            } else {
                error!(
                    "Error decoding payload from topic {} [{}] : {}",
                    topic,
                    payload.len(),
                    r.err().unwrap()
                );
            }
        }
        _ => {}
    });

    let sender = zenoh_actor.sender().unwrap();

    brain_actor.add_listener(move |_event| match _event {
        brain_actor::BrainEvent::Msg(brain_msg) => {
            let mut writer = Vec::<u8>::new();
            let mut encoder = Encoder::new(&mut writer);
            let r = brain_msg.encode(&mut encoder, &mut ()).unwrap();
            let mut zenoh_cmd = zenoh_actor::ZenohCmd::Publish {
                topic: "src/brain/msg".to_string(),
                payload: writer,
            };
            sender.try_send(zenoh_cmd).unwrap();
        }
        _ => {}
    });

    tokio::spawn(async move {
        let _ = zenoh_actor.run().await.map_err(|err| {
            error!("Error in Zenoh actor: {}", err);
        });
    });
    tokio::spawn(async move {
        let _ = brain_actor.run().await.map_err(|err| {
            error!("Error in Brain actor: {}", err);
        });
    });
    tokio::time::sleep(Duration::from_secs(1000)).await;
    Result::Ok(())
}
