use alloc::collections::VecDeque;
use alloc::string::String;
use alloc::string::ToString;
use cobs::CobsDecoder;
use crc::Crc;
use crc::CRC_16_IBM_SDLC;
use log::info;
use minicbor::{encode::write::EndOfSlice, Decode, Decoder, Encode, Encoder};

extern crate alloc;
use alloc::vec;
use alloc::vec::Vec;

const MTU_SIZE: usize = 1023;
const MAX_FRAME_SIZE: usize = MTU_SIZE + 2;
/* 
https://github.com/ty4tw/MQTT-SN
*/
#[derive(Encode, Decode, Debug)]
#[cbor(array)]
pub enum ProxyMessage {
    #[n(0)]
    Connect { #[n(0)] protocol_id: u8, #[n(1)] duration: u16, #[n(2)] client_id: String,},
    #[n(1)]
    ConnAck { #[n(0)] return_code: u8,},
    #[n(2)]
    WillTopicReq,
    #[n(3)]
    WillTopic { #[n(0)] topic: String,},
    #[n(4)]
    WillMsgReq,
    #[n(5)]
    WillMsg { #[n(0)] message: String,},
    #[n(6)]
    Register { #[n(0)] topic_id: u16, #[n(1)] topic_name: String,},
    #[n(7)]
    RegAck { #[n(0)] topic_id: u16, #[n(1)] return_code: u8,},
    #[n(8)]
    Publish { #[n(0)] topic_id: u16, #[n(1)] message: String,},
    #[n(9)]
    PubAck { #[n(0)] topic_id: u16, #[n(1)] return_code: u8,},
    #[n(10)]
    Subscribe { #[n(0)] topic_id: u16, #[n(1)] qos: u8,},
    #[n(11)]
    SubAck { #[n(0)] topic_id: u16, #[n(1)] return_code: u8,},
    #[n(12)]
    Unsubscribe { #[n(0)] topic_id: u16,},
    #[n(13)]
    UnsubAck { #[n(0)] topic_id: u16, #[n(1)] return_code: u8,},
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
    Log { #[n(0)] timestamp: u64, #[n(1)] message: String, #[n(2)] level: Option<LogLevel>, #[n(3)] component: Option<String>, #[n(4)] file: Option<String>, #[n(5)] line: Option<u32>,},


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
    type Error = EndOfSlice;
    // Required method
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.extend_from_slice(buf);
        Ok(())
    }
}

type Id = u16;

#[derive(Encode, Decode, Debug)]
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

#[derive(Encode, Decode, Debug)]
#[cbor(array)]

pub struct Log {
    #[n(0)]
    pub timestamp: u64,
    #[n(1)]
    pub message: String,
    #[n(2)]
    pub level: Option<LogLevel>,
    #[n(3)]
    pub component: Option<String>,
    #[n(4)]
    pub file: Option<String>,
    #[n(5)]
    pub line: Option<u32>,
}




impl Log {
    pub fn new(message: &str) -> Log {
        Log {
            timestamp: 0,
            message: message.to_string(),
            level: None,
            component: None,
            file: None,
            line: None,
        }
    }
}











pub fn make_frame(msg: ProxyMessage) -> Result<Vec<u8>, String> {
    let mut buffer = VecWriter::new();
    let mut encoder = Encoder::new(&mut buffer);
    let mut ctx = 1;
    let _ = msg.encode(&mut encoder, &mut ctx);
    info!("Encoded cbor : {:02X?}", buffer.to_bytes());
    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
    let crc = crc16.checksum(&buffer.to_bytes());
    info!("CRC : {:04X}", crc);
    buffer.push((crc & 0xFF) as u8);
    buffer.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_buffer = [0; 256];
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer.to_bytes()).unwrap();
    let size = cobs_encoder.finalize().unwrap();
    buffer.push(0);
    info!("COBS : {:02X?}", &cobs_buffer[0..(size + 1)]);
    Ok(cobs_buffer[0..size + 1].to_vec())
}

fn decode_frame(queue: &Vec<u8>) -> Option<ProxyMessage> {
    let mut output = [0; MTU_SIZE + 2];
    let mut decoder = CobsDecoder::new(&mut output);
    let res = decoder.push(&queue);

    drop(decoder);

    match res {
        Ok(None) => {
            return None;
        }
        Ok(Some((output_size, _input_size))) => {
            let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
            let crc = crc16.checksum(&output[0..(output_size - 2)]);
            let crc_received =
                (output[output_size - 1] as u16) << 8 | output[output_size - 2] as u16;
            if crc != crc_received {
                info!("CRC error : {:04X} != {:04X}", crc, crc_received);
                return None;
            }

            let mut cbor_decoder = minicbor::decode::Decoder::new(&output[0..(output_size - 2)]);
            let mut ctx = 1;
            let msg = ProxyMessage::decode(&mut cbor_decoder, &mut ctx).unwrap();
            return Some(msg);
        }
        Err(j) => {
            info!("COBS decoding error : {:?}", j);
            return None;
        }
    }
}

pub struct MessageDecoder {
    queue: Vec<u8>,
}

impl MessageDecoder {
    pub fn new() -> Self {
        Self { queue: Vec::new() }
    }

    pub fn decode(&mut self, data: &[u8]) -> Vec<ProxyMessage> {
        let mut messages_found = Vec::new();
        for byte in data {
            self.queue.push(*byte);
            if *byte == 0 {
                // decode cobs from frame
                let msg = decode_frame(&self.queue);
                msg.into_iter().for_each(|m| {
                    messages_found.push(m);
                });
                self.queue.clear();
            }
        }
        messages_found
    }
}


