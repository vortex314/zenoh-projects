use crate::limero::Sink;
use crate::limero::SinkTrait;
use crate::limero::Source;
use crate::limero::SourceTrait;
use crate::protocol::msg::*;
use crate::protocol::MessageDecoder;
use crate::protocol::MTU_SIZE;
use log::*;
use minicbor::decode::info;
use tokio_serial::available_ports;
use tokio_serial::SerialPortInfo;
use tokio_serial::SerialPortType;

#[derive(Clone)]
pub enum PortScannerEvent {
    PortAdded { port: SerialPortInfo },
    PortRemoved { port: SerialPortInfo },
}

#[derive(Clone)]
pub enum PortScannerCmd {
    Scan,
}

// this function will scan for available ports and add them to the shared list
pub struct PortScanner {
    events: Source<PortScannerEvent>,
    commands: Sink<PortScannerCmd>,
    active_ports: Vec<SerialPortInfo>,
    port_patterns: Vec<PortPattern>,
}

impl PortScanner {
    pub fn new(port_patterns: Vec<PortPattern>) -> Self {
        PortScanner {
            events: Source::new(),
            commands: Sink::new(10),
            active_ports: Vec::new(),
            port_patterns,
        }
    }

    pub async fn run(&mut self) {
        info!("Port scanner started ");
        loop {
            info!("Scanning for new ports {} ", self.active_ports.len());

            let scanned_ports = available_ports().unwrap();
            scanned_ports.iter().for_each(|port_info| {
                if self.matches(port_info) {
                    info!(
                        "Port : {:?} active count {}",
                        port_info.port_name,
                        self.active_ports.len()
                    );
                    if self.active_ports.contains(port_info) {
                        info!("Port : {:?} already active ", port_info.port_name);
                    } else {
                        info!("Port : {:?} added ", port_info.port_name);
                        self.active_ports.push(port_info.clone());
                        self.events.emit(PortScannerEvent::PortAdded {
                            port: port_info.clone(),
                        });
                    }
                }
            });
            self.active_ports.retain(|port_info| {
                if scanned_ports.contains(port_info) {
                    return true;
                } else {
                    info!("Port : {:?} removed ", port_info.port_name);
                    self.events.emit(PortScannerEvent::PortRemoved {
                        port: port_info.clone(),
                    });
                    return false;
                }
            });
            tokio::time::sleep(tokio::time::Duration::from_secs(20)).await;
        }
    }

    fn matches(&self, port_info: &SerialPortInfo) -> bool {
        self.port_patterns
            .iter()
            .any(|pattern| pattern.matches(port_info))
    }
}

impl SourceTrait<PortScannerEvent> for PortScanner {
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<PortScannerEvent>>) {
        self.events.subscribe(sink);
    }
}
pub struct PortPattern {
    pub name_regexp: String,
    pub vid: Option<u16>,
    pub pid: Option<u16>,
    pub serial_number: Option<String>,
}

impl PortPattern {
    pub fn matches(&self, port_info: &SerialPortInfo) -> bool {
        if !self.name_regexp.is_empty() {
            let re = regex::Regex::new(&self.name_regexp).unwrap();
            if !re.is_match(&port_info.port_name) {
                debug!(
                    "Port name {} does not match {} ",
                    port_info.port_name, self.name_regexp
                );
                return false;
            }
        }
        match &port_info.port_type {
            SerialPortType::UsbPort(usb_info) => {
                if let Some(vid) = self.vid {
                    if usb_info.vid != vid {
                        debug!("Port vid {} does not match {} ", usb_info.vid, vid);
                        return false;
                    }
                }
                if let Some(pid) = self.pid {
                    if usb_info.pid != pid {
                        debug!("Port pid {} does not match {} ", usb_info.pid, pid);
                        return false;
                    }
                }
                if let Some(serial) = &self.serial_number {
                    if usb_info.serial_number != Some(serial.clone()) {
                        debug!(
                            "Port serial {} does not match {:?} ",
                            usb_info.serial_number.clone().unwrap(),
                            serial
                        );
                        return false;
                    }
                }
                return true;
            }
            _ => {
                debug!("Port type does not match USB ");
                return false;
            }
        }
    }
}
