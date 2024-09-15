#![no_std]
#![allow(unused_imports)]
pub mod payload_json;
pub mod payload_cbor;
pub use payload_json::Json as Json;
pub use payload_cbor::Cbor as Cbor;
pub mod framer;
pub use framer::cobs_crc_frame as cobs_crc_frame;
pub use framer::cobs_crc_deframe as cobs_crc_deframe;
pub use framer::FrameExtractor as FrameExtractor;
pub mod msg;
pub use msg::MotorEvent as MotorEvent;
pub use msg::MotorCmd as MotorCmd;
pub use msg::Ps4Event as Ps4Event;
pub use msg::Ps4PropIdx as Ps4PropIdx;
pub use msg::EspNowHeader as EspNowHeader;
pub use msg::fnv as fnv;
pub use msg::START_FRAME as START_FRAME;
pub use msg::PS4_ID as PS4_ID;

pub use msg::MsgType as MsgType;
pub use msg::reply as reply;
pub use msg::request as request;


extern crate alloc;

use anyhow::Result;
use serde::de::DeserializeOwned;
use serde::Deserialize;
use serde::Serialize;

use alloc::vec::Vec;
use alloc::string::String;


pub trait PayloadCodec {
    fn as_f64(v:& Vec<u8>) -> Result<f64> ;
    fn to_string(v:&Vec<u8>) -> String ;// => display as string
    fn decode<T>(v:&Vec<u8>) -> Result<T> where T : DeserializeOwned;
    fn encode<T>(v:&T) -> Vec<u8> where T : Serialize;
}

pub enum Codec {
    Json,
    Cbor,
}


pub fn payload_display(payload: &Vec<u8>, codec: Codec) -> String {
    match codec {
        Codec::Json => Json::to_string(payload),
        Codec::Cbor => Cbor::to_string(payload),
    }
}

pub fn payload_as_f64(payload: &Vec<u8>, codec: Codec) -> Result<f64> {
    match codec {
        Codec::Json => Json::as_f64(payload),
        Codec::Cbor => Cbor::as_f64(payload),
    }
}

pub fn payload_decode<T>(payload: &Vec<u8>, codec: Codec) -> Result<T>
where
    T: DeserializeOwned,
{
    match codec {
        Codec::Json => Json::decode(payload),
        Codec::Cbor => Cbor::decode(payload),
    }
}

pub fn payload_encode<T>(payload: &T, codec: Codec) -> Vec<u8>
where
    T: Serialize,
{
    match codec {
        Codec::Json => Json::encode(payload),
        Codec::Cbor => Cbor::encode(payload),
    }
}
