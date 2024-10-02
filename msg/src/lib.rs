#![no_std]
extern crate alloc;

use const_fnv1a_hash::fnv1a_hash_32;
use minicbor::{Decode,Encode};

mod framer;
pub use framer::encode_frame;
pub use framer::FrameExtractor;

pub mod esp_now;
pub use esp_now::EspNowHeader as EspNowHeader;
pub use esp_now::EspNowMessage as EspNowMessage;
pub use esp_now::Log;
pub use esp_now::LogLevel;

pub mod pubsub;
pub use pubsub::PubSubCmd as PubSubCmd;
pub use pubsub::PubSubEvent as PubSubEvent;

pub mod cbor;
pub mod json;
#[cfg(feature = "cbor")]
pub use cbor::decode as payload_decode;
#[cfg(feature = "cbor")]
pub use cbor::encode as payload_encode;
#[cfg(feature = "cbor")]
pub use cbor::to_string as payload_display;
#[cfg(feature = "cbor")]
pub use cbor::as_f64 as payload_as_f64;
#[cfg(feature = "json")]
pub use json::decode as payload_decode;
#[cfg(feature = "json")]
pub use json::encode as payload_encode;
#[cfg(feature = "json")]
pub use json::to_string as payload_display;
#[cfg(feature = "json")]
pub use json::as_f64 as payload_as_f64;

pub mod hb;
pub use hb as msg;

pub mod ps4;

pub const fn fnv(s: &str) -> u32 {
    fnv1a_hash_32(s.as_bytes(), None)
}


type ObjecId = u32;

#[derive(Encode, Decode,Clone)]
#[cbor(array)]
pub struct MsgHeader {
    #[n(0)]
    pub dst : Option<ObjecId>,
    #[n(1)]
    pub src : Option<ObjecId>,
    #[n(2)]
    pub msg_type : u8,
    #[n(3)]
    pub msg_id : Option<u16>,
}

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



