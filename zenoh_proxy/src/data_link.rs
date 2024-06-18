use tokio_serial::SerialPortInfo;

use crate::limero::Sink;
use crate::limero::SinkTrait;
use crate::limero::SourceTrait;
use crate::limero::Src;

#[derive(Clone)]
pub enum DataLinkCmd {
    SendFrame { frame: Vec<u8> },
}

#[derive(Clone)]
pub enum DataLinkEvent {
    RecvFrame { frame: Vec<u8> },
}

struct DataLink {
    events: Src<DataLinkEvent>,
    commands: Sink<DataLinkCmd>,
    port_info: SerialPortInfo,
    input_buffer: Vec<u8>,
    output_buffer: Vec<u8>,
}

impl DataLink {
    fn new(port_info: SerialPortInfo) -> Self {
        let commands = Sink::new(100);
        let events = Src::new();
        DataLink {
            events,
            commands,
            port_info,
            input_buffer: Vec::new(),
            output_buffer: Vec::new(),
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
    async fn run() {
        loop {
            let message_decoder = MessageDecoder::new();
            let mut serial_stream = tokio_serial::new(port_info.port_name.clone(), 115200)
                .open_native_async()
                .unwrap();
            info!("Port {} opened", port_info.port_name.clone());
            input_buffer.clear();

            loop {
                select!{
                    _ = commands.read() => {
                        let cmd = commands.pop();
                        match cmd {
                            DataLinkCmd::SendFrame { frame } => {
                                let _res = serial_stream.try_write(&frame.as_slice());
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
                            info!("Port {} closed", port_info.port_name);
                            break;
                        } else {
                            self.push_input(buf[0..n].to_vec());
                            }
                        }
                }

            }
            serial_stream.close().unwrap();
        }
    }
}

impl SinkTrait<DataLinkCmd> for DataLink {
    fn push(&self, message: DataLinkCmd) {
        self.commands.push(message);
    }
}

impl SourceTrait<DataLinkEvent> for DataLink {
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<DataLinkEvent>>) {
        self.events.add_listener(sink);
    }
}
