#[cfg(feature = "esp32")]
use alloc::{collections::VecDeque, fmt::format, string::String, string::ToString, vec::Vec};
use minicbor::decode::Error as DecodeError;

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
use core::result::Result;
use minicbor::bytes::ByteArray;
use minicbor::encode::Error as EncodeError;
use minicbor::{Decode, Decoder, Encode, Encoder};

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

pub struct VecWriter {
    buffer: Vec<u8>,
}

impl VecWriter {
    pub fn new() -> Self {
        VecWriter { buffer: Vec::new() }
    }

    pub fn len(&self) -> usize {
        self.buffer.len()
    }

    fn to_bytes(&self) -> &[u8] {
        self.buffer.as_slice()
    }

    fn push(&mut self, data: u8) {
        self.buffer.push(data);
    }

    pub fn to_vec(&self) -> Vec<u8> {
        self.buffer.to_vec()
    }

    fn clear(&mut self) {
        self.buffer.clear();
    }
    pub fn to_inner(self) -> Vec<u8> {
        self.buffer.clone()
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

impl Encode<()> for Flags {
    fn encode<VecWriter: minicbor::encode::Write>(
        &self,
        e: &mut minicbor::encode::Encoder<VecWriter>,
        _ctx: &mut (),
    ) -> Result<(), minicbor::encode::Error<<VecWriter>::Error>> {
        e.u8(self.0);
        Ok(())
    }
}

#[derive(Debug, Clone, PartialEq, Encode, Decode)]
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
        timestamp: u64,
    },
    PingResp {
        timestamp: u64,
    },
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

impl Encode<()> for MqttSnMessage {
    fn encode<VecWriter: minicbor::encode::Write>(
        &self,
        encoder: &mut minicbor::encode::Encoder<VecWriter>,
        _ctx: &mut (),
    ) -> Result<(), minicbor::encode::Error<VecWriter::Error>> {
        encoder.begin_array()?;
        match self {
            MqttSnMessage::Connect {
                flags,
                duration,
                client_id,
            } => {
                encoder
                    .u8(0x04)?
                    .encode(flags)?
                    .u16(*duration)?
                    .str(&client_id);
            }
            MqttSnMessage::ConnAck { return_code } => {
                encoder.u8(0x05)?.encode(return_code);
            }
            MqttSnMessage::WillTopicReq => {
                encoder.u8(0x06);
            }
            MqttSnMessage::WillTopic { flags, topic } => {
                encoder.u8(0x07)?.encode(flags)?.str(&topic);
            }
            MqttSnMessage::WillMsgReq => {
                encoder.u8(0x08);
            }
            MqttSnMessage::WillMsg { message } => {
                encoder.u8(0x09)?.bytes(&message);
            }
            MqttSnMessage::Register {
                topic_id,
                msg_id,
                topic_name,
            } => {
                encoder
                    .u8(0x0a)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .str(&topic_name);
            }
            MqttSnMessage::RegAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                encoder
                    .u8(0x0b)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .encode(return_code);
            }
            MqttSnMessage::Publish {
                flags,
                topic_id,
                msg_id,
                data,
            } => {
                encoder
                    .u8(0x0c)?
                    .encode(flags)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .bytes(&data);
            }
            MqttSnMessage::PubAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                encoder
                    .u8(0x0d)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .encode(return_code);
            }
            MqttSnMessage::PubRec { msg_id } => {
                encoder.u8(0x0e)?.u16(*msg_id);
            }
            MqttSnMessage::PubRel { msg_id } => {
                encoder.u8(0x0f)?.u16(*msg_id);
            }
            MqttSnMessage::PubComp { msg_id } => {
                encoder.u8(0x10)?.u16(*msg_id);
            }
            MqttSnMessage::Subscribe {
                flags,
                msg_id,
                topic,
                topic_id,
                qos,
            } => {
                encoder.u8(0x12)?.encode(flags)?.u16(*msg_id);
                if let Some(topic) = topic {
                    encoder.str(&topic);
                } else {
                    encoder.u16(topic_id.unwrap());
                }
                encoder.u8(*qos);
            }
            MqttSnMessage::SubAck {
                flags,
                topic_id,
                msg_id,
                return_code,
            } => {
                encoder
                    .u8(0x13)?
                    .encode(flags)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .encode(return_code);
            }
            MqttSnMessage::Unsubscribe {
                flags,
                msg_id,
                topic,
                topic_id,
                qos,
            } => {
                encoder.u8(0x14)?.encode(flags)?.u16(*msg_id);
                if let Some(topic) = topic {
                    encoder.str(&topic)?;
                } else {
                    encoder.u16(topic_id.unwrap())?;
                }
                encoder.u8(*qos);
            }
            MqttSnMessage::UnsubAck {
                msg_id,
                return_code,
            } => {
                encoder.u8(0x15)?.u16(*msg_id)?.encode(return_code);
            }
            MqttSnMessage::PingReq { timestamp } => {
                encoder.u8(0x16)?.u64(*timestamp);
            }
            MqttSnMessage::PingResp { timestamp } => {
                encoder.u8(0x17)?.u64(*timestamp);
            }
            MqttSnMessage::Disconnect { duration } => {
                encoder.u8(0x18)?.u16(*duration);
            }
            MqttSnMessage::WillTopicUpd { flags, topic } => {
                encoder.u8(0x1a)?.encode(flags)?.str(&topic);
            }
            MqttSnMessage::WillMsgUpd { message } => {
                encoder.u8(0x1b)?.bytes(&message);
            }
            MqttSnMessage::WillTopicResp { return_code } => {
                encoder.u8(0x1c)?.encode(return_code);
            }
            MqttSnMessage::WillMsgResp { return_code } => {
                encoder.u8(0x1d)?.encode(return_code);
            }
        }
        encoder.end()?;
        Ok(())
    }
}

