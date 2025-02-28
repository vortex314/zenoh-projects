// ignore unused imports
// ignore unused variables

#![allow(unused_imports)]
#![allow(unused_variables)]

use anyhow::{Context, Error, Result};
use clap::{Arg, Command};
use log::{debug, error, info};
use minicbor::{to_vec, Encode};
use std::path::Path;
use std::{fs, thread::Thread};
use tokio::time;
use walkdir::WalkDir;
use zenoh::handlers::FifoChannelHandler;
use zenoh::pubsub::Subscriber;
use zenoh::qos::CongestionControl;
use zenoh::sample::Sample;
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
            config
                .insert_json5("connect/endpoints", r#"["tcp/limero.ddns.net:7447"]"#)
                .unwrap();
            config
                .insert_json5("scouting/multicast/enabled", "false")
                .unwrap();

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

    info!("Declared subscriber for key: '{}'", reply_topic);

    let mut ota_msg = OtaMsg::default();
    ota_msg.operation = Some(OtaOperation::OtaBegin);
    ota_msg.reply_to = Some(reply_topic.to_string());

    // Publish the OTA begin message to the specified Zenoh key

    let reply = request(session.clone(), &subscriber, key.to_string(), ota_msg, 5).await?;

    if reply.operation.unwrap() != OtaOperation::OtaBegin && reply.rc != Some(0) {
        return Err(anyhow::anyhow!(
            "Expected OTA begin ack, got: {:?}",
            reply.message.unwrap()
        ));
    }

    let block_size = 1024;
    let blocks = firmware_data.len() / block_size;
    info!(
        "Sending OTA data in {} blocks of {} bytes",
        blocks, block_size
    );

    for i in 0..=blocks {
        let begin = i * block_size;
        let mut end = begin + block_size;
        if end > firmware_data.len() {
            end = firmware_data.len()
        }
        let slice = &firmware_data[begin..end];

        info!(
            "Sending OTA data chunk: {}..{} - [{}]",
            begin,
            end,
            end - begin
        );
        let mut ota_msg = OtaMsg::default();
        ota_msg.operation = Some(OtaOperation::OtaWrite);
        ota_msg.offset = Some(begin as u32);
        ota_msg.image = Some(slice.to_vec());
        ota_msg.reply_to = Some(reply_topic.to_string());
        let reply = request(session.clone(), &subscriber, key.to_string(), ota_msg, 3).await?;
        if reply.operation != Some(OtaOperation::OtaWrite) && reply.rc != Some(0) {
            return Err(anyhow::anyhow!(
                "Expected OTA data ack, got: {:?}",
                reply.message.unwrap()
            ));
        }
    }

    let mut ota_msg = OtaMsg::default();
    ota_msg.operation = Some(OtaOperation::OtaEnd);
    ota_msg.reply_to = Some(reply_topic.to_string());

    // Publish the OTA begin message to the specified Zenoh key
    info!(" Sending OtaEnd ");

    let reply = request(session.clone(), &subscriber, key.to_string(), ota_msg, 1).await?;

    if reply.operation != Some(OtaOperation::OtaEnd) && reply.rc != Some(0) {
        return Err(anyhow::anyhow!(
            "Expected OTA begin ack, got: {:?}",
            reply.operation
        ));
    }

    session.close().await.map_err(|e| anyhow::anyhow!(e))?;

    Ok(())
}

async fn request(
    session: Session,
    subscriber: &Subscriber<FifoChannelHandler<Sample>>,
    request_topic: String,
    request: OtaMsg,
    trials: u32,
) -> Result<OtaMsg> {
    debug!(
        "Publishing OTA  message to key: '{}':{:?}",
        request_topic, request.operation
    );

    for i in 0..trials {
        let request_bytes = to_vec(&request)?;
        info!("Request bytes: '{}' => [{}]",request_topic, request_bytes.len()); // {:02X?}
        session
            .put(request_topic.clone(), request_bytes)
            .congestion_control(CongestionControl::Block)
            .await
            .map_err(|e| anyhow::anyhow!(e))?;

        tokio::select! {
            _ = time::sleep(time::Duration::from_secs(1)) =>{

            },
            sample = subscriber.recv_async() =>  {
                match sample {
                Ok(sample) => {
                    let bytes: Vec<u8> = sample.payload().slices().fold(Vec::new(), |mut b, x| {
                        b.extend_from_slice(x);
                        b
                    });
                    let reply = minicbor::decode::<OtaMsg>(bytes.as_slice())?;
                    info!("Received OTA reply message: {:?}", reply);
                    return Ok(reply)
                }
                Err(e) => {},
            }
        }
        }
        if  i+1 < trials { info!(" Retry ......................... {}",i+1);};
    }
    Err(Error::msg("failed after retries"))
}
