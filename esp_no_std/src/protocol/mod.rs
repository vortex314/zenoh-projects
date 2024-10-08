use core::cell::RefCell;
use core::ops::Shr;

use alloc::collections::VecDeque;
use alloc::fmt::format;
use alloc::rc::Rc;
use alloc::string::String;
use alloc::string::ToString;
use cobs::CobsDecoder;
use crc::Crc;
use crc::CRC_16_IBM_SDLC;
use log::{debug, info};

//#[cfg(feature = "embassy")]
use embassy_sync::channel::{Channel, DynamicReceiver, DynamicSender};

extern crate alloc;
use alloc::format;
use alloc::vec;
use alloc::vec::Vec;

pub const MTU_SIZE: usize = 1023;
const MAX_FRAME_SIZE: usize = MTU_SIZE + 2;
/*
https://github.com/ty4tw/MQTT-SN

Request - seq nr - destination_topic_id - source_topic_id - payload - crc16
Response - seq nr - source_topic_id - payload - crc16
Publish - source_id - payload
PubAck - source_id - return_code
Subscribe - source_pattern
SubAck - source_id - return_code
Register source_id - source_topic_name
RegAck - source_id - return_code

destination_topic_id -
0 -Publish
1 - PubAck
2 - Subscribe
3 - SubAck
4 - Register
5 - RegAck
6 - Unsubscribe
7 - UnsubAck
8 - PingReq
9 - PingResp
10 - Disconnect
11 - WillTopicUpd
12 - WillMsgUpd
13 - Log
14 - Connect
15 - ConnAck
16 - WillTopicReq
17 - WillTopic
18 - WillMsgReq
19 - WillMsg

Server
- waitConnect - Connect - ConnAck - Connected
-

- waitSubscribe - Subscribe - SubAck - Subscribed
- waitPublish - Publish - PubAck - Published


*/
use minicbor::bytes::ByteVec;
use minicbor::decode::Decode;
use minicbor::encode::Encode;
use minicbor::encode::Encoder;
use minicbor::encode::Write;

pub mod msg;

use byte::TryRead;
use byte::TryWrite;
use msg::ProxyMessage  ;
// use mqtt_sn::defs::Message as ProxyMessage;



pub struct MessageDecoder {
    buffer: Vec<u8>,
}

impl MessageDecoder {
    pub fn new() -> Self {
        Self { buffer: Vec::new() }
    }

    pub fn decode(&mut self, data: &[u8]) -> Vec<ProxyMessage> {
        let mut messages_found = Vec::new();
        for byte in data {
            self.buffer.push(*byte);
            if *byte == 0 {
                // decode cobs from frame
                let msg = decode_frame(&self.buffer);
                msg.into_iter().for_each(|m| {
                    messages_found.push(m);
                });
                self.buffer.clear();
            }
        }
        messages_found
    }

    pub fn to_str(&self) -> String {
        match String::from_utf8(self.buffer.clone()) {
            Ok(s) => s,
            Err(_) => String::from(""),
        }
    }
}



pub fn encode_frame(msg: ProxyMessage) -> Result<Vec<u8>, String> {

    let writer = msg::VecWriter::new();
    let mut encoder = minicbor::encode::Encoder::new(writer);
    let res = msg.encode(&mut encoder, &mut());
    if let Err(e) = res {
        return Err(format!("CBOR encoding error : {:?}", e));
    }
    let mut bytes = encoder.into_writer().to_inner();

    debug!("Encoded MQTT-SN : {:02X?}", bytes);

    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&bytes);
    debug!("CRC : {:04X}", crc);
    bytes.push((crc & 0xFF) as u8);
    bytes.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_buffer = [0; MTU_SIZE];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&bytes);
    if let Err(e) = _res {
        return Err(format!("COBS encoding error : {:?}", e));
    }
    let size = cobs_encoder.finalize().unwrap();
    // prefix with delimiter
    let mut res_vec = Vec::new();
    res_vec.push(0x00 as u8);
    res_vec.extend_from_slice(&cobs_buffer[0..size + 1]);
//    res_vec.push(0x00 as u8);
    res_vec.push('\r' as u8);
    res_vec.push('\n' as u8);
    Ok(res_vec)
    
}

pub fn decode_frame(queue: &Vec<u8>) -> Result<ProxyMessage, String> {
   let mut output = [0; MTU_SIZE + 2];
    let mut decoder = CobsDecoder::new(&mut output);
    let res = decoder.push(&queue);

    drop(decoder);

    match res {
        Ok(None) => {
            return Err("no correct COBS found".to_string());
        }
        Ok(Some((output_size, _input_size))) => {
            if output_size < 2 {
                return Err("no correct COBS found".to_string());
            }
            let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
            let crc = crc16.checksum(&output[0..(output_size - 2)]);
            let crc_received =
                (output[output_size - 1] as u16) << 8 | output[output_size - 2] as u16;
            if crc != crc_received {
                return Err(format!("CRC error : {:04X} != {:04X}", crc, crc_received));
            }
            let mut d = minicbor::decode::Decoder::new(&output[0..(output_size - 2)]);
            let msg_res = ProxyMessage::decode(&mut d, &mut());
            match msg_res {
                Ok(m) => {
                    return Ok(m);
                }
                Err(e) => {
                    return Err(format!("CBOR decoding error : {:?}", e));
                }
            }
        }
        Err(j) => Err(format!("COBS decoding error : {:?}", j)),
    }
}
