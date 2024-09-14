#![no_std]
#![allow(unused_imports)]
pub mod payload_json;
pub mod payload_cbor;
pub use payload_json::Json as Json;
pub use payload_cbor::Cbor as Cbor;

extern crate alloc;

use anyhow::Result;
use serde::de::DeserializeOwned;
use serde::Deserialize;
use serde::Serialize;

use alloc::vec::Vec;
use alloc::string::String;


// =============== pubsub interface actor ===============    

#[derive(Clone, Debug)]
pub enum PubSubCmd {
    Publish { topic: String, payload : Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
    Connect { client_id: String },
    Disconnect,
}
#[derive(Clone, Debug)]
pub enum PubSubEvent {
    Publish { topic: String, payload: Vec<u8> },
    Connected,
    Disconnected,
}

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


// =============== pubsub interface actor ===============
/* 
type TopicId = u32;
struct TopicEndpoint<T> {
    name: String,
    id:TopicId,
    description : Option<String>,
    range : Option<(f64,f64)>,
    last_value : T,
}

// to topic
enum TopicReq {
    NameReq { id : TopicId  },
    DescReq { id : TopicId  },
    Set { id : TopicId, value : Vec<u8> },
}

// from topic
enum TopicResp {
    NameResp { name:String },
    DescResp { desc:Option<String> },
}*/