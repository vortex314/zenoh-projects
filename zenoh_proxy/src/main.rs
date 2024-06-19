#![allow(unused_imports)]
#![allow(dead_code)]
use std::sync::Arc;

use chrono::offset;

use serde::de;
use tokio::select;
use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

mod logger;
use log::{debug, info};

mod protocol;
use protocol::msg::ProxyMessage;
use protocol::*;

mod proxy;
use proxy::*;

mod limero;
use limero::*;

mod transport;
use transport::*;
#[allow(unused_imports)]
#[derive(Clone)]
enum PortScannerEvent {
    PortAdded { port: SerialPortInfo },
    PortRemoved { port: SerialPortInfo },
}

#[derive(Clone)]
enum PortScannerCmd {
    Scan,
}

// this function will scan for available ports and add them to the shared list
struct PortScanner {
    events: Src<PortScannerEvent>,
    commands: Sink<PortScannerCmd>,
    active_ports: Vec<SerialPortInfo>,
    accepted_ports: Vec<PortPattern>,
    port_patterns: Vec<PortPattern>,
}

impl PortScanner {
    pub fn new(port_patterns: Vec<PortPattern>) -> Self {
        PortScanner {
            events:Src::new(),
            commands:Sink::new(10),
            active_ports: Vec::new(),
            accepted_ports: Vec::new(),
            port_patterns,
        }
    }

    async fn run(&mut self) {
        info!("Port scanner started ");
        loop {
            {
                info!("Scanning for new ports {} ", self.active_ports.len());

                let scanned_ports = available_ports().unwrap();
                let mut ignored_ports = Vec::new();
                scanned_ports.iter().for_each(|port_info| {
                    if !self.active_ports.contains(&port_info) {
                        match &port_info.port_type {
                            SerialPortType::UsbPort(usb_info) => {
                                self.accepted_ports.iter().for_each(|pattern| {
                                    if pattern.matches(&port_info) {
                                        info!("USB port {} : {:?} ", port_info.port_name, usb_info);
                                        self.active_ports.push(port_info.clone());
                                        self.events.emit(PortScannerEvent::PortAdded {
                                            port: port_info.clone(),
                                        });
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
}

impl SourceTrait<PortScannerEvent> for PortScanner {
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<PortScannerEvent>>) {
        self.events.add_listener(sink);
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

fn start_proxy(event: PortScannerEvent) -> Option<()> {
    match event {
        PortScannerEvent::PortAdded { port } => {
            info!("Port added : {:?}", port);
            let mut transport = Transport::new(port.clone());
            tokio::spawn(async move {
                transport.run().await;
            });
        }
        PortScannerEvent::PortRemoved { port } => {
            info!("Port removed : {:?}", port);
        }
    }
    None
}

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Error> {
    logger::init();
    info!("Starting Serial Proxy");

    let mut null_sink = Sink::<()>::new(1);

    let port_patterns = vec![PortPattern {
        name_regexp: "/dev/tty.*".to_string(),
        vid: Some(4292),
        pid: None,
        serial_number: None,
    }];

    let mut port_scanner = PortScanner::new(port_patterns);
    connect(&mut port_scanner, start_proxy, null_sink.sink_ref()); // start a proxy when port detected

    select! {
        _ = port_scanner.run()  => {
            info!("Port scanner task finished !! ");
        }
    }
    Ok(())
}
