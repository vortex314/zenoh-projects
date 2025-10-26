#![warn(rust_2018_idioms)]

use futures::stream::StreamExt;
use limero::{Actor, CmdQueue, Endpoint, EventHandlers, Handler, Timer, Timers};
use log::info;
use std::{collections::VecDeque, env, io, str, thread::sleep, time::Duration};
use tokio::io::AsyncReadExt;
use tokio_util::codec::{Decoder, Encoder};

use bytes::BytesMut;
use tokio_serial::SerialPortBuilderExt;

#[cfg(unix)]
const DEFAULT_TTY: &str = "/dev/ttyUSB1";
#[cfg(windows)]
const DEFAULT_TTY: &str = "COM1";

const START_FRAME: u16 = 0xABCD;

#[derive(Debug, Clone)]
struct HbCmd {
    steer: i16,
    speed: i16,
}

#[derive(Debug, Clone)]
struct HbInfo {
    frame: u16,
    cmd1: i16,
    cmd2: i16,
    speed_right: i16,
    speed_left: i16,
    battery_voltage: i16,
    board_temperature: i16,
    cmd_led: u16,
    crc: u16,
}

impl HbCmd {
    fn new() -> HbCmd {
        HbCmd { steer: 0, speed: 0 }
    }
    fn encode(&self) -> Vec<u8> {
        let mut v = Vec::new();
        v.push((START_FRAME & 0xFF) as u8);
        v.push((START_FRAME >> 8) as u8);
        v.push((self.steer & 0xFF) as u8);
        v.push((self.steer >> 8) as u8);
        v.push((self.speed & 0xFF) as u8);
        v.push((self.speed >> 8) as u8);
        v
    }

    fn crc(&self) -> u16 {
        let mut crc = 0;
        crc = crc ^ (START_FRAME) as u16;
        crc = crc ^ self.steer as u16;
        crc = crc ^ self.speed as u16;
        crc
    }

    fn add_crc(&self, vec: &mut Vec<u8>) {
        let crc = self.crc();
        vec.push((crc & 0xFF) as u8);
        vec.push((crc >> 8) as u8);
    }
}

impl HbInfo {
    fn new() -> HbInfo {
        HbInfo {
            frame: 0,
            cmd1: 0,
            cmd2: 0,
            speed_right: 0,
            speed_left: 0,
            battery_voltage: 0,
            board_temperature: 0,
            cmd_led: 0,
            crc: 0,
        }
    }
    fn decode(&mut self, data: &mut VecDeque<u8>) {
        self.frame = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
        self.cmd1 = data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.cmd2 = data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.speed_right =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.speed_left =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.battery_voltage =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.board_temperature =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.cmd_led = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
        self.crc = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
    }

    fn crc(&self) -> u16 {
        let mut crc = 0;
        crc = crc ^ self.frame as u16;
        crc = crc ^ self.cmd1 as u16;
        crc = crc ^ self.cmd2 as u16;
        crc = crc ^ self.speed_right as u16;
        crc = crc ^ self.speed_left as u16;
        crc = crc ^ self.battery_voltage as u16;
        crc = crc ^ self.board_temperature as u16;
        crc = crc ^ self.cmd_led as u16;
        crc
    }
}

#[tokio::main]
async fn main() -> tokio_serial::Result<()> {
    limero::logger::init();

    let scanned_ports = tokio_serial::available_ports()?;
    info!("Available ports: {:?}", scanned_ports);

    info!("Starting serial port reader");
    let mut args = env::args();
    let tty_path = args.nth(1).unwrap_or_else(|| DEFAULT_TTY.into());
    info!("Reading from serial port: {}", tty_path);

    let mut serial_stream = tokio_serial::new(tty_path, 19200).open_native_async()?;
    info!("Serial port opened");
    #[cfg(unix)]
    serial_stream
        .set_exclusive(false)
        .expect("Unable to set serial port exclusive to false");

    let mut uart_actor = UartActor::new(serial_stream);

    uart_actor.for_each(|event| {
        info!("Received event: {:?}", event);
    });

    tokio::spawn(async move {
        uart_actor.run().await;
    });
    sleep(Duration::from_secs(100000));
    Ok(())
}

