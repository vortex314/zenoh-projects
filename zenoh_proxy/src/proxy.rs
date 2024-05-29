use crate::protocol;
use protocol::decode_frame;
use protocol::encode_frame;
use protocol::msg::ProxyMessage;
use protocol::MTU_SIZE;
use protocol::MessageDecoder;
use bytes::BytesMut;
use log::*;
use minicbor::encode;
use std::arch::x86_64::_MM_FLUSH_ZERO_MASK;
use std::io;
use std::io::Write;
use std::result::Result;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};

const GREEN : &str = "\x1b[0;32m";
const RESET : &str = "\x1b[m";

pub async fn port_handler(port_info: SerialPortInfo) -> Result<(), String> {
    let mut message_decoder = MessageDecoder::new();
    info!(
        "Port Handler started for port {}",
        port_info.port_name.clone()
    );

    let mut serial_stream = tokio_serial::new(port_info.port_name.clone(), 115200)
        .open_native_async()
        .unwrap();
    info!("Port {} opened", port_info.port_name.clone() );
    loop {
        let has_data = serial_stream.readable().await;
        if has_data.is_ok() {
            let mut buf = [0; MTU_SIZE];
            let n = serial_stream.read(&mut buf).await.unwrap();
            if n == 0 {
                info!("Port {} closed", port_info.port_name);
                break;
            } else {

                let messages = message_decoder.decode(&buf[0..n]);

                if messages.is_empty() {
                    let line = String::from_utf8(buf[0..n].to_vec()).ok();
                 if line.is_some() { 
                        print!("{}{}{}",GREEN,line.unwrap(),RESET);
                        std::io::stdout().flush().unwrap();
                    };
                }
                for message in messages {
                    info!("Received Message : {:?}", message);
                    match message {
                        ProxyMessage::Connect {
                            protocol_id: _,
                            duration: _,
                            client_id: _,
                        } => {
                            let conn_ack = ProxyMessage::ConnAck { return_code: 0 };
                            info!("Send Msg : {:?}", conn_ack);
                            let bytes = encode_frame(conn_ack)?;
                            let _res = serial_stream.try_write(&bytes.as_slice());
                            serial_stream.flush();
                            if _res.is_err() {
                                info!("Error writing to serial port");
                            }
                        }
                        ProxyMessage::PingReq => {
                            let ping_resp = ProxyMessage::PingResp;
                            info!("Send Msg : {:?}", ping_resp);
                            let bytes = encode_frame(ping_resp)?;
                            let _res = serial_stream.try_write(&bytes.as_slice());
                            serial_stream.flush();
                            if _res.is_err() {
                                info!("Error writing to serial port");
                            }
                        }
                        _ => {
                        }
                    }
                }
            }
        } else if has_data.is_err() {
            info!("Port {} closed", port_info.port_name);
            break;
        }
    }
    Ok(())
}
