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
        match e.u8(self.0) {
            Ok(_) => Ok(()),
            Err(_e) => Err(minicbor::encode::Error::<<VecWriter>::Error>::message(
                "Error encoding flags".to_string(),
            )),
        }
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
pub enum ProxyMessage {
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
        topic: String,
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

impl Encode<()> for ProxyMessage {
    fn encode<VecWriter: minicbor::encode::Write>(
        &self,
        encoder: &mut minicbor::encode::Encoder<VecWriter>,
        _ctx: &mut (),
    ) -> Result<(), minicbor::encode::Error<VecWriter::Error>> {
        encoder.begin_array()?;
        match self {
            ProxyMessage::Connect {
                flags,
                duration,
                client_id,
            } => {
                encoder
                    .u8(0x04)?
                    .encode(flags)?
                    .u16(*duration)?
                    .str(&client_id)?;
            }
            ProxyMessage::ConnAck { return_code } => {
                encoder.u8(0x05)?.encode(return_code)?;
            }
            ProxyMessage::WillTopicReq => {
                encoder.u8(0x06)?;
            }
            ProxyMessage::WillTopic { flags, topic } => {
                encoder.u8(0x07)?.encode(flags)?.str(&topic)?;
            }
            ProxyMessage::WillMsgReq => {
                encoder.u8(0x08)?;
            }
            ProxyMessage::WillMsg { message } => {
                encoder.u8(0x09)?.bytes(&message)?;
            }
            ProxyMessage::Register {
                topic_id,
                msg_id,
                topic_name,
            } => {
                encoder
                    .u8(0x0a)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .str(&topic_name)?;
            }
            ProxyMessage::RegAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                encoder
                    .u8(0x0b)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .encode(return_code)?;
            }
            ProxyMessage::Publish {
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
                    .bytes(&data)?;
            }
            ProxyMessage::PubAck {
                topic_id,
                msg_id,
                return_code,
            } => {
                encoder
                    .u8(0x0d)?
                    .u16(*topic_id)?
                    .u16(*msg_id)?
                    .encode(return_code)?;
            }
            ProxyMessage::PubRec { msg_id } => {
                encoder.u8(0x0e)?.u16(*msg_id)?;
            }
            ProxyMessage::PubRel { msg_id } => {
                encoder.u8(0x0f)?.u16(*msg_id)?;
            }
            ProxyMessage::PubComp { msg_id } => {
                encoder.u8(0x10)?.u16(*msg_id)?;
            }
            ProxyMessage::Subscribe {
                flags,
                msg_id,
                topic,
                qos,
            } => {
                encoder.u8(0x12)?.encode(flags)?.u16(*msg_id)?;
                encoder.str(&topic)?;
                encoder.u8(*qos)?;
            }
            ProxyMessage::SubAck {
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
                    .encode(return_code)?;
            }
            ProxyMessage::Unsubscribe {
                flags,
                msg_id,
                topic,
                topic_id,
                qos,
            } => {
                encoder.u8(0x14)?.encode(flags)?.u16(*msg_id)?;
                if let Some(topic) = topic {
                    encoder.str(&topic)?;
                } else {
                    encoder.u16(topic_id.unwrap())?;
                }
                encoder.u8(*qos)?;
            }
            ProxyMessage::UnsubAck {
                msg_id,
                return_code,
            } => {
                encoder.u8(0x15)?.u16(*msg_id)?.encode(return_code)?;
            }
            ProxyMessage::PingReq { timestamp } => {
                encoder.u8(0x16)?.u64(*timestamp)?;
            }
            ProxyMessage::PingResp { timestamp } => {
                encoder.u8(0x17)?.u64(*timestamp)?;
            }
            ProxyMessage::Disconnect { duration } => {
                encoder.u8(0x18)?.u16(*duration)?;
            }
            ProxyMessage::WillTopicUpd { flags, topic } => {
                encoder.u8(0x1a)?.encode(flags)?.str(&topic)?;
            }
            ProxyMessage::WillMsgUpd { message } => {
                encoder.u8(0x1b)?.bytes(&message)?;
            }
            ProxyMessage::WillTopicResp { return_code } => {
                encoder.u8(0x1c)?.encode(return_code)?;
            }
            ProxyMessage::WillMsgResp { return_code } => {
                encoder.u8(0x1d)?.encode(return_code)?;
            }
        }
        encoder.end()?;
        Ok(())
    }
}

