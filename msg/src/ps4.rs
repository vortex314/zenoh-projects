
use minicbor::Decoder;
use anyhow::Result;
use crate::fnv;


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
