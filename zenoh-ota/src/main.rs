// ignore unused imports
// ignore unused variables

#![allow(unused_imports)]
#![allow(unused_variables)]

use anyhow::{Result, Context};
use clap::{Arg, Command};
use log::info;
use std::fs;
use std::path::Path;
use zenoh::{bytes::Encoding, key_expr::KeyExpr, session, Config};

mod logger;
use logger::init;

#[tokio::main]
async fn main()  -> Result<()> {
    zenoh::init_log_from_env_or("info");
    logger::init();

    let config = Config::from_file("./config.json5")?;
    // Parse command-line arguments
    let matches = Command::new("OTA Zenoh Uploader")
        .version("1.0")
        .about("Uploads a firmware binary to an ESP32 via Zenoh for OTA updates")
        .arg(
            Arg::new("key")
                .short('k')
                .long("key")
                .value_name("KEY")
                .default_value(".pio/build/esp32dev/firmware.bin")
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
                .required(true),
        )
        .get_matches();

    // Get the Zenoh key and binary path from the arguments
    let key = matches.get_one::<String>("key").ok_or(Err(anyhow::anyhow!("")))?;
    let binary_path = matches.get_one::<String>("binary").ok_or(Err(anyhow::anyhow!("")))?;
    let binary_path = Path::new(binary_path);

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
    let session = zenoh::open(config).await.map(|| Err( anyhow::anyhow!("")))?; // Open a Zenoh session

    // Publish the firmware binary to the specified Zenoh key
    info!("Publishing firmware binary to key: {}", key);
    session.put(key, firmware_data).await?;

    info!("Firmware binary published successfully!");

    // Close the Zenoh session
    session.close().await?;

    Ok(())
}
