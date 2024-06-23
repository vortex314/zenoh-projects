use cobs::CobsDecoder;
use log::info;
use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;
use tokio::sync::Mutex;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};

use std::io::Write;

use crate::limero::Sink;
use crate::limero::SinkTrait;
use crate::limero::SourceTrait;
use crate::limero::Source;
use crate::limero::SinkRef;

use crate::encode_frame;
use crate::protocol::msg::*;
use crate::protocol::MessageDecoder;


const MTU_SIZE: usize = 1023;

#[derive(Clone)]
pub enum TransportCmd {
    SendMessage { message: MqttSnMessage },
}

#[derive(Clone)]
pub enum TransportEvent {
    RecvMessage { message: MqttSnMessage },
}

pub struct Transport {
    events: Source<TransportEvent>,
    commands: Sink<TransportCmd>,
    port_info: SerialPortInfo,
    message_decoder: MessageDecoder,
}

impl Transport {
    pub fn new(port_info: SerialPortInfo) -> Self {
        let commands = Sink::new(100);
        let events = Source::new();
        Transport {
            events,
            commands,
            port_info,
            message_decoder: MessageDecoder::new(),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<TransportCmd> {
        self.commands.sink_ref()
    }

    pub async fn run(&mut self) {
        const GREEN: &str = "\x1b[0;32m";
        const RESET: &str = "\x1b[m";
        loop {
            let _message_decoder = MessageDecoder::new();
            let mut buf = [0; MTU_SIZE];
            let  serial_stream = tokio_serial::new(self.port_info.port_name.clone(), 115200)
                .open_native_async();
            if serial_stream.is_err() {
                info!("Error opening port {}", self.port_info.port_name.clone());
                break;
            }
            let mut serial_stream = serial_stream.unwrap();
            info!("Port {} opened", self.port_info.port_name.clone());

            loop {
                select! {
                    cmd = self.commands.read() => {
                        match cmd.unwrap() {
                            TransportCmd::SendMessage { message } => {
                                info!("Sending Message : {:?}", message);
                                let x = encode_frame(message);
                                let _res = serial_stream.try_write(&x.unwrap().as_slice());
                                let _r = serial_stream.flush();
                                if _res.is_err()  || _r.is_err() {
                                    info!("Error writing to serial port");
                                }
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
                            let _res = self.message_decoder.decode(&buf[0..n]);
                            if _res.is_empty() {
                                let line = String::from_utf8(buf[0..n].to_vec()).ok();
                                if line.is_some() {
                                    print!("{}{}{}", GREEN, line.unwrap(), RESET);
                                    std::io::stdout().flush().unwrap();
                                };
                            } else {
                                for message in _res {
                                    info!("Received Message : {:?}", message);
                                    self.events.emit(TransportEvent::RecvMessage { message: message });
                                }
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
}


impl SourceTrait<TransportEvent> for Transport {
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<TransportEvent>>) {
        self.events.subscribe(sink);
    }
}
