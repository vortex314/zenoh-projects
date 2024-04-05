use std::sync::Arc;

use chrono::offset;
use crc::{
    poly::{CRC_16, CRC_16_ANSI},
    Crc, CRC_16_IBM_SDLC,
};
use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

mod logger;
use log::{debug, info};

mod protocol;
use protocol::Log;

use cobs::CobsEncoder;
use minicbor::{
    decode::info,
    encode::{write::EndOfSlice, Write},
    Decode, Decoder, Encode, Encoder,
};

// this function will scan for available ports and add them to the shared list

async fn scan_available_ports(
    active_ports: Arc<Mutex<Vec<SerialPortInfo>>>,
    accepted_ports: Vec<PortPattern>,
) {
    info!("Port scanner started ");
    loop {
        {
            let mut active_ports = active_ports.lock().await;
            info!("Scanning for new ports {} ", active_ports.len());

            let scanned_ports = available_ports().unwrap();
            let mut ignored_ports = Vec::new();
            scanned_ports.iter().for_each(|port_info| {
                if !active_ports.contains(&port_info) {
                    match &port_info.port_type {
                        SerialPortType::UsbPort(usb_info) => {
                            accepted_ports.iter().for_each(|pattern| {
                                if pattern.matches(&port_info) {
                                    info!("USB port {} : {:?} ", port_info.port_name, usb_info);
                                    active_ports.push(port_info.clone());
                                }
                            });
                        }
                        _ => {
                            ignored_ports.push(port_info.clone());
                            debug!(
                                "Ignore port : {:?} - {:?} ",
                                port_info.port_name, port_info.port_type
                            );
                        }
                    }
                    //                   tx.try_send(port.clone()).unwrap();
                }
            });
            //drop(active_ports);
        }
        tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
    }
}
struct PortPattern {
    name_regexp: String,
    vid: Option<u16>,
    pid: Option<u16>,
    serial_number: Option<String>,
}

impl PortPattern {
    fn matches(&self, port_info: &SerialPortInfo) -> bool {
        if !self.name_regexp.is_empty() {
            let re = regex::Regex::new(&self.name_regexp).unwrap();
            if !re.is_match(&port_info.port_name) {
                return false;
            }
        }
        match &port_info.port_type {
            SerialPortType::UsbPort(usb_info) => {
                if let Some(vid) = self.vid {
                    if usb_info.vid != vid {
                        return false;
                    }
                }
                if let Some(pid) = self.pid {
                    if usb_info.pid != pid {
                        return false;
                    }
                }
                if let Some(serial) = &self.serial_number {
                    if usb_info.serial_number != Some(serial.clone()) {
                        return false;
                    }
                }
                return true;
            }
            _ => {
                return false;
            }
        }
    }
}

use core::result::Result;

struct VecWriter {
    buffer: Vec<u8>,
}

impl VecWriter {
    fn new() -> Self {
        VecWriter { buffer: Vec::new() }
    }

    fn len(&self) -> usize {
        self.buffer.len()
    }

    fn to_bytes(&self) -> &[u8] {
        self.buffer.as_slice()
    }

    fn push(&mut self, data: u8) {
        self.buffer.push(data);
    }

    fn to_vec(&self) -> Vec<u8> {
        self.buffer.to_vec()
    }

    fn clear(&mut self) {
        self.buffer.clear();
    }
}
impl minicbor::encode::Write for VecWriter {
    type Error = EndOfSlice;
    // Required method
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.extend_from_slice(buf);
        Ok(())
    }
}

fn make_frame(msg: protocol::Message) -> Result<Vec<u8>, String> {
    let mut buffer = VecWriter::new();
    let mut encoder = Encoder::new(&mut buffer);
    let mut ctx = 1;
    let _ = msg.encode(&mut encoder, &mut ctx);
    info!("Encoded cbor : {:02X?}", buffer.to_bytes());
    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&buffer.to_bytes());
    info!("CRC : {:04X}", crc);
    buffer.push((crc & 0xFF) as u8);
    buffer.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_buffer = [0; 256];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer.to_bytes()).unwrap();
    let size = cobs_encoder.finalize().unwrap();
    buffer.push(0);
    info!("COBS : {:02X?}", &cobs_buffer[0..(size+1)]);
    Ok(cobs_buffer[0..size+1].to_vec())
}

fn decode_frame(frame_bytes: Vec<u8>) -> Result<protocol::Message, String> {
    if frame_bytes.len() < 2 {
        return Err("Frame too short".to_string());
    }
    let mut cobs_output = [0; 256];
    let mut cobs_decoder = cobs::CobsDecoder::new(&mut cobs_output);
    let res = cobs_decoder.push(&frame_bytes);
    match res {
        Ok(offset) => {
            match offset {
                None => {
                    return Err("COBS more data needed.".to_string());
                },
                Some((n, m)) => {
                    if m != frame_bytes.len() {
                        return Err("COBS decoding error".to_string());
                    }
                    let bytes = cobs_output[0..n].to_vec();
                    info!("Decoded : {:02X?}", bytes);

                    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
                    let crc = crc16.checksum(&bytes[0..(bytes.len() - 2)]);
                    let crc_received =
                        (bytes[bytes.len() - 1] as u16) << 8 | bytes[bytes.len() - 2] as u16;
                    if crc != crc_received {
                        info!("CRC error : {:04X} != {:04X}", crc, crc_received);
                        return Err("CRC error".to_string());
                    }

                    let mut decoder = minicbor::decode::Decoder::new(&bytes);
                    let mut ctx = 1;
                    let msg = protocol::Message::decode(&mut decoder, &mut ctx).unwrap();
                    Ok(msg)
                }
            }
        },
        Err(j) => {
            info!("COBS decoding error : {:?}", j);
            Err("COBS decoding error".to_string())
        },
    }
}

fn encode_log() -> () {
    let log_msg = protocol::Message::new_log("Hello, World!");
    let bytes = make_frame(log_msg).unwrap();
    info!("Encoded : {:02X?}", bytes);
    let _msg = decode_frame(bytes).unwrap();
    info!("Decoded : {:?}", _msg);
}

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Error> {
    logger::init();

    encode_log();
    let port_patterns = vec![PortPattern {
        name_regexp: "/dev/tty.*".to_string(),
        vid: Some(4292),
        pid: None,
        serial_number: None,
    }];
    let active_ports = Arc::new(Mutex::new(Vec::new()));
    //    let (tx,rx) = tokio::sync::mpsc::channel(100);
    // spawn task to collect new serial USB devices
    let a_ports = active_ports.clone();
    let _port_scanner_task =
        tokio::spawn(async move { scan_available_ports(a_ports, port_patterns).await });
    // spawn task to read from serial port

    let _port_remover_task = tokio::spawn(async move {
        loop {
            let mut active_ports = active_ports.lock().await;
            info!("Remove port 0");
            if active_ports.len() > 0 {
                active_ports.remove(0);
            };
            drop(active_ports);
            tokio::time::sleep(tokio::time::Duration::from_secs(3)).await;
        }
    });

    tokio::time::sleep(tokio::time::Duration::from_secs(50)).await;

    Ok(())
}
