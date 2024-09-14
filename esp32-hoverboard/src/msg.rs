use minicbor::{Decoder};

use anyhow::Result;
use alloc::vec::Vec;
use alloc::collections::VecDeque;
use serdes::{Cbor, PayloadCodec};

#[derive(Debug, Clone)]
enum Ps4PropIdx {
    LeftButtons = 0,
    LeftAxisX,
    LeftAxisY,
    RightAxisX,
    RightAxisY,
    RightButtons,
    MiscButtons,
    RedLed, // =7
    GreenLed,
    BlueLed,
    Rumble,
}

struct Ps4Props {
    left_buttons: u8,
    left_axis_x: i8,
    left_axis_y: i8,
    right_axis_x: i8,
    right_axis_y: i8,
    right_buttons: u8,
    misc_buttons: u8,
}

impl Ps4Props {
    fn decode(data: &Vec<u8>) -> Result<Ps4Props> {
        let mut decoder = Decoder::new(data.as_slice());
        let left_buttons = decoder.decode().map_err(anyhow::Error::msg)?;
        let left_axis_x = decoder.decode().map_err(anyhow::Error::msg)?;
        let left_axis_y = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_axis_x = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_axis_y = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_buttons = decoder.decode().map_err(anyhow::Error::msg)?;
        let misc_buttons = decoder.decode().map_err(anyhow::Error::msg)?;
        Ok(Ps4Props {
            left_buttons,
            left_axis_x,
            left_axis_y,
            right_axis_x,
            right_axis_y,
            right_buttons,
            misc_buttons,
        })
    }
}

pub const START_FRAME: u16 = 0xABCD;

#[derive(Debug, Clone)]
pub struct MotorCmd {
    pub speed: i16,
    pub steer: i16,
}

impl MotorCmd {
    pub fn encode(&self) -> Vec<u8> {
        let mut v = Vec::new();
        v.push((START_FRAME & 0xFF) as u8);
        v.push((START_FRAME >> 8) as u8);
        v.push((self.steer & 0xFF) as u8);
        v.push((self.steer >> 8) as u8);
        v.push((self.speed & 0xFF) as u8);
        v.push((self.speed >> 8) as u8);
        self.add_crc(&mut v);
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

#[derive(Debug, Clone)]
pub struct MotorEvent {
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

impl MotorEvent {
    pub fn new() -> MotorEvent {
        MotorEvent {
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
    pub fn decode(&mut self, data: &mut VecDeque<u8>) {
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
