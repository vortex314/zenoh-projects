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

#[cfg(feature = "embassy")]
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
use msg::MqttSnMessage   ;
// use mqtt_sn::defs::Message as ProxyMessage;

/*


struct VecWriter {
    buffer: Vec<u8>,
}

impl VecWriter {
    fn new() -> Self {
        VecWriter { buffer: Vec::new() }
    }

    fn len(&self) -> usize {
        self.buffer.len()
    }

    fn to_bytes(&self) -> &[u8] {
        self.buffer.as_slice()
    }

    fn push(&mut self, data: u8) {
        self.buffer.push(data);
    }

    fn to_vec(&self) -> Vec<u8> {
        self.buffer.to_vec()
    }

    fn clear(&mut self) {
        self.buffer.clear();
    }
}
impl minicbor::encode::Write for VecWriter {
    type Error = String;
    // Required method
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.extend_from_slice(buf);
        Ok(())
    }
}


pub fn encode_frame(msg: ProxyMessage) -> Result<Vec<u8>, String> {
    let mut buffer = VecWriter::new();
    let mut encoder = Encoder::new(&mut buffer);
    let mut ctx = 1;
    let _ = msg.encode(&mut encoder, &mut ctx);
    debug!("Encoded cbor : {:02X?}", buffer.to_bytes());
    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&buffer.to_bytes());
    debug!("CRC : {:04X}", crc);
    buffer.push((crc & 0xFF) as u8);
    buffer.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_buffer = [0; MTU_SIZE];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer.to_bytes());
    if let Err(e) = _res {
        return Err(format!("COBS encoding error : {:?}", e));
    }
    let size = cobs_encoder.finalize().unwrap();
    // prefix with delimiter
    let mut res_vec = Vec::new();
    res_vec.push(0x00 as u8);
    res_vec.extend_from_slice(&cobs_buffer[0..size + 1]);
    res_vec.push(0x00 as u8);
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
            let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
            let crc = crc16.checksum(&output[0..(output_size - 2)]);
            let crc_received =
                (output[output_size - 1] as u16) << 8 | output[output_size - 2] as u16;
            if crc != crc_received {
                return Err(format!("CRC error : {:04X} != {:04X}", crc, crc_received));
            }

            let mut cbor_decoder = minicbor::decode::Decoder::new(&output[0..(output_size - 2)]);
            let mut ctx = 1;
            let msg_res = ProxyMessage::decode(&mut cbor_decoder, &mut ctx);
            if msg_res.is_err() {
                return Err(format!("CBOR decoding error : {:?}", msg_res));
            }
            return Ok(msg_res.unwrap());
        }
        Err(j) => Err(format!("COBS decoding error : {:?}", j)),
    }
}
*/

pub struct MessageDecoder {
    buffer: Vec<u8>,
}

impl MessageDecoder {
    pub fn new() -> Self {
        Self { buffer: Vec::new() }
    }

    pub fn decode(&mut self, data: &[u8]) -> Vec<MqttSnMessage> {
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

struct VecWriter {
    buffer: Vec<u8>,
}

impl VecWriter {
    fn new() -> Self {
        VecWriter { buffer: Vec::new() }
    }

    fn len(&self) -> usize {
        self.buffer.len()
    }

    fn to_bytes(&self) -> &[u8] {
        self.buffer.as_slice()
    }

    fn push(&mut self, data: u8) {
        self.buffer.push(data);
    }

    fn to_vec(&self) -> Vec<u8> {
        self.buffer.to_vec()
    }

    fn clear(&mut self) {
        self.buffer.clear();
    }
}
impl minicbor::encode::Write for VecWriter {
    type Error = String;
    // Required method
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.extend_from_slice(buf);
        Ok(())
    }
}

pub fn encode_frame(msg: MqttSnMessage) -> Result<Vec<u8>, String> {
    let buffer = &mut [0u8; 1000];
    let _res = msg.try_write(buffer, ());
    if let Err(e) = _res {
        return Err(format!("CBOR encoding error : {:?}", e));
    }
    let size = _res.unwrap();
    debug!("Encoded MQTT-SN : {:02X?}", &buffer[0..size]);

    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&buffer[0..size]);
    debug!("CRC : {:04X}", crc);
    buffer[size] = (crc & 0xFF) as u8;
    buffer[size + 1] = ((crc >> 8) & 0xFF) as u8;
    let mut cobs_buffer = [0; MTU_SIZE];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer[0..size + 2]);
    if let Err(e) = _res {
        return Err(format!("COBS encoding error : {:?}", e));
    }
    let size = cobs_encoder.finalize().unwrap();
    // prefix with delimiter
    let mut res_vec = Vec::new();
    res_vec.push(0x00 as u8);
    res_vec.extend_from_slice(&cobs_buffer[0..size + 1]);
    res_vec.push(0x00 as u8);
    res_vec.push('\r' as u8);
    res_vec.push('\n' as u8);
    Ok(res_vec)
    
}

pub fn decode_frame(queue: &Vec<u8>) -> Result<MqttSnMessage, String> {
   let mut output = [0; MTU_SIZE + 2];
    let mut decoder = CobsDecoder::new(&mut output);
    let res = decoder.push(&queue);

    drop(decoder);

    match res {
        Ok(None) => {
            return Err("no correct COBS found".to_string());
        }
        Ok(Some((output_size, _input_size))) => {
            let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
            let crc = crc16.checksum(&output[0..(output_size - 2)]);
            let crc_received =
                (output[output_size - 1] as u16) << 8 | output[output_size - 2] as u16;
            if crc != crc_received {
                return Err(format!("CRC error : {:04X} != {:04X}", crc, crc_received));
            }
            let msg_res = MqttSnMessage::try_read(&output[0..(output_size - 2)], ());
            match msg_res {
                Ok((m, _size)) => {
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
