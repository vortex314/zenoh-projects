#![no_std]
extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;
use anyhow::Result;
use const_fnv1a_hash::fnv1a_hash_32;
use minicbor::data::Type;
use minicbor::{Decode, Encode};

pub mod framer;
pub use framer::encode_frame;
pub use framer::FrameExtractor;

pub mod esp_now;
pub use esp_now::EspNowHeader;
pub use esp_now::EspNowMessage;
pub use esp_now::Log;
pub use esp_now::LogLevel;

pub mod pubsub;
pub use pubsub::PubSubCmd;
pub use pubsub::PubSubEvent;

pub mod cbor;
pub mod json;
#[cfg(feature = "cbor")]
pub use cbor::as_f64 as payload_as_f64;
#[cfg(feature = "cbor")]
pub use cbor::decode as payload_decode;
#[cfg(feature = "cbor")]
pub use cbor::encode as payload_encode;
#[cfg(feature = "cbor")]
pub use cbor::to_string as payload_display;
#[cfg(feature = "json")]
pub use json::as_f64 as payload_as_f64;
#[cfg(feature = "json")]
pub use json::decode as payload_decode;
#[cfg(feature = "json")]
pub use json::encode as payload_encode;
#[cfg(feature = "json")]
pub use json::to_string as payload_display;

pub mod hb;
pub mod ps4;

pub const fn fnv(s: &str) -> u32 {
    fnv1a_hash_32(s.as_bytes(), None)
}

pub type ObjectId = u32;

#[derive(Encode, Decode, Clone)]
#[cbor(array)]
pub struct MsgHeader {
    #[n(0)]
    pub dst: Option<ObjectId>,
    #[n(1)]
    pub src: Option<ObjectId>,
    #[n(2)]
    pub msg_type: MsgType,
}

impl Default for MsgHeader {
    fn default() -> Self {
        MsgHeader {
            dst: None,
            src: None,
            msg_type: MsgType::Alive,
        }
    }
}
#[derive(PartialEq)]
#[derive(Encode, Decode, Clone)]
#[cbor(index_only)]
pub enum MsgType {
    #[n(0)]
    Alive,
    #[n(1)]
    Pub,
    #[n(2)]
    Sub,
    #[n(3)]
    Info,
}

/*#[derive(Encode, Decode, Clone)]
#[cbor(array)]
struct PubMsg {
    #[n(0)]
    topic: String,
    #[n(1)]
    payload: Vec<u8>,
}*/

pub fn reply(msg_type: MsgType) -> u8 {
    msg_type as u8 | 0x80
}

pub fn request(msg_type: MsgType) -> u8 {
    msg_type as u8
}

pub type PropertyId = i8;

#[repr(i8)]
pub enum MetaPropertyId {
    RetCode = -1,
    Qos = -2,
    MsgId = -3,
}
#[derive(Encode, Decode, Clone)]
#[cbor(index_only)]
#[repr(i8)]
pub enum InfoPropertyId {
    #[n(0)]
    PropId = 0,
    #[n(1)]
    Name,
    #[n(2)]
    Desc,
    #[n(3)]
    Type,
    #[n(4)]
    Mode,
}

#[derive(Encode, Decode, Clone)]
#[cbor(index_only)]
#[repr(i8)]
pub enum PropType {
    #[n(0)]
    UINT = 0,
    #[n(1)]
    INT = 1,
    #[n(2)]
    STR = 2,
    #[n(3)]
    BYTES = 3,
    #[n(4)]
    FLOAT = 4,
}

#[derive(Encode, Decode, Clone)]
#[cbor(index_only)]
pub enum PropMode {
    #[n(0)]
    Read = 0,
    #[n(1)]
    Write = 1,
}

#[derive(Encode, Decode, Clone)]
#[cbor(map)]
pub struct InfoMsg {
    #[n(0)]
    pub id: PropertyId,
    #[n(1)]
    pub name: String,
    #[n(2)]
    pub desc: String,
    #[n(3)]
    pub prop_type: Option<PropType>,
    #[n(4)]
    pub prop_mode: Option<PropMode>,
}

pub struct MsgDecoder<'a> {
    decoder: minicbor::Decoder<'a>,
}

impl<'a> MsgDecoder<'a> {
    pub fn new(data: &'a Vec<u8>) -> Self {
        MsgDecoder {
            decoder: minicbor::Decoder::new(data),
        }
    }

    pub fn decode<T>(&mut self) -> Result<T>
    where
        T: Decode<'a, ()>,
    {
        self.decoder.decode().map_err(anyhow::Error::msg)
    }

    pub fn decode_header(&mut self) -> Result<MsgHeader> {
        let _x = self.begin_array()?;
        let dst = self.decode::<Option<ObjectId>>()?;
        let src = self.decode::<Option<ObjectId>>()?;
        let msg_type = self.decode::<MsgType>()?;
        Ok(MsgHeader { dst, src, msg_type })
    }

    pub fn begin_map(&mut self) -> Result<Option<u64>> {
        self.decoder.map().map_err(anyhow::Error::msg)
    }

    pub fn begin_array(&mut self) -> Result<Option<u64>> {
        self.decoder.array().map_err(anyhow::Error::msg)
    }

    pub fn decode_end(&mut self) -> Result<()> {
        Ok(())
    }

    pub fn peek_next_type(&mut self) -> Result<Type> {
        self.decoder.datatype().map_err(anyhow::Error::msg)
    }

    pub fn position(&self) -> usize {
        self.decoder.position()
    }

    pub fn set_position(&mut self, pos: usize) {
        self.decoder.set_position(pos);
    }

    pub fn rewind(&mut self) {
        self.decoder.set_position(0);
    }

    pub fn skip_next(&mut self) -> Result<()> {
        self.decoder.skip().map_err(anyhow::Error::msg)
    }

    pub fn find_in_map(&mut self,search_key : i8) -> Result<()>{
        let _ = self.decoder.set_position(0);
        let _ = self.decode_header()?;
        let _ = self.begin_map()?;
        loop {
            let key = self.decode::<i8>()?;
            if key == search_key  {
                break;
            }
            self.skip_next()?;
        }
        Ok(())
    }
}
