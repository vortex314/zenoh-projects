// ignore unused imports
// ignore unused variables

#![allow(unused_imports)]
#![allow(unused_variables)]

use anyhow::{Context, Error, Result};
use clap::{Arg, Command};
use log::{error, info};
use zenoh_config::WhatAmI;
use std::fs;
use std::path::Path;
use walkdir::WalkDir;
use zenoh::{bytes::Encoding, key_expr::KeyExpr, session, Config};

mod logger;
use logger::init;

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
                .default_value(".pio/build/esp32dev/firmware.bin")
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
            config.insert_json5("mode", r#""client""#).unwrap();
    //        config.insert_json5("mode", "\"client\"").unwrap();
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

    // Publish the firmware binary to the specified Zenoh key
    info!("Publishing firmware binary to key: {}", key);
    session
        .put(key, firmware_data)
        .await
        .map_err(|e| anyhow::anyhow!(e))?;

    info!("Firmware binary published successfully!");

    // Close the Zenoh session
    session.close().await.map_err(|e| anyhow::anyhow!(e))?;

    Ok(())
}
