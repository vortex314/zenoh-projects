use anyhow::Result;
use clap::{Arg, Command};
use std::fs;
use std::path::Path;
use zenoh::prelude::r#async::*;

#[tokio::main]
async fn main() -> Result<()> {
    // Parse command-line arguments
    let matches = Command::new("OTA Zenoh Uploader")
        .version("1.0")
        .about("Uploads a firmware binary to an ESP32 via Zenoh for OTA updates")
        .arg(
            Arg::new("key")
                .short('k')
                .long("key")
                .value_name("KEY")
                .help("The Zenoh key (topic) to publish the firmware binary to")
                .required(true),
        )
        .arg(
            Arg::new("binary")
                .short('b')
                .long("binary")
                .value_name("BINARY")
                .help("The path to the firmware binary file")
                .required(true),
        )
        .get_matches();

    // Get the Zenoh key and binary path from the arguments
    let key = matches.get_one::<String>("key").unwrap();
    let binary_path = matches.get_one::<String>("binary").unwrap();
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
    println!(
        "Loaded firmware binary ({} bytes) from: {:?}",
        firmware_data.len(),
        binary_path
    );

    // Initialize Zenoh
    let zenoh = zenoh::open(zenoh::config::default()).res().await?;
    let session = zenoh.into_arc();

    // Publish the firmware binary to the specified Zenoh key
    println!("Publishing firmware binary to key: {}", key);
    session
        .put(key, firmware_data)
        .res()
        .await?;

    println!("Firmware binary published successfully!");

    // Close the Zenoh session
    session.close().res().await?;

    Ok(())
}