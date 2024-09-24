
use alloc::string::String;
use alloc::string::ToString;
use cobs::CobsDecoder;
use crc::Crc;
use crc::CRC_16_IBM_SDLC;
use log::debug;

extern crate alloc;
use alloc::format;
use alloc::vec::Vec;

use minicbor::encode::Encoder;
use minicbor::Encode;

// use mqtt_sn::defs::Message as ProxyMessage;
use anyhow::Error;
use anyhow::Result;

pub const MTU_SIZE: usize = 1023;
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


pub struct FrameExtractor {
    buffer: Vec<u8>,
}

impl FrameExtractor {
    pub fn new() -> Self {
        Self { buffer: Vec::new() }
    }

    pub fn decode(&mut self, data: &[u8]) -> Vec<Vec<u8>>
    {
        let mut messages_found = Vec::new();
        for byte in data {
            self.buffer.push(*byte);
            if *byte == 0 {
                // decode cobs from frame
                let msg = cobs_crc_deframe(&self.buffer);
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

pub fn encode_frame<T>(msg: &T) -> Result<Vec<u8>>
where
    T: Encode<()>,
{
    let  bytes = Vec::new();

    let mut encoder = Encoder::new(bytes);
    encoder
        .encode(msg)
        .map_err(|_| Error::msg("CBOR encode failed "))?;

    debug!("Encoded MQTT-SN : {:02X?}", encoder.writer());
    cobs_crc_frame(encoder.writer())
}

pub fn cobs_crc_frame(input: &Vec<u8>) -> Result<Vec<u8>>
{
    let mut bytes = Vec::new();
    bytes.extend_from_slice(input);
    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&bytes);
    debug!("CRC : {:04X}", crc);
    bytes.push((crc & 0xFF) as u8);
    bytes.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_buffer = [0; MTU_SIZE];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    cobs_encoder
        .push(&bytes)
        .map_err(|_| Error::msg("COBS encoding error "))?;
    let size = cobs_encoder
        .finalize()
        .map_err(|_| Error::msg("cobs finalize"))?;
    // prefix with delimiter
    let mut res_vec = Vec::new();
    res_vec.push(0x00 as u8);
    res_vec.extend_from_slice(&cobs_buffer[0..size + 1]);
    //    res_vec.push(0x00 as u8);
    res_vec.push('\r' as u8);
    res_vec.push('\n' as u8);
    Ok(res_vec)
}

pub fn cobs_crc_deframe(queue: &Vec<u8>) -> Result<Vec<u8>> {
    let mut output = [0; MTU_SIZE + 2];
    let mut decoder = CobsDecoder::new(&mut output);
    let res = decoder.push(&queue).map_err(|e| Error::msg(e))?;

    drop(decoder);

    match res {
        None => {
            return Err(Error::msg("no correct COBS found".to_string()));
        }
        Some((output_size, _input_size)) => {
            if output_size < 2 {
                return Err(Error::msg("no correct COBS found".to_string()));
            }
            let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
            let crc = crc16.checksum(&output[0..(output_size - 2)]);
            let crc_received =
                (output[output_size - 1] as u16) << 8 | output[output_size - 2] as u16;
            if crc != crc_received {
                return Err(Error::msg(format!(
                    "CRC error : {:04X} != {:04X}",
                    crc, crc_received
                )));
            }
            return Ok(output[0..(output_size - 2)].to_vec());
        }
    }
}