impl<'a> Decode<'a, ()> for Flags {
    fn decode(d: &mut Decoder<'a>, _ctx: &mut ()) -> Result<Self, minicbor::decode::Error> {
        let flags = d.u8()?;
        Ok(Flags(flags))
    }
}

impl<'a> Decode<'a, ()> for ProxyMessage {
    fn decode(d: &mut Decoder<'a>, _ctx: &mut ()) -> Result<Self, minicbor::decode::Error> {
        d.array()?;
        let msg_type = d.u8()?;
        let res = match msg_type {
            0x04 => {
                let flags = d.decode()?;
                let duration = d.u16()?;
                let client_id = d.str()?;
                Ok(ProxyMessage::Connect {
                    flags,
                    duration,
                    client_id: client_id.to_string(),
                })
            }
            0x05 => {
                let return_code = d.decode()?;
                Ok(ProxyMessage::ConnAck { return_code })
            }
            0x06 => Ok(ProxyMessage::WillTopicReq),
            0x07 => {
                let flags = d.decode()?;
                let topic = d.str()?;
                Ok(ProxyMessage::WillTopic {
                    flags,
                    topic: topic.to_string(),
                })
            }
            0x08 => Ok(ProxyMessage::WillMsgReq),
            0x09 => {
                let message = d.bytes()?;
                Ok(ProxyMessage::WillMsg {
                    message: message.to_vec(),
                })
            }
            0x0a => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let topic_name = d.str()?;
                Ok(ProxyMessage::Register {
                    topic_id,
                    msg_id,
                    topic_name: topic_name.to_string(),
                })
            }
            0x0b => {
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(ProxyMessage::RegAck {
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
                Ok(ProxyMessage::Publish {
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
                Ok(ProxyMessage::PubAck {
                    topic_id,
                    msg_id,
                    return_code,
                })
            }
            0x0e => {
                let msg_id = d.u16()?;
                Ok(ProxyMessage::PubRec { msg_id })
            }
            0x0f => {
                let msg_id = d.u16()?;
                Ok(ProxyMessage::PubRel { msg_id })
            }
            0x10 => {
                let msg_id = d.u16()?;
                Ok(ProxyMessage::PubComp { msg_id })
            }
            0x12 => {
                let flags = d.decode()?;
                let msg_id = d.u16()?;
                let topic = d.str()?;
                let qos = d.u8()?;
                Ok(ProxyMessage::Subscribe {
                    flags,
                    msg_id,
                    topic:topic.to_string(),
                    qos,
                })
            }
            0x13 => {
                let flags = d.decode()?;
                let topic_id = d.u16()?;
                let msg_id = d.u16()?;
                let return_code = d.decode()?;
                Ok(ProxyMessage::SubAck {
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
                Ok(ProxyMessage::Unsubscribe {
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
                Ok(ProxyMessage::UnsubAck {
                    msg_id,
                    return_code,
                })
            }
            0x16 => {
                let timestamp = d.u64()?;
                Ok(ProxyMessage::PingReq { timestamp })
            }
            0x17 => {
                let timestamp = d.u64()?;
                Ok(ProxyMessage::PingResp { timestamp })
            }
            0x18 => {
                let duration = d.u16()?;
                Ok(ProxyMessage::Disconnect { duration })
            }
            0x1a => {
                let flags = d.decode()?;
                let topic = d.str()?;
                Ok(ProxyMessage::WillTopicUpd {
                    flags,
                    topic: topic.to_string(),
                })
            }
            0x1b => {
                let message = d.bytes()?;
                Ok(ProxyMessage::WillMsgUpd {
                    message: message.to_vec(),
                })
            }
            0x1c => {
                let return_code = d.decode()?;
                Ok(ProxyMessage::WillTopicResp { return_code })
            }
            0x1d => {
                let return_code = d.decode()?;
                Ok(ProxyMessage::WillMsgResp { return_code })
            }
            _ => Err(DecodeError::message("unrecognized msg type ")),
        };
        res
    }
}

