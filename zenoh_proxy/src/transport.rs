use byte::TryWrite;
use log::debug;
use log::info;
use minicbor::decode::info;
use minicbor::Decoder;
use minicbor::Encoder;
use minicbor::decode::Decode;
use minicbor::encode::Encode;
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
    SendMessage(EspNowMessage),
}

#[derive(Clone)]
pub enum TransportEvent {
    RecvMessage(EspNowMessage),
    ConnectionLost {},
}

pub struct Transport {
    events: EventHandlers<TransportEvent>,
    commands: CmdQueue<TransportCmd>,
    port_info: SerialPortInfo,
    frame_extractor: FrameExtractor,
}

impl Transport {
    pub fn new(port_info: SerialPortInfo) -> Self {
        let commands = CmdQueue::new(2);
        let events = EventHandlers::new();
        Transport {
            events,
            commands,
            port_info,
            frame_extractor: FrameExtractor::new(),
        }
    }
}

impl Actor<TransportCmd, TransportEvent> for Transport {
    async fn run(&mut self) {
        const GREEN: &str = "\x1b[0;32m";
        const RESET: &str = "\x1b[m";
        loop {
            let mut buf = [0; MTU_SIZE];
            let serial_stream =
                tokio_serial::new(self.port_info.port_name.clone(), 921600).open_native_async();
            if serial_stream.is_err() {
                info!("Error opening port {}", self.port_info.port_name.clone());
                self.events.handle(&TransportEvent::ConnectionLost {});
                return;
            }
            let mut serial_stream = serial_stream.unwrap();
            info!("Port {} opened", self.port_info.port_name.clone());

            loop {
                select! {
                cmd = self.commands.next() => {
                    match cmd.unwrap() {
                        TransportCmd::SendMessage ( message ) => {
                            encode_frame(&message).map(|frame| {
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
                            let line = String::from_utf8(buf[0..n].to_vec()).ok();
                            if line.is_some() {
                                print!("{}{}{}", GREEN, line.unwrap(), RESET);
                                std::io::stdout().flush().unwrap();
                            };
                        } else {
                            for frame in _res {

                                let mut decoder = minicbor::Decoder::new(frame.as_slice());
                                let r = EspNowMessage::decode(&mut decoder,&mut ());
                                let r = decoder.decode::<EspNowMessage>();
                              //  self.events.handle(&TransportEvent::RecvMessage ( esp_now_message ));
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
        self.events.add_listener(handler);
    }
}

fn bytes_to_string(bytes: &[u8]) -> String {
    let mut s = String::new();
    for b in bytes {
        s.push_str(&format!("{:02X} ", b));
    }
    s
}
