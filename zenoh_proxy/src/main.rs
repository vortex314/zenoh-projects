#![allow(unused_imports)]
#![allow(dead_code)]
use std::sync::Arc;

use chrono::offset;

use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

mod logger;
use log::{debug, info};

mod protocol;
use protocol::*;

mod proxy;
use proxy::*;

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
                                    tokio::spawn(port_handler(port_info.clone()));
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
        tokio::time::sleep(tokio::time::Duration::from_secs(20)).await;
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





#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Error> {
    logger::init();
    let m =  ProxyMessage::ConnAck { return_code: 3 };
    let _bytes = protocol::encode_frame(m).unwrap();

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


    tokio::time::sleep(tokio::time::Duration::from_secs(50)).await;

    Ok(())
}
