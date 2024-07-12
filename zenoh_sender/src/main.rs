#![allow(unused_imports)]
#![allow(dead_code)]

use clap::Parser;
use encode::{Error, Write};
use std::convert::Infallible;
use std::time::Duration;
use zenoh::config::Config;
use zenoh::prelude::r#async::*;
mod common;
use common::CommonArgs;
use log::*;
use minicbor::*;
use minicbor::bytes::ByteVec;

mod logger;
use logger::init;



fn compose_data(idx:u32) -> Result<Vec<u8>, Error<Infallible>>{
    let mut buffer = Vec::<u8>::new();
    let mut encoder = Encoder::new(&mut buffer);

    encoder
        .begin_array()?
        .str("hello")?
        .f32(idx as f32 )?
        .u32(idx)?
        .end()?;
    let len = encoder.writer().len();
    Ok(buffer[..len].to_vec())
}

#[tokio::main]
async fn main() {
    logger::init();
    // Initiate logging
    zenoh_util::try_init_log_from_env();


    println!("Opening session...");
    let session = zenoh::open(config::default()).res().await.unwrap();


    println!("Press CTRL-C to quit...");
    for idx in 0..u32::MAX {
        tokio::time::sleep(Duration::from_millis(10)).await;
        info!("Publishing data... {}", idx  );
        let v  = compose_data(idx).unwrap();
        session.put("dst/esp32/test/data2", v).res().await.unwrap();
    }
}

#[derive(clap::Parser, Clone, PartialEq, Eq, Hash, Debug)]
struct Args {
    #[arg(short, long, default_value = "demo/example/zenoh-rs-pub")]
    /// The key expression to write to.
    key: KeyExpr<'static>,
    #[arg(short, long, default_value = "Pub from Rust!")]
    /// The value to write.
    value: String,
    #[arg(short, long)]
    /// The attachments to add to each put.
    ///
    /// The key-value pairs are &-separated, and = serves as the separator between key and value.
    attach: Option<String>,
    #[command(flatten)]
    common: CommonArgs,
}

fn split_once(s: &str, c: char) -> (&[u8], &[u8]) {
    let s_bytes = s.as_bytes();
    match s.find(c) {
        Some(index) => {
            let (l, r) = s_bytes.split_at(index);
            (l, &r[1..])
        }
        None => (s_bytes, &[]),
    }
}

fn parse_args() -> (Config, KeyExpr<'static>, String, Option<String>) {
    let args = Args::parse();
    (args.common.into(), args.key, args.value, args.attach)
}
