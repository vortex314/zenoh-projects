use tokio::io::split;
use tokio::io::AsyncReadExt;
use tokio_serial::*;
use tokio_util::codec::{Decoder, Encoder};
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;
use tokio::sync::Mutex;
use log::info;
use cobs::CobsDecoder;

use crate::limero::Sink;
use crate::limero::SinkTrait;
use crate::limero::SourceTrait;
use crate::limero::Src;

use crate::protocol::msg::*;
use crate::protocol::MessageDecoder;
use crate::encode_frame;

const MTU_SIZE: usize = 1023;

#[derive(Clone)]
pub enum TransportCmd {
    SendFrame { frame: ProxyMessage },
}

#[derive(Clone)]
pub enum TransportEvent {
    RecvFrame { frame: ProxyMessage },
}

struct Transport {
    events: Src<TransportEvent>,
    commands: Sink<TransportCmd>,
    port_info: SerialPortInfo,
    input_buffer: Vec<u8>,
    output_buffer: Vec<u8>,
    message_decoder: MessageDecoder,
}

impl Transport {
    fn new(port_info: SerialPortInfo) -> Self {
        let commands = Sink::new(100);
        let events = Src::new();
        Transport {
            events,
            commands,
            port_info,
            input_buffer: Vec::new(),
            output_buffer: Vec::new(),
            message_decoder: MessageDecoder::new(),
        }
    }
    fn cobs_decoder(&self, data: Vec<u8>) -> Option<Vec<u8>> {
        let mut output = [0; MTU_SIZE + 2];
        let mut decoder = CobsDecoder::new(&mut output);
        let res = decoder.push(&data);
    }
    fn check_crc(&self, data: &[u8]) -> bool {
        let mut crc = 0;
        for byte in data {
            crc = crc ^ *byte;
        }
        crc == 0
    }
    fn push_input(&mut self, data: Vec<u8>) {
        for byte in data {
            if byte == 0 {

            }
            self.input_buffer.push(byte);
        }
    }
    async fn run(&mut self) {
        loop {
            let message_decoder = MessageDecoder::new();
            let mut serial_stream = tokio_serial::new(self.port_info.port_name.clone(), 115200)
                .open_native_async()
                .unwrap();
            info!("Port {} opened", self.port_info.port_name.clone());
            self.input_buffer.clear();

            loop {
                select!{
                    cmd = self.commands.read() => {
                        match cmd.unwrap() {
                            TransportCmd::SendFrame { frame } => {
                                let x = encode_frame(frame);
                                let _res = serial_stream.try_write(&x);
                                serial_stream.flush();
                                if _res.is_err() {
                                    info!("Error writing to serial port");
                                }
                            }
                        }
                    }
                    _ = serial_stream.readable() => {
                        let mut buf = [0; MTU_SIZE];
                        let n = serial_stream.read(&mut buf).await.unwrap();
                        if n == 0 {
                            info!("Port {} closed", self.port_info.port_name);
                            break;
                        } else {
                            let _res = self.message_decoder.push_input(&buf[0..n]);
                            }
                        }
                }

            }
            serial_stream.close().unwrap();
        }
    }
}

impl SinkTrait<TransportCmd> for Transport {
    fn push(&self, message: TransportCmd) {
        self.commands.push(message);
    }
}

impl SourceTrait<TransportEvent> for Transport {
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<TransportEvent>>) {
        self.events.add_listener(sink);
    }
}
