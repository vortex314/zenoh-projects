use byte::TryWrite;
use log::debug;
use log::info;
use minicbor::decode::info;
use minicbor::decode::Decode;
use minicbor::encode::Encode;
use minicbor::Decoder;
use minicbor::Encoder;
use msg::encode_frame;
use msg::esp_now::SendCmd;
use msg::EspNowMessage;
use msg::FrameExtractor;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;
use tokio::sync::Mutex;
use tokio_serial::*;

use std::io::Write;

use limero::Actor;
use limero::CmdQueue;
use limero::EventHandlers;
use limero::Handler;

const MTU_SIZE: usize = 1023;

#[derive(Clone)]
pub enum TransportCmd {
    SendMessage(Vec<u8>),
}

#[derive(Clone)]
pub enum TransportEvent {
    RecvMessage(Vec<u8>),
    ConnectionLost {},
}

pub struct Transport {
    event_handlers: EventHandlers<TransportEvent>,
    commands: CmdQueue<TransportCmd>,
    port_info: SerialPortInfo,
    frame_extractor: FrameExtractor,
}

impl Transport {
    pub fn new(port_info: SerialPortInfo) -> Self {
        Transport {
            event_handlers: EventHandlers::new(),
            commands: CmdQueue::new(2),
            port_info,
            frame_extractor: FrameExtractor::new(),
        }
    }
}

impl Actor<TransportCmd, TransportEvent> for Transport {
    async fn run(&mut self) {
        info!("Transport started on port {}", self.port_info.port_name);
        const GREEN: &str = "\x1b[0;32m";
        const RESET: &str = "\x1b[m";
        loop {
            let mut buf = [0; MTU_SIZE];
            let serial_stream =
                tokio_serial::new(self.port_info.port_name.clone(), 115200).open_native_async();
            if serial_stream.is_err() {
                info!("Error opening port {}", self.port_info.port_name.clone());
                self.event_handlers
                    .handle(&TransportEvent::ConnectionLost {});
                return;
            }
            let mut serial_stream = serial_stream.unwrap();
            info!("Port {} opened", self.port_info.port_name.clone());

            loop {
                select! {
                cmd = self.commands.next() => {
                    match cmd.unwrap() {
                        TransportCmd::SendMessage ( message ) => {
                            let _ = encode_frame(&message).map(|frame| {
                                let _res = serial_stream.try_write(&frame);
                                let _r = serial_stream.flush();
                                if _res.is_err()  || _r.is_err() {
                                    info!("Error writing to serial port");
                                }
                            });
                        }
                    }
                }
                count = serial_stream.read(&mut buf) => {
                    if count.is_err() {
                        info!("Port {} closed", self.port_info.port_name);
                        break;
                    } else {
                    let n = count.unwrap();

                    if n == 0 {
                        info!("Port {} closed", self.port_info.port_name);
                        break;
                    } else {
                        let _res = self.frame_extractor.decode(&buf[0..n]);
                        if _res.is_empty() {

                            /*let line = String::from_utf8(buf[0..n].to_vec()).ok();
                            if line.is_some() {
                                print!("{}{}{}", GREEN, line.unwrap(), RESET);
                                std::io::stdout().flush().unwrap();
                            } else { // show in hex
                                for b in &buf[0..n] {
                                    print!("{:02X} ", b);
                                }
                                println!();
                            }*/
                        } else {
                            for frame in _res {
                                debug!("Frame : {}", bytes_to_string(&frame));
                                self.event_handlers.handle(&TransportEvent::RecvMessage(frame.clone()));
                                };
                            }
                        }
                    }
                    }
                }
            }
            info!("Port {} closing", self.port_info.port_name);
            // serial_stream.close().unwrap();
        }
    }

    fn handler(&self) -> Box<dyn Handler<TransportCmd>> {
        self.commands.handler()
    }

    fn add_listener(&mut self, handler: Box<dyn Handler<TransportEvent>>) {
        self.event_handlers.add_listener(handler);
    }
}

pub fn bytes_to_string(bytes: &[u8]) -> String {
    let mut s = String::new();
    for b in bytes {
        s.push_str(&format!("{:02X} ", b));
    }
    s
}
