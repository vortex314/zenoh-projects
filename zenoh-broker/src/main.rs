//
// Copyright (c) 2023 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
use std::time::Duration;

use clap::Parser;
use log::info;
use zenoh::{bytes::Encoding, key_expr::KeyExpr, session, Config};
mod common;

use common::CommonArgs;
use anyhow::Result;

mod logger;
use logger::init;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    // Initiate logging
    //  zenoh::init_log_from_env_or("info");
    zenoh::init_log_from_env_or("debug");
    logger::init();

    let config = Config::from_file("./config.json5")?;

    let key_expr = KeyExpr::try_from("src/lm1/ps4")?;

    info!("Opening session...");
    let session = zenoh::open(config).await?;

    println!("Declaring Subscriber on '{}'...", &key_expr);
    let subscriber = session.declare_subscriber(&key_expr).await?;

    println!("Press CTRL-C to quit...");
    let mut counter = 0;
    let mut start_time = std::time::Instant::now();
    let mut start_counter = 0;
    while let Ok(sample) = subscriber.recv_async().await {
        // Refer to z_bytes.rs to see how to deserialize different types of message
        let payload = sample.payload().to_bytes();
        counter += 1;
        if counter % 100 == 0 {
            info!(
                "Received {} samples , {:.2} msg/sec ",
                counter,
                (counter - start_counter) as f64 / start_time.elapsed().as_secs_f64()
            );
            start_time = std::time::Instant::now();
            start_counter = counter;
        }
        info!(
            ">> [Subscriber] Received {} ('{}': '{}')",
            sample.kind(),
            sample.key_expr().as_str(),
            minicbor::display(&payload)
        );
        if let Some(att) = sample.attachment() {
            let att = att.try_to_string().unwrap_or_else(|e| e.to_string().into());
            info!(" ({})", att);
        }

        //  }
        /*         let session_info = session.info();
        info!(" zid = {}", session.zid());
        for router in session_info.routers_zid().await {
            info!(" router = {}", router);
        }*/
    }
    Ok(())
}

#[derive(clap::Parser, Clone, PartialEq, Eq, Hash, Debug)]
struct Args {
    #[arg(short, long, default_value = "demo/example/zenoh-rs-pub")]
    /// The key expression to write to.
    key: KeyExpr<'static>,
    #[arg(short, long, default_value = "Pub from Rust!")]
    /// The payload to write.
    payload: String,
    #[arg(short, long)]
    /// The attachments to add to each put.
    attach: Option<String>,
    /// Enable matching listener.
    #[cfg(feature = "unstable")]
    #[arg(long)]
    add_matching_listener: bool,
    #[command(flatten)]
    common: CommonArgs,
}

fn parse_args() -> (Config, KeyExpr<'static>, String, Option<String>, bool) {
    let args = Args::parse();
    (
        args.common.into(),
        args.key,
        args.payload,
        args.attach,
        #[cfg(feature = "unstable")]
        args.add_matching_listener,
        #[cfg(not(feature = "unstable"))]
        false,
    )
}
