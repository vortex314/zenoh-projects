#[cfg(feature = "esp32")]
use alloc::{collections::VecDeque, fmt::format, string::String, vec::Vec};
use byte::Error;

#[cfg(feature = "linux")]
use std::{collections::VecDeque, fmt::format, string::String, vec::Vec};

use cobs::CobsDecoder;
use crc::Crc;
use crc::CRC_16_IBM_SDLC;
use log::{debug, info};

use minicbor::bytes::ByteVec;
use minicbor::encode::Write;

use byte::TryRead;
use byte::TryWrite;
use minicbor::bytes::ByteArray;

use bitfield::{bitfield_bitrange, bitfield_fields};

#[derive(Debug, Clone)]
pub struct Flags(pub u8);
bitfield_bitrange! {struct Flags(u8)}

 impl Flags {
    bitfield_fields! {
      u8;
      pub dup, set_dup: 7;
      pub qos, set_qos: 6, 5;
      pub retain, set_retain: 4;
      pub will, set_will: 3;
      pub clean_session, set_clean_session: 2;
      pub topic_id_type, set_topic_id_type: 1, 0;
    }
}

#[derive(Debug, Clone,PartialEq)]
pub enum ReturnCode {
    Accepted,
    Rejected,
    Congestion,
    InvalidTopicId,
    NotSupported,
}

#[derive(Debug, Clone)]
pub enum MqttSnMessage {
    Connect {
        flags: Flags,
        duration: u16,
        client_id: String,
    },
    ConnAck {
        return_code: ReturnCode,
    },
    WillTopicReq,
    WillTopic {
        flags: Flags,
        topic: String,
    },
    WillMsgReq,
    WillMsg {
        message: Vec<u8>,
    },
    Register {
        topic_id: u16,
        msg_id: u16,
        topic_name: String,
    },
    RegAck {
        topic_id: u16,
        msg_id: u16,
        return_code: ReturnCode,
    },
    Publish {
        flags: Flags,
        topic_id: u16,
        msg_id: u16,
        data: Vec<u8>,
    },
    PubAck {
        topic_id: u16,
        msg_id: u16,
        return_code: ReturnCode,
    },
    PubRec {
        msg_id: u16,
    },
    PubRel {
        msg_id: u16,
    },

    PubComp {
        msg_id: u16,
    },
    Subscribe {
        flags: Flags,
        msg_id: u16,
        topic: Option<String>,
        topic_id: Option<u16>,
        qos: u8,
    },
    SubAck {
        flags: Flags,
        topic_id: u16,
        msg_id: u16,
        return_code: ReturnCode,
    },
    Unsubscribe {
        flags: Flags,
        msg_id: u16,
        topic: Option<String>,
        topic_id: Option<u16>,
        qos: u8,
    },
    UnsubAck {
        msg_id: u16,
        return_code: ReturnCode,
    },
    PingReq {
        client_id: Option<String>,
    },
    PingResp,
    Disconnect {
        duration: u16,
    },
    WillTopicUpd {
        flags: Flags,
        topic: String,
    },
    WillMsgUpd {
        message: Vec<u8>,
    },
    WillTopicResp {
        return_code: ReturnCode,
    },
    WillMsgResp {
        return_code: ReturnCode,
    },
}

struct MqttSnEncoder {
    bytes: Vec<u8>,
}

impl MqttSnEncoder {
    fn new() -> Self {
        MqttSnEncoder { bytes: Vec::new() }
    }
    fn encode_u8(&mut self, v: u8) {
        self.bytes.push(v);
    }
    fn encode_u16(&mut self, v: u16) {
        self.encode_u8(v as u8);
        self.encode_u8((v >> 8) as u8);
    }
    fn encode_string(&mut self, v: &str) {
        self.encode_u8(v.len() as u8);
        self.bytes.extend(v.as_bytes());
    }
    fn encode_bytes(&mut self, v: &[u8]) {
        self.encode_u8(v.len() as u8);
        self.bytes.extend(v);
    }
    fn encode_flags(&mut self, v: &Flags) {
        self.encode_u8(v.0);
    }
    fn encode_return_code(&mut self, v: &ReturnCode) {
        self.encode_u8(match v {
            ReturnCode::Accepted => 0x00,
            ReturnCode::Rejected => 0x01,
            ReturnCode::Congestion => 0x02,
            ReturnCode::InvalidTopicId => 0x03,
            ReturnCode::NotSupported => 0x04,
        });
    }
    fn len(&self) -> usize {
        self.bytes.len()
    }
}

