

#[cfg(feature = "esp32")]
use alloc::{
    string::String,
    vec::Vec,
    collections::VecDeque,
    fmt::format,
};

#[cfg(feature = "linux")]
use std::{
    string::String,
    vec::Vec,
    collections::VecDeque,
    fmt::format,
};





use cobs::CobsDecoder;
use crc::Crc;
use crc::CRC_16_IBM_SDLC;
use log::{debug, info};

use minicbor::bytes::ByteVec;
use minicbor::encode::Write;

use minicbor::bytes::ByteArray;
use minicbor::{encode::write::EndOfSlice, Decode, Decoder, Encode, Encoder};

#[derive(Encode, Decode, Debug, Clone)]
#[cbor(array)]
pub enum ProxyMessage {
    #[n(0)]
    Connect {
        #[n(0)]
        protocol_id: u8,
        #[n(1)]
        duration: u16,
        #[n(2)]
        client_id: String,
    },
    #[n(1)]
    ConnAck {
        #[n(0)]
        return_code: u8,
    },
    #[n(2)]
    WillTopicReq,
    #[n(3)]
    WillTopic {
        #[n(0)]
        topic: String,
    },
    #[n(4)]
    WillMsgReq,
    #[n(5)]
    WillMsg {
        #[n(0)]
        message: Vec<u8>,
    },
    #[n(6)]
    Register {
        #[n(0)]
        topic_id: u16,
        #[n(1)]
        topic_name: String,
    },
    #[n(7)]
    RegAck {
        #[n(0)]
        topic_id: u16,
        #[n(1)]
        return_code: u8,
    },
    #[n(8)]
    Publish {
        #[n(0)]
        topic_id: u16,
        #[n(1)]
        message: Vec<u8>,
    },
    #[n(9)]
    PubAck {
        #[n(0)]
        topic_id: u16,
        #[n(1)]
        return_code: u8,
    },
    #[n(10)]
    Subscribe {
        #[n(0)]
        topic: String,
        #[n(1)]
        qos: u8,
    },
    #[n(11)]
    SubAck {
        #[n(0)]
        return_code: u8,
    },
    #[n(12)]
    Unsubscribe {
        #[n(0)]
        topic_id: u16,
    },
    #[n(13)]
    UnsubAck {
        #[n(0)]
        topic_id: u16,
        #[n(1)]
        return_code: u8,
    },
    #[n(14)]
    PingReq,
    #[n(15)]
    PingResp,
    #[n(16)]
    Disconnect,
    #[n(17)]
    WillTopicUpd,
    #[n(18)]
    WillMsgUpd,
    #[n(19)]
    Log {
        #[n(0)]
        timestamp: u64,
        #[n(1)]
        message: String,
        #[n(2)]
        level: Option<LogLevel>,
        #[n(3)]
        component: Option<String>,
        #[n(4)]
        file: Option<String>,
        #[n(5)]
        line: Option<u32>,
    },
}

#[derive(Encode, Decode, Debug, Clone)]
#[cbor(index_only)]
pub enum LogLevel {
    #[n(0)]
    Trace = 0,
    #[n(1)]
    Debug,
    #[n(2)]
    Info,
    #[n(3)]
    Warning,
    #[n(4)]
    Error,
}