struct MqttSnDecoder {
    bytes: Vec<u8>,
    pos: usize,
}

impl MqttSnDecoder {
    fn new(bytes: &[u8]) -> Self {
        MqttSnDecoder {
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
    fn u32(&mut self) -> byte::Result<u32> {
        let v = u32::from_le_bytes([
            self.bytes[self.pos],
            self.bytes[self.pos + 1],
            self.bytes[self.pos + 2],
            self.bytes[self.pos + 3],
        ]);
        self.pos += 4;
        Ok(v)
    }
    fn u64(&mut self) -> byte::Result<u64> {
        let v = u64::from_le_bytes([
            self.bytes[self.pos],
            self.bytes[self.pos + 1],
            self.bytes[self.pos + 2],
            self.bytes[self.pos + 3],
            self.bytes[self.pos + 4],
            self.bytes[self.pos + 5],
            self.bytes[self.pos + 6],
            self.bytes[self.pos + 7],
        ]);
        self.pos += 8;
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

impl<'a> Decode<'a, ()> for Flags {
    fn decode(d: &mut Decoder<'a>, _ctx: &mut ()) -> Result<Self, minicbor::decode::Error> {
        let flags = d.u8()?;
        Ok(Flags(flags))
    }
}

impl<'a> Decode<'a, ()> for MqttSnMessage {
    fn decode(d: &mut Decoder<'a>, _ctx: &mut ()) -> Result<Self, minicbor::decode::Error> {
        d.array()?;
        let msg_type = d.u8()?;
        let res = match msg_type {
            0x04 => {
                let flags = d.decode()?;
                let duration = d.u16()?;
                let client_id = d.str()?;
                Ok(MqttSnMessage::Connect {
                    flags,
                    duration,
                    client_id: client_id.to_string(),
                })
            }
            0x05 => {
                let return_code = d.decode()?;
                Ok(MqttSnMessage::ConnAck { return_code })
            }
            0x06 => Ok(MqttSnMessage::WillTopicReq),
            0x07 => {
                let flags = d.decode()?;
                let topic = d.str()?;
                Ok(MqttSnMessage::WillTopic {
                    flags,
                    topic: topic.to_string(),
                })
            }
            0x08 => Ok(MqttSnMessage::WillMsgReq),
            0x09 => {
                let message = d.bytes()?;
                Ok(MqttSnMessage::WillMsg {
                    message: message.to_vec(),
                })
            }
            0x0a => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let topic_name = d.str()?;
                Ok(MqttSnMessage::Register {
                    topic_id,
                    msg_id,
                    topic_name: topic_name.to_string(),
                })
            }
            0x0b => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(MqttSnMessage::RegAck {
                    topic_id,
                    msg_id,
                    return_code,
                })
            }
            0x0c => {
                let flags = d.decode()?;
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let data = d.bytes()?;
                Ok(MqttSnMessage::Publish {
                    flags,
                    topic_id,
                    msg_id,
                    data: data.to_vec(),
                })
            }
            0x0d => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(MqttSnMessage::PubAck {
                    topic_id,
                    msg_id,
                    return_code,
                })
            }
            0x0e => {
                let msg_id = d.u16()?;
                Ok(MqttSnMessage::PubRec { msg_id })
            }
            0x0f => {
                let msg_id = d.u16()?;
                Ok(MqttSnMessage::PubRel { msg_id })
            }
            0x10 => {
                let msg_id = d.u16()?;
                Ok(MqttSnMessage::PubComp { msg_id })
            }
            0x12 => {
                let flags = d.decode()?;
                let msg_id = d.u16()?;
                let topic = d.str()?;
                let topic_id = d.u16()?;
                let qos = d.u8()?;
                Ok(MqttSnMessage::Subscribe {
                    flags,
                    msg_id,
                    topic: Some(topic.to_string()),
                    topic_id: Some(topic_id),
                    qos,
                })
            }
            0x13 => {
                let flags = d.decode()?;
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(MqttSnMessage::SubAck {
                    flags,
                    topic_id,
                    msg_id,
                    return_code,
                })
            }
            0x14 => {
                let flags = d.decode()?;
                let msg_id = d.u16()?;
                let topic = d.str()?;
                let topic_id = d.u16()?;
                let qos = d.u8()?;
                Ok(MqttSnMessage::Unsubscribe {
                    flags,
                    msg_id,
                    topic: Some(topic.to_string()),
                    topic_id: Some(topic_id),
                    qos,
                })
            }
            0x15 => {
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(MqttSnMessage::UnsubAck {
                    msg_id,
                    return_code,
                })
            }
            0x16 => {
                let timestamp = d.u64()?;
                Ok(MqttSnMessage::PingReq { timestamp })
            }
            0x17 => {
                let timestamp = d.u64()?;
                Ok(MqttSnMessage::PingResp { timestamp })
            }
            0x18 => {
                let duration = d.u16()?;
                Ok(MqttSnMessage::Disconnect { duration })
            }
            0x1a => {
                let flags = d.decode()?;
                let topic = d.str()?;
                Ok(MqttSnMessage::WillTopicUpd {
                    flags,
                    topic: topic.to_string(),
                })
            }
            0x1b => {
                let message = d.bytes()?;
                Ok(MqttSnMessage::WillMsgUpd {
                    message: message.to_vec(),
                })
            }
            0x1c => {
                let return_code = d.decode()?;
                Ok(MqttSnMessage::WillTopicResp { return_code })
            }
            0x1d => {
                let return_code = d.decode()?;
                Ok(MqttSnMessage::WillMsgResp { return_code })
            }
            _ => Err(DecodeError::message("unrecognized msg type ")),
        };
        res
    }
}

/*
impl<'a> TryRead<'a> for MqttSnMessage {
    fn try_read(bytes: &'a [u8], _ctx: ()) -> byte::Result<(Self, usize)> {
        let mut d = MqttSnDecoder::new(bytes);
        let msg_type = d.u8()?;
        match msg_type {
            0x04 => {
                let flags = Flags(d.u8()?);
                let duration = d.u16()?;
                let client_id = d.string()?;
                Ok((
                    MqttSnMessage::Connect {
                        flags,
                        duration,
                        client_id,
                    },
                    d.offset(),
                ))
            }
            0x05 => {
                let return_code = d.return_code()?;
                Ok((MqttSnMessage::ConnAck { return_code }, d.offset()))
            }
            0x06 => Ok((MqttSnMessage::WillTopicReq, d.offset())),
            0x07 => {
                let flags = Flags(d.u8()?);
                let topic = d.string()?;
                Ok((MqttSnMessage::WillTopic { flags, topic }, d.offset()))
            }
            0x08 => Ok((MqttSnMessage::WillMsgReq, d.offset())),
            0x09 => {
                let message = d.bytes()?;
                Ok((MqttSnMessage::WillMsg { message }, d.offset()))
            }
            0x0a => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let topic_name = d.string()?;
                Ok((
                    MqttSnMessage::Register {
                        topic_id,
                        msg_id,
                        topic_name,
                    },
                    d.offset(),
                ))
            }
            0x0b => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((
                    MqttSnMessage::RegAck {
                        topic_id,
                        msg_id,
                        return_code,
                    },
                    d.offset(),
                ))
            }
            0x0c => {
                let flags = Flags(d.u8()?);
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let data = d.bytes()?;
                Ok((
                    MqttSnMessage::Publish {
                        flags,
                        topic_id,
                        msg_id,
                        data,
                    },
                    d.offset(),
                ))
            }
            0x0d => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((
                    MqttSnMessage::PubAck {
                        topic_id,
                        msg_id,
                        return_code,
                    },
                    d.offset(),
                ))
            }
            0x0e => {
                let msg_id = d.u16()?;
                Ok((MqttSnMessage::PubRec { msg_id }, d.offset()))
            }
            0x0f => {
                let msg_id = d.u16()?;
                Ok((MqttSnMessage::PubRel { msg_id }, d.offset()))
            }
            0x10 => {
                let msg_id = d.u16()?;
                Ok((MqttSnMessage::PubComp { msg_id }, d.offset()))
            }
            0x12 => {
                let flags = Flags(d.u8()?);
                let msg_id = d.u16()?;
                let topic = d.string()?;
                let topic_id = d.u16()?;
                let qos = d.u8()?;
                Ok((
                    MqttSnMessage::Subscribe {
                        flags,
                        msg_id,
                        topic: Some(topic),
                        topic_id: Some(topic_id),
                        qos,
                    },
                    d.offset(),
                ))
            }
            0x13 => {
                let flags = Flags(d.u8()?);
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((
                    MqttSnMessage::SubAck {
                        flags,
                        topic_id,
                        msg_id,
                        return_code,
                    },
                    d.offset(),
                ))
            }
            0x14 => {
                let flags = Flags(d.u8()?);
                let msg_id = d.u16()?;
                let topic = d.string()?;
                let topic_id = d.u16()?;
                let qos = d.u8()?;
                Ok((
                    MqttSnMessage::Unsubscribe {
                        flags,
                        msg_id,
                        topic: Some(topic),
                        topic_id: Some(topic_id),
                        qos,
                    },
                    d.offset(),
                ))
            }
            0x15 => {
                let msg_id = d.u16()?;
                let return_code = ReturnCode::Accepted;
                Ok((
                    MqttSnMessage::UnsubAck {
                        msg_id,
                        return_code,
                    },
                    d.offset(),
                ))
            }
            0x16 => {
                if d.bytes.len() == d.pos {
                    return Ok((MqttSnMessage::PingReq { timestamp: 0 }, d.offset()));
                } else {
                    let timestamp = d.u64()?;
                    Ok((MqttSnMessage::PingReq { timestamp }, d.offset()))
                }
            }
            0x17 => {
                if d.bytes.len() == d.pos {
                    return Ok((MqttSnMessage::PingResp { timestamp: 0 }, d.offset()));
                } else {
                    let timestamp = d.u64()?;
                    Ok((MqttSnMessage::PingResp { timestamp }, d.offset()))
                }
            }
            0x18 => {
                let duration = d.u16()?;
                Ok((MqttSnMessage::Disconnect { duration }, d.offset()))
            }
            0x1a => {
                let flags = Flags(d.u8()?);
                let topic = d.string()?;
                Ok((
                    MqttSnMessage::WillTopicUpd { flags, topic },
                    d.offset(),
                ))
            }
            0x1b => {
                let message = d.bytes()?;
                Ok((MqttSnMessage::WillMsgUpd { message }, d.offset()))
            }
            0x1c => {
                let return_code = ReturnCode::Accepted;
                Ok((
                    MqttSnMessage::WillTopicResp { return_code },
                    d.offset(),
                ))
            }
            0x1d => {
                let return_code = ReturnCode::Accepted;
                Ok((MqttSnMessage::WillMsgResp { return_code }, d.offset()))
            }
            _ => Err(Error::BadInput {
                err: "parsing failed ",
            }),
        }
    }
}*/
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
