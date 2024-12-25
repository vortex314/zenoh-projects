#![no_std]
extern crate alloc;

use core::fmt::Display;

use alloc::fmt;
use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;
use const_fnv1a_hash::fnv1a_hash_32;
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

#[derive(Encode, Decode, Default, Clone, Debug)]
#[cbor(map)]
pub struct MsgHeader {
    #[n(0)]
    pub dst: Option<ObjectId>,
    #[n(1)]
    pub src: Option<ObjectId>,
    #[n(2)]
    pub msg_type: MsgType,
    #[n(3)]
    pub return_code: Option<u32>,
    #[n(4)]
    pub msg_id: Option<u16>,
    #[n(5)]
    pub qos: Option<u8>,
}

#[derive(Encode, Decode, Default, Clone, Debug)]
#[cbor(map)]
pub struct Msg {
    #[n(0)]
    pub dst: Option<ObjectId>,
    #[n(1)]
    pub src: Option<ObjectId>,
    #[n(2)]
    pub msg_id: Option<u16>,
    #[n(3)]
    pub return_code: Option<u32>,
    #[n(4)]
    pub publish: Option<Vec<u8>>,
    #[n(5)]
    pub info_req: Option<Vec<u8>>,
    #[n(6)]
    pub info_prop: Option<InfoProp>,
    #[n(7)]
    pub info_topic: Option<InfoTopic>,
}

impl Msg {
    pub fn new() -> Self {
        Msg {
            dst: None,
            src: None,
            msg_id: None,
            return_code: None,
            publish: None,
            info_req: None,
            info_prop: None,
            info_topic: None,
        }
    }
    pub fn reply(&self) -> Self {
        let mut reply = Self::new();
        reply.dst = self.src;
        reply.src = self.dst;
        self.msg_id.map(|msg_id| reply.msg_id = Some(msg_id));
        reply
    }
    pub fn display(&self) -> String {
        let mut s = String::new();
        self.dst.map(|dst| s.push_str(&format!("dst: {},", dst)));
        self.src.map(|src| s.push_str(&format!("src: {},", src)));
        self.msg_id
            .map(|msg_id| s.push_str(&format!("msg_id: {},", msg_id)));
        self.return_code
            .map(|return_code| s.push_str(&format!("return_code: {},", return_code)));
        self.publish
            .as_ref()
            .map(|pub_req| s.push_str(&format!("pub_req:  {},", minicbor::display(pub_req))));
        self.info_req
            .as_ref()
            .map(|info_req| s.push_str(&format!("info_req: {},", minicbor::display(info_req))));
        self.info_prop
            .as_ref()
            .map(|info_reply| s.push_str(&format!("info_reply: {{ {} }},", info_reply)));
        self.info_topic
            .as_ref()
            .map(|info_topic| s.push_str(&format!("info_topic: {{ {} }},", info_topic)));
        s
    }
}

impl Display for Msg {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.display())
    }
}

#[derive(Encode, Decode, Clone, Debug, Default)]
#[cbor(map)]
pub struct InfoTopic {
    #[n(0)]
    pub name: Option<String>,
    #[n(2)]
    pub desc: Option<String>,
}

impl Display for InfoTopic {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "name: {}, desc: {}", self.name.as_ref().unwrap(), self.desc.as_ref().unwrap())
    }
}

/*

MsgType         Dst     Src    Payload     PubSUb
Publish         -       Id1     (payload)   src/topic1 (header)(payload)
Subscribe    Id1     -       src/topic1
Request             Id1     Id2                 dst/topic1 (header)(payload)
Reply
Info            -     Id1     (propId)    info/topic1
Info(req)       Id1   xxx    (propId)    info/topic1
Info(reply)     xxx   Id1    (propId)    dst/topic2
*/
/* 
impl MsgHeader {
    pub fn is_msg(&self, msg_type: MsgType, dst: Option<u32>, src: Option<u32>) -> bool {
        let mut matches = msg_type as u8 == self.msg_type as u8;
        dst.map(|dst| matches = matches && dst == self.dst.unwrap());
        src.map(|src| matches = matches && src == self.src.unwrap());
        matches
    }
}*/



#[derive(PartialEq, Encode, Decode, Clone, Default, Debug, Copy)]
#[cbor(index_only)]
pub enum MsgType {
    #[n(0)]
    #[default]
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
#[derive(Encode, Decode, Clone, Debug, Default)]
#[cbor(map)]
pub struct InfoProp {
    #[n(0)]
    pub id: PropertyId,
    #[n(1)]
    pub name: Option<String>,
    #[n(2)]
    pub desc: Option<String>,
    #[n(3)]
    pub prop_type: Option<PropType>,
    #[n(4)]
    pub prop_mode: Option<PropMode>,
}

impl InfoProp {
    pub fn new(id: PropertyId) -> Self {
        InfoProp {
            id,
            name: None,
            desc: None,
            prop_type: None,
            prop_mode: None,
        }
    }
    pub fn display(&self) -> String {
        let mut s = String::new();
        s.push_str(&format!("id: {},", self.id));
        self.name.as_ref()
            .map(|name| s.push_str(&format!("name:'{}',", name)));
        self.desc.as_ref()
            .map(|desc| s.push_str(&format!("desc:'{}',", desc)));
        self.prop_type
            .map(|prop_type| s.push_str(&format!("prop_type: {:?},", prop_type)));
        self.prop_mode
            .map(|prop_mode| s.push_str(&format!("prop_mode: {:?},", prop_mode)));
        s
    }
}

impl Display for InfoProp {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.display())
    }
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

#[derive(Encode, Decode, Clone, Debug, Copy)]
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
    #[n(5)]
    OBJECT,
    #[n(6)]
    ARRAY,
}

#[derive(Encode, Decode, Clone, Debug, Copy)]
#[cbor(index_only)]
#[repr(i8)]
pub enum PropMode {
    #[n(0)]
    Read = 0,
    #[n(1)]
    Write = 1,
    #[n(2)]
    ReadWrite = 2,
    #[n(3)]
    Notify = 3,
}

#[derive(Encode, Decode, Clone)]
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

#[derive(Encode, Decode, Clone)]
#[repr(u8)]
pub enum LogLvl {
    #[n(0)]
    TRACE = 0,
    #[n(1)]
    DEBUG = 1,
    #[n(2)]
    INFO = 2,
    #[n(3)]
    WARN = 3,
    #[n(4)]
    ERROR = 4,
    #[n(5)]
    FATAL = 5,
}

#[derive(Encode, Decode, Clone)]
#[cbor(map)]
pub struct LogMsg {
    #[n(0)]
    pub timestamp: Option<u64>, // timestamp
    #[n(1)]
    pub message: String,
    #[n(2)]
    pub object_id: Option<ObjectId>,
    #[n(3)]
    pub level: Option<LogLvl>,
    #[n(4)]
    pub component: Option<String>,
    #[n(5)]
    pub file: Option<String>,
    #[n(6)]
    pub line: Option<u32>,
    #[n(7)]
    pub device: Option<String>,
}

impl Default for LogMsg {
    fn default() -> Self {
        LogMsg {
            timestamp: None,
            message: String::new(),
            object_id: None,
            level: None,
            component: None,
            file: None,
            line: None,
            device: None,
        }
    }
}
/*
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

}*/
