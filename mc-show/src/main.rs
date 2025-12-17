use core::error;
use serde_derive::{Deserialize, Serialize};
use serde_json::Value;
use std::net::{Ipv4Addr, UdpSocket};
use std::str;
use std::time::Duration;

use anyhow::Result;
use log::{error, info};
mod logger;

const MULTICAST_GROUP: Ipv4Addr = Ipv4Addr::new(224, 0, 0, 1);
const MULTICAST_PORT: u16 = 50002;
const LISTEN_PORT: u16 = 50001;
const BUFFER_SIZE: usize = 1024;

#[derive(Debug, Serialize, Deserialize, Default)]
struct UdpMessage {
    dst: Option<String>,
    src: Option<String>,
    payload: Vec<u8>,
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    logger::init();
    // Create socket for receiving multicast messages
    let socket = UdpSocket::bind(format!("0.0.0.0:{}", LISTEN_PORT))?;
    socket.set_read_timeout(Some(Duration::from_secs(10)))?;

    // Join multicast group
    socket.join_multicast_v4(&MULTICAST_GROUP, &Ipv4Addr::UNSPECIFIED)?;

    info!(
        "Listening for multicast events on {}:{}",
        MULTICAST_GROUP, LISTEN_PORT
    );
    info!("Messages will be sent from port: {}", MULTICAST_PORT);
    info!("Press Ctrl+C to exit\n");

    let mut buffer = [0u8; BUFFER_SIZE];

    loop {
        match socket.recv_from(&mut buffer) {
            Ok((size, _src_addr)) => {
                // Try to parse as CBOR
                match parse_cbor_message(&buffer[..size]) {
                    Ok(parsed) => {
                        // payload as utf8
                        let payload_str = match str::from_utf8(&parsed.payload) {
                            Ok(s) => s.to_string(),
                            Err(_) => format!("{:2X?}", parsed.payload),
                        };
                        //
                        let s = format!(
                            "{:^20}| {:20}| {}",
                            parsed.dst.unwrap_or("-".to_string()),
                            parsed.src.unwrap_or("-".to_string()),
                            payload_str
                        );
                        println!("{}", s);
                    }
                    Err(e) => {
                        // If not CBOR, try to display as text
                        error!("Failed to parse CBOR: {}", e);
                    }
                }
            }
            Err(e) => {
                if e.kind() == std::io::ErrorKind::WouldBlock {
                    // Timeout occurred, continue listening
                    continue;
                }
                error!("Error receiving data: {}", e);
            }
        }
    }
}

fn parse_cbor_message(data: &[u8]) -> Result<UdpMessage> {
    // info!("Received CBOR data: {:2X?}", data);
    let value = serde_cbor::from_slice::<serde_cbor::Value>(data)?;
    let mut udp_message = UdpMessage::default();
    if let serde_cbor::Value::Array(array) = value {
        if array.len() == 3 {
            if let serde_cbor::Value::Text(dst) = &array[0] {
                udp_message.dst = Some(dst.clone());
            }
            if let serde_cbor::Value::Text(src) = &array[1] {
                udp_message.src = Some(src.clone());
            }
            if let serde_cbor::Value::Bytes(payload) = &array[2] {
                udp_message.payload = payload.clone();
            }
        } else {
            return Err(anyhow::anyhow!(
                "Unexpected CBOR array length: {}",
                array.len()
            ));
        }
    } else {
        return Err(anyhow::anyhow!("Expected CBOR array"));
    }

    Ok(udp_message)
}