impl TryWrite for MqttSnMessage {
    fn try_write(self, bytes: &mut [u8], _ctx: ()) -> byte::Result<usize> {
        encode(&self, bytes, ())
    }
}

fn encode(msg: &MqttSnMessage, bytes: &mut [u8], _ctx: ()) -> byte::Result<usize> {
    let mut encoder = MqttSnEncoder::new();
    match msg {
        MqttSnMessage::Connect {
            flags,
            duration,
            client_id,
        } => {
            encoder.encode_u8(0x04);
            encoder.encode_flags(flags);
            encoder.encode_u16(*duration);
            encoder.encode_string(&client_id);
        }
        MqttSnMessage::ConnAck { return_code } => {
            encoder.encode_u8(0x05);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::WillTopicReq => {
            encoder.encode_u8(0x06);
        }
        MqttSnMessage::WillTopic { flags, topic } => {
            encoder.encode_u8(0x07);
            encoder.encode_flags(flags);
            encoder.encode_string(&topic);
        }
        MqttSnMessage::WillMsgReq => {
            encoder.encode_u8(0x08);
        }
        MqttSnMessage::WillMsg { message } => {
            encoder.encode_u8(0x09);
            encoder.encode_bytes(&message);
        }
        MqttSnMessage::Register {
            topic_id,
            msg_id,
            topic_name,
        } => {
            encoder.encode_u8(0x0a);
            encoder.encode_u16(*topic_id);
            encoder.encode_u16(*msg_id);
            encoder.encode_string(&topic_name);
        }
        MqttSnMessage::RegAck {
            topic_id,
            msg_id,
            return_code,
        } => {
            encoder.encode_u8(0x0b);
            encoder.encode_u16(*topic_id);
            encoder.encode_u16(*msg_id);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::Publish {
            flags,
            topic_id,
            msg_id,
            data,
        } => {
            encoder.encode_u8(0x0c);
            encoder.encode_flags(flags);
            encoder.encode_u16(*topic_id);
            encoder.encode_u16(*msg_id);
            encoder.encode_bytes(&data);
        }
        MqttSnMessage::PubAck {
            topic_id,
            msg_id,
            return_code,
        } => {
            encoder.encode_u8(0x0d);
            encoder.encode_u16(*topic_id);
            encoder.encode_u16(*msg_id);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::PubRec { msg_id } => {
            encoder.encode_u8(0x0e);
            encoder.encode_u16(*msg_id);
        }
        MqttSnMessage::PubRel { msg_id } => {
            encoder.encode_u8(0x0f);
            encoder.encode_u16(*msg_id);
        }
        MqttSnMessage::PubComp { msg_id } => {
            encoder.encode_u8(0x10);
            encoder.encode_u16(*msg_id);
        }
        MqttSnMessage::Subscribe {
            flags,
            msg_id,
            topic,
            topic_id,
            qos,
        } => {
            encoder.encode_u8(0x12);
            encoder.encode_flags(flags);
            encoder.encode_u16(*msg_id);
            if let Some(topic) = topic {
                encoder.encode_string(&topic);
            } else {
                encoder.encode_u16(topic_id.unwrap());
            }
            encoder.encode_u8(*qos);
        }
        MqttSnMessage::SubAck {
            flags,
            topic_id,
            msg_id,
            return_code,
        } => {
            encoder.encode_u8(0x13);
            encoder.encode_flags(flags);
            encoder.encode_u16(*topic_id);
            encoder.encode_u16(*msg_id);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::Unsubscribe {
            flags,
            msg_id,
            topic,
            topic_id,
            qos,
        } => {
            encoder.encode_u8(0x14);
            encoder.encode_flags(flags);
            encoder.encode_u16(*msg_id);
            if let Some(topic) = topic {
                encoder.encode_string(&topic);
            } else {
                encoder.encode_u16(topic_id.unwrap());
            }
            encoder.encode_u8(*qos);
        }
        MqttSnMessage::UnsubAck {
            msg_id,
            return_code,
        } => {
            encoder.encode_u8(0x15);
            encoder.encode_u16(*msg_id);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::PingReq { client_id } => {
            encoder.encode_u8(0x16);
            if let Some(client_id) = client_id {
                encoder.encode_string(&client_id);
            }
        }
        MqttSnMessage::PingResp => {
            encoder.encode_u8(0x17);
        }
        MqttSnMessage::Disconnect { duration } => {
            encoder.encode_u8(0x18);
            encoder.encode_u16(*duration);
        }
        MqttSnMessage::WillTopicUpd { flags, topic } => {
            encoder.encode_u8(0x1a);
            encoder.encode_flags(flags);
            encoder.encode_string(&topic);
        }
        MqttSnMessage::WillMsgUpd { message } => {
            encoder.encode_u8(0x1b);
            encoder.encode_bytes(&message);
        }
        MqttSnMessage::WillTopicResp { return_code } => {
            encoder.encode_u8(0x1c);
            encoder.encode_return_code(return_code);
        }
        MqttSnMessage::WillMsgResp { return_code } => {
            encoder.encode_u8(0x1d);
            encoder.encode_return_code(return_code);
        }
    }
    let length = encoder.len();
    bytes[0] = 0x01;
    bytes[1] = length as u8;
    bytes[2..length + 2].copy_from_slice(&encoder.bytes);
    Ok(length)
}

struct Decoder {
    bytes: Vec<u8>,
    pos: usize,
}

impl Decoder {
    fn new(bytes: &[u8]) -> Self {
        Decoder {
            bytes: bytes.to_vec(),
            pos: 0,
        }
    }
    fn u8(&mut self) -> byte::Result<u8> {
        let v = self.bytes[self.pos];
        self.pos += 1;
        Ok(v)
    }
    fn u16(&mut self) -> byte::Result<u16> {
        let v = u16::from_le_bytes([self.bytes[self.pos], self.bytes[self.pos + 1]]);
        self.pos += 2;
        Ok(v)
    }
    fn string(&mut self) -> byte::Result<String> {
        let len = self.u8()? as usize;
        let v = String::from_utf8(self.bytes[self.pos..self.pos + len].to_vec()).unwrap();
        self.pos += len;
        Ok(v)
    }
    fn bytes(&mut self) -> byte::Result<Vec<u8>> {
        let len = self.u8()? as usize;
        let v = self.bytes[self.pos..self.pos + len].to_vec();
        self.pos += len;
        Ok(v)
    }
    fn return_code(&mut self) -> byte::Result<ReturnCode> {
        let v = match self.u8()? {
            0x00 => ReturnCode::Accepted,
            0x01 => ReturnCode::Rejected,
            0x02 => ReturnCode::Congestion,
            0x03 => ReturnCode::InvalidTopicId,
            0x04 => ReturnCode::NotSupported,
            _ => ReturnCode::Accepted,
        };
        Ok(v)
    }
    fn flags(&mut self) -> byte::Result<Flags> {
        let v = Flags(self.u8()?);
        Ok(v)
    }
    fn offset(&self) -> usize {
        self.pos
    }
}

impl<'a> TryRead<'a> for MqttSnMessage {
    fn try_read(bytes: &'a [u8], _ctx: ()) -> byte::Result<(Self, usize)> {
        let mut decoder = Decoder::new(bytes);
        let msg_type = decoder.u8()?;
        match msg_type {
            0x04 => {
                let flags = Flags(decoder.u8()?);
                let duration = decoder.u16()?;
                let client_id = decoder.string()?;
                Ok((
                    MqttSnMessage::Connect {
                        flags,
                        duration,
                        client_id,
                    },
                    decoder.offset(),
                ))
            }
            0x05 => {
                let return_code = decoder.return_code()?;
                Ok((MqttSnMessage::ConnAck { return_code },decoder.offset()))
            }
            0x06 => Ok((MqttSnMessage::WillTopicReq,decoder.offset()))  ,
            0x07 => {
                let flags = Flags(decoder.u8()?);
                let topic = decoder.string()?;
                Ok((MqttSnMessage::WillTopic { flags, topic },decoder.offset()))
            }
            0x08 => Ok((MqttSnMessage::WillMsgReq,decoder.offset())) ,
            0x09 => {
                let message = decoder.bytes()?;
                Ok((MqttSnMessage::WillMsg { message },decoder.offset()))
            }
            0x0a => {
                let topic_id = decoder.u16()?;
                let msg_id = decoder.u16()?;
                let topic_name = decoder.string()?;
                Ok((MqttSnMessage::Register {
                    topic_id,
                    msg_id,
                    topic_name,
                },decoder.offset()))
            }
            0x0b => {
                let topic_id = decoder.u16()?;
                let msg_id = decoder.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::RegAck {
                    topic_id,
                    msg_id,
                    return_code,
                },decoder.offset()))
            }
            0x0c => {
                let flags = Flags(decoder.u8()?);
                let topic_id = decoder.u16()?;
                let msg_id = decoder.u16()?;
                let data = decoder.bytes()?;
                Ok((MqttSnMessage::Publish {
                    flags,
                    topic_id,
                    msg_id,
                    data,
                },decoder.offset()))
            }
            0x0d => {
                let topic_id = decoder.u16()?;
                let msg_id = decoder.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::PubAck {
                    topic_id,
                    msg_id,
                    return_code,
                },decoder.offset()))
            }
            0x0e => {
                let msg_id = decoder.u16()?;
                Ok((MqttSnMessage::PubRec { msg_id },decoder.offset())) 
            }
            0x0f => {
                let msg_id = decoder.u16()?;
                Ok((MqttSnMessage::PubRel { msg_id },decoder.offset()))  
            }
            0x10 => {
                let msg_id = decoder.u16()?;
                Ok((MqttSnMessage::PubComp { msg_id },decoder.offset()))
            }
            0x12 => {
                let flags = Flags(decoder.u8()?);
                let msg_id = decoder.u16()?;
                let topic = decoder.string()?;
                let topic_id = decoder.u16()?;
                let qos = decoder.u8()?;
                Ok((MqttSnMessage::Subscribe {
                    flags,
                    msg_id,
                    topic: Some(topic),
                    topic_id: Some(topic_id),
                    qos,
                },decoder.offset()))
            }
            0x13 => {
                let flags = Flags(decoder.u8()?);
                let topic_id = decoder.u16()?;
                let msg_id = decoder.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::SubAck {
                    flags,
                    topic_id,
                    msg_id,
                    return_code,
                },decoder.offset()))
            }
            0x14 => {
                let flags = Flags(decoder.u8()?);
                let msg_id = decoder.u16()?;
                let topic = decoder.string()?;
                let topic_id = decoder.u16()?;
                let qos = decoder.u8()?;
                Ok((MqttSnMessage::Unsubscribe {
                    flags,
                    msg_id,
                    topic: Some(topic),
                    topic_id: Some(topic_id),
                    qos,
                },decoder.offset()))
            }
            0x15 => {
                let msg_id = decoder.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::UnsubAck {
                    msg_id,
                    return_code,
                },decoder.offset()))
            }
            0x16 => {
                let client_id = decoder.string()?;
                Ok((MqttSnMessage::PingReq {
                    client_id: Some(client_id),
                },decoder.offset()))
            }
            0x17 => Ok((MqttSnMessage::PingResp,decoder.offset())),
            0x18 => {
                let duration = decoder.u16()?;
                Ok((MqttSnMessage::Disconnect { duration },decoder.offset()))
            }
            0x1a => {
                let flags = Flags(decoder.u8()?);
                let topic = decoder.string()?;
                Ok((MqttSnMessage::WillTopicUpd { flags, topic },decoder.offset()))
            }
            0x1b => {
                let message = decoder.bytes()?;
                Ok((MqttSnMessage::WillMsgUpd { message },decoder.offset()))
            }
            0x1c => {
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::WillTopicResp { return_code },decoder.offset()))
            }
            0x1d => {
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::WillMsgResp { return_code },decoder.offset()))
            }
            _ => Err(Error::BadInput { err: "parsing failed " }),
        }
    }
}
/*
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
        return_code: ReturnCode,
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
        return_code: ReturnCode,
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
        return_code: ReturnCode,
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
        return_code: ReturnCode,
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
        return_code: ReturnCode,
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

#[derive(Encode, Decode, Debug, Clone)]
#[cbor(index_only)]
pub enum ReturnCode {
    #[n(0)]
    Accepted,
    #[n(1)]
    Rejected,
    #[n(2)]
    Congestion,
    #[n(3)]
    InvalidTopicId,
    #[n(4)]
    NotSupported,
}

    */
