
// ignore dead code
#![allow(dead_code)]
// allow unusde variables
#![allow(unused_variables)]

use const_fnv1a_hash::fnv1a_hash_32;
use minicbor::{Decoder,Encoder};

use alloc::collections::VecDeque;
use alloc::vec::Vec;
use anyhow::Result;

pub enum  MsgType {
    Alive = 0,
    PubReq = 1,
    Pub1Req,
    PingReq,
    NameReq,
    DescReq,
    SetReq,
    GetReq,
}  

pub fn reply( msg_type : MsgType ) -> u8 {
    msg_type as u8 | 0x80
}

pub fn request( msg_type : MsgType ) -> u8 {
    msg_type as u8
}



#[derive(Debug, Clone)]
pub struct EspNowHeader {
    pub dst: Option<u32>, // always known , can be broadcast = 0
    pub src: Option<u32>, //
    pub msg_type: u8,
    pub msg_id: Option<u16>,
}

impl EspNowHeader {
    /*pub fn new() -> EspNowHeader {
        EspNowHeader {
            dst: None,
            src: None,
            msg_type: 0,
            msg_id: None,
        }
    }*/
    pub fn from_decoder(decoder: &mut Decoder) -> Result<EspNowHeader> {
        let _length = decoder.array().map_err(anyhow::Error::msg)?;
        if _length.is_some() {
            return Err(anyhow::Error::msg("Invalid array length"));
        }
        let dst: Option<u32> = decoder.decode().map_err(anyhow::Error::msg)?;
        let src: Option<u32> = decoder.decode().map_err(anyhow::Error::msg)?;
        let msg_type: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        let msg_id: Option<u16> = decoder.decode().map_err(anyhow::Error::msg)?;
        Ok(EspNowHeader {
            dst,
            src,
            msg_type,
            msg_id,
        })
    }
    pub fn encode(&self, encoder: &mut Encoder<Vec<u8>>) -> Result<()> {
        encoder.begin_array().map_err(anyhow::Error::msg)?;
        encoder.encode(&self.dst).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.src).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.msg_type).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.msg_id).map_err(anyhow::Error::msg)?;
        Ok(())
    }
}

#[derive(Debug, Clone)]
pub enum Ps4PropIdx {
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

#[derive(Debug, Clone)]

pub struct Ps4Event {
    pub event_type: u8,
    pub left_buttons: u8,
    pub left_axis_x: i16,
    pub left_axis_y: i16,
    pub right_axis_x: i16,
    pub right_axis_y: i16,
    pub right_buttons: u8,
    pub misc_buttons: u8,
}

impl Ps4Event {
    pub fn new() -> Ps4Event {
        Ps4Event {
            event_type: 0,
            left_buttons: 0,
            left_axis_x: 0,
            left_axis_y: 0,
            right_axis_x: 0,
            right_axis_y: 0,
            right_buttons: 0,
            misc_buttons: 0,
        }
    }
    pub fn from_decoder(decoder: &mut Decoder) -> Result<Ps4Event> {
        let _length = decoder.array().map_err(anyhow::Error::msg)?;
        if _length.is_some() {
            return Err(anyhow::Error::msg("Invalid array length"));
        }
        let event_type: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        let left_buttons: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        let left_axis_x: i16 = decoder.decode().map_err(anyhow::Error::msg)?;
        let left_axis_y: i16 = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_axis_x: i16 = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_axis_y: i16 = decoder.decode().map_err(anyhow::Error::msg)?;
        let right_buttons: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        let misc_buttons: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        Ok(Ps4Event {
            event_type,
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

pub const PS4_ID: u32 = fnv("ps4");

#[derive(Debug, Clone)]
pub struct Ps4Cmd {
    pub red_led: u8,
    pub green_led: u8,
    pub blue_led: u8,
    pub rumble: u8,
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

#[derive(Debug, Clone,PartialEq)]
pub struct MotorEvent {
    frame: u16,
    pub cmd1: i16,
    pub cmd2: i16,
    pub speed_right: i16,
    pub speed_left: i16,
    pub battery_voltage: i16,
    pub board_temperature: i16,
    pub cmd_led: u16,
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

    pub fn encode(&self,encoder: &mut Encoder<Vec<u8>>) -> Result<()> {
        encoder.begin_array().map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd1).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd2).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.speed_right).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.speed_left).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.battery_voltage).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.board_temperature).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd_led).map_err(anyhow::Error::msg)?;
        encoder.end().map_err(anyhow::Error::msg)?;
        Ok(())
    }
}
pub const fn fnv(s: &str) -> u32 {
    fnv1a_hash_32(s.as_bytes(), None)
}
