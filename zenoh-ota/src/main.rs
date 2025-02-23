// ignore unused imports
// ignore unused variables

#![allow(unused_imports)]
#![allow(unused_variables)]

use anyhow::{Context, Error, Result};
use clap::{Arg, Command};
use log::{error, info};
use minicbor::{to_vec, Encode};
use zenoh::sample::Sample;
use std::path::Path;
use std::{fs, thread::Thread};
use walkdir::WalkDir;
use zenoh::handlers::FifoChannelHandler;
use zenoh::pubsub::Subscriber;
use zenoh::qos::CongestionControl;
use zenoh::Session;
use zenoh::{bytes::Encoding, key_expr::KeyExpr, session, Config};
use zenoh_config::WhatAmI;

mod logger;
use logger::init;

mod msg;
use msg::*;

#[tokio::main]
async fn main() -> Result<()> {
    zenoh::init_log_from_env_or("info");
    logger::init();

    let r = do_ota().await;
    if r.is_err() {
        error!("Error: {:?}", r.err().unwrap().to_string());
    }
    Ok(())
}

fn find_default_file() -> Result<String> {
    let default_file = ".pio/build/esp32dev/firmware.bin";
    // walk the directory tree from . to look for regexp match
    for entry in WalkDir::new(".") {
        let entry = entry.unwrap();
        if entry.file_name().to_string_lossy().contains("firmware.bin") {
            return Ok(entry.path().to_str().unwrap().to_string());
        }
    }
    Ok(default_file.to_string())
}

async fn do_ota() -> Result<()> {
    /*let config = Config::from_file("./config.json5")
    .map_err(|e| anyhow::anyhow!(e))
    .context("Error on Zenoh config ") ?; // Load the Zenoh configuration from a file*/
    // Parse command-line arguments
    let matches = Command::new("OTA Zenoh Uploader")
        .version("1.0")
        .about("Uploads a firmware binary to an ESP32 via Zenoh for OTA updates")
        .arg(
            Arg::new("key")
                .short('k')
                .long("key")
                .value_name("KEY")
                .default_value("dst/cam1/ota/firmware")
                .help("The Zenoh key (topic) to publish the firmware binary to")
                .required(true),
        )
        .arg(
            Arg::new("binary")
                .short('b')
                .long("binary")
                .value_name("BINARY")
                .help("The path to the firmware binary file")
                .required(false),
        )
        .arg(
            Arg::new("config")
                .short('c')
                .long("config")
                .value_name("config")
                .help("The path to the zenoh config file")
                .required(false),
        )
        .get_matches();

    // Get the Zenoh key and binary path from the arguments
    let key = matches.get_one::<String>("key").context("argument key")?;

    let binary_path = match matches.get_one::<String>("binary") {
        Some(binary_path) => binary_path.clone(),
        None => find_default_file()?,
    };
    let binary_path = Path::new(&binary_path);

    let config = match matches.get_one::<String>("config") {
        Some(c) => Config::from_file(c).map_err(|e| anyhow::anyhow!(e))?,
        None => {
            let mut config = Config::default();
            //        config.insert_json5("mode", r#""client""#).unwrap();
            config.insert_json5("mode", "\"client\"").unwrap();
            //        config.insert_json5("connect/endpoints", r#"["tcp/192.168.0.240:7447"]"#).unwrap();
            //         info!("Using default Zenoh configuration: {:?}", config);
            config
        }
    };

    // Check if the firmware binary exists
    if !binary_path.exists() {
        return Err(anyhow::anyhow!(
            "Firmware binary not found at: {:?}",
            binary_path
        ));
    }

    // Read the firmware binary into a byte vector
    let firmware_data = fs::read(binary_path)?;
    info!(
        "Loaded firmware binary ({} bytes) from: {:?}",
        firmware_data.len(),
        binary_path
    );

    info!("Opening session...");
    let session = zenoh::open(config).await.map_err(|e| anyhow::anyhow!(e))?; // Open a Zenoh session

    let session_info = session.info();
    let zid = session_info.zid().await;
    info!("Session ZID: {:?}", zid);
    let mut routers_zid = session.info().routers_zid().await;
    while let Some(router_zid) = routers_zid.next() {
        info!("Router ZID: {:?}", router_zid);
    }
    let mut peers_zid = session.info().peers_zid().await;
    while let Some(peer_zid) = peers_zid.next() {
        info!("Peer ZID: {:?}", peer_zid);
    }

    let reply_topic = format!("dst/{}", zid);
    let reply_topic = reply_topic.as_str();

    let subscriber = session.declare_subscriber(reply_topic).await.unwrap();

    info!("Declared subscriber for key: '{}'", key);

    let mut ota_msg = OtaMsg::default();
    ota_msg.operation = Some(OtaOperation::OtaBegin);
    ota_msg.reply_to = Some(reply_topic.to_string());

    // Publish the OTA begin message to the specified Zenoh key

    let reply = request(session.clone(), & subscriber, key.to_string(), ota_msg).await?;


    session.close().await.map_err(|e| anyhow::anyhow!(e))?;

    Ok(())
}

async fn request(
    session: Session,
    subscriber: & Subscriber<FifoChannelHandler<Sample>>,
    request_topic: String,
    request: OtaMsg,
) -> Result<OtaMsg> {
    info!("Publishing OTA begin message to key: '{}':{:?}", request_topic, request);
    let request_bytes = to_vec(&request)?;
    info!("Request bytes: {}", hex::encode(&request_bytes));    
    session
        .put(request_topic, request_bytes)
        .congestion_control(CongestionControl::Block)
        .await
        .map_err(|e| anyhow::anyhow!(e))?;
    match subscriber.recv_async().await {
        Ok(sample) => {
            let bytes:Vec<u8> = sample.payload().slices().fold(Vec::new(), |mut b, x| { b.extend_from_slice(x); b });
            let reply = minicbor::decode::<OtaMsg>(bytes.as_slice())?;
            info!("Received OTA reply message: {:?}", reply);
            Ok(reply)
        }
        Err(e) => Err(anyhow::anyhow!(e)),
    }
}