#[derive(Debug, Clone)]
enum UartCmd {
    Send(HbCmd),
}
#[derive(Debug, Clone)]

enum UartEvent {
    Recv(HbInfo),
}
struct UartActor {
    serial_stream: tokio_serial::SerialStream,
    cmds: CmdQueue<UartCmd>,
    events: EventHandlers<UartEvent>,
    timers: Timers,
    buffer: VecDeque<u8>,
    prev_byte: u8,
}

impl UartActor {
    fn new(serial_stream: tokio_serial::SerialStream) -> UartActor {
        UartActor {
            serial_stream,
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            buffer: VecDeque::new(),
            prev_byte: 0,
        }
    }

    fn fill_buffer(&mut self, byte: u8) -> Option<HbInfo> {
        if byte == (START_FRAME >> 8) as u8 {
            if self.buffer.len() > 0 && self.prev_byte == (START_FRAME & 0xFF) as u8 {
                self.buffer.clear();
                self.buffer.push_back((START_FRAME & 0xFF) as u8);
                self.buffer.push_back((START_FRAME >> 8) as u8);
                info!("Restarting buffer");
            } else {
                self.buffer.push_back(byte);
            }
        } else {
            self.buffer.push_back(byte);
        }
        self.prev_byte = byte;
              info!("Received: {:02X?}", self.buffer);

        if self.buffer.len() > 100 {
            self.buffer.clear();
            info!("Buffer too long");
        }
        if self.buffer.len() == 18
            && self.buffer[1] == (START_FRAME >> 8) as u8
            && self.buffer[0] == (START_FRAME & 0xFF) as u8
        {
            info!("Received: {:02X?}", self.buffer);
            let mut info = HbInfo::new();
            info.decode(&mut self.buffer);
            self.buffer.clear();
            Some(info)
        } else {
            None
        }
    }
}
impl Actor<UartCmd, UartEvent> for UartActor {
    async fn run(&mut self) {
        self.timers
            .add_timer(Timer::new_repeater(5, Duration::from_millis(10)));
        loop {
            tokio::select! {
                cmd = self.cmds.next() => {
                    match cmd {
                        Some(UartCmd::Send(cmd)) => {

                            let mut data = cmd.encode();
                            cmd.add_crc(&mut data);
                            info!("Sending: {:02X?}", data);
                            let r = self.serial_stream.try_write(&data);
                            match r {
                                Ok(_) => {
                                    info!("Sent: {:?}", cmd);
                                }
                                Err(e) => {
                                    info!("Error sending data: {}", e);
                                }
                            }
                        }
                        _ => {}
                    }
                }
                   res = self.serial_stream.readable() => {
                    let mut buf = [0; 1024];
                    match  self.serial_stream.read(&mut buf).await {
                        Ok(n) => {
                            if n == 0 {
                                info!("EOF");
                                break;
                            }
                            for i in 0..n {
                                if let Some(info)  = self.fill_buffer(buf[i]) {
                                    self.events.handle(&UartEvent::Recv(info));
                                }
                            }
                        }
                        Err(e) => {
                            info!("Error reading data: {}", e);
                        }
                    };
                }
                timeout = self.timers.alarm() => {
                    info!("Timeout");
                    let cmd = HbCmd { steer: 0, speed: 300 };
                    let mut data = cmd.encode();
                    cmd.add_crc(&mut data);
                    info!("Sending: {:02X?}", data);
                    let r = self.serial_stream.try_write(&data);
                    match r {
                        Ok(_) => {
                            info!("Sent: {:?}", cmd);
                        }
                        Err(e) => {
                            info!("Error sending data: {}", e);
                        }
                    }
                    // self.cmds.handler().handle(&UartCmd::Send(v));
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Endpoint<UartEvent>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Endpoint<UartCmd> {
        self.cmds.handler()
    }
}
