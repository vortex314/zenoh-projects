use minicbor::{encode::write::EndOfSlice, Decode, Encode,Encoder,Decoder };
use log::info;
use crc::CRC_16_IBM_SDLC;
use crc::Crc;
use std::{cell::RefCell, rc::Rc};


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

#[derive(Encode, Decode,Debug)]
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

#[derive(Encode, Decode,Debug)]
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
#[derive(Encode, Decode,Debug,PartialEq)]
#[cbor(index_only)]

pub enum Kind {
    #[n(0)]
    Publisher = 0,
    #[n(1)]
    Subscriber,
    #[n(2)]
    Queryable,
}
#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub struct SessionOpen {
    #[n(0)]
    pub mtu: u32,
    #[n(1)]
    pub mss: u32,
    #[n(2)]
    pub qos: u8,
    #[n(3)]
    pub client_id: Option<String>,
}
#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub struct SessionClose {
    #[n(0)]
    pub reason: Option<String>,
}
#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub struct SessionRegister {
    #[n(0)]
    pub name: String,
    #[n(1)]
    pub id: Id,
    #[n(2)]
    pub kind: Option<Kind>,
}
#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub struct Publish {
    #[n(0)]
    pub topic: Id,
    #[n(1)]
    pub payload: Vec<u8>,
}
#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub struct Subscribe {
    #[n(0)]
    pub topic: String,
}

struct Query {
    pub dst_topic: Id,
    pub src_topic: Id,
    pub payload: Vec<u8>,
}

#[derive(Encode, Decode,Debug)]
#[cbor(array)]
pub enum Message {
    #[n(0)]
    Log(#[n(0)] Log),
    #[n(1)]
    SessionOpen(#[n(1)] SessionOpen),
    #[n(2)]
    SessionClose(#[n(2)] SessionClose),
    #[n(3)]
    SessionRegister(#[n(3)] SessionRegister),
    #[n(4)]
    Publish(#[n(4)] Publish),
    #[n(5)]
    Subscribe(#[n(5)] Subscribe),
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

impl SessionOpen {
    pub fn new(mtu: u32, mss: u32) -> SessionOpen {
        SessionOpen {
            mtu,
            mss,
            qos: 0,
            client_id: None,
        }
    }
}

impl SessionClose {
    pub fn new(reason: &str) -> SessionClose {
        SessionClose {
            reason: Some(reason.to_string()),
        }
    }
}

impl SessionRegister {
    pub fn new(name: &str, id: Id, kind: Kind) -> SessionRegister {
        SessionRegister {
            name: name.to_string(),
            id: id,
            kind: Some(kind),
        }
    }
}

impl Publish {
    pub fn new(topic: Id, payload: Vec<u8>) -> Publish {
        Publish {
            topic: topic,
            payload: payload,
        }
    }
}

impl Subscribe {
    pub fn new(topic: String) -> Subscribe {
        Subscribe { topic }
    }
}

impl Query {
    pub fn new(dst_topic: Id, src_topic: Id, payload: Vec<u8>) -> Query {
        Query {
            dst_topic,
            src_topic,
            payload,
        }
    }
}

impl Message {
    pub fn new_log(message: &str) -> Message {
        Message::Log(Log::new(message))
    }

    pub fn new_session_open(mtu: u32, mss: u32) -> Message {
        Message::SessionOpen(SessionOpen::new(mtu, mss))
    }

    pub fn new_session_close(reason: &str) -> Message {
        Message::SessionClose(SessionClose::new(reason))
    }

    pub fn new_session_register(name: &str, id: Id, kind: Kind) -> Message {
        Message::SessionRegister(SessionRegister::new(name, id, kind))
    }

    pub fn new_publish(topic: Id, payload: Vec<u8>) -> Message {
        Message::Publish(Publish::new(topic, payload))
    }

    pub fn new_subscribe(topic: String) -> Message {
        Message::Subscribe(Subscribe::new(topic))
    }
}

fn test() {
    let log = Message::new_log("Hello, World!");
    let session_open = Message::new_session_open(1500, 1460);
    let session_close = Message::new_session_close("Goodbye, World!");
    let session_register = Message::new_session_register("Hello, World!", 0, Kind::Publisher);
    let publish = Message::new_publish(0, vec![0, 1, 2, 3, 4]);
    let subscribe = Message::new_subscribe("src/*".to_string());

    let _messages = vec![
        log,
        session_open,
        session_close,
        session_register,
        publish,
        subscribe,
    ];
}


pub fn make_frame(msg: Message) -> Result<Vec<u8>, String> {
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
    info!("COBS : {:02X?}", &cobs_buffer[0..(size+1)]);
    Ok(cobs_buffer[0..size+1].to_vec())
}

pub fn decode_frame(frame_bytes: Vec<u8>) -> Result<Message, String> {
    if frame_bytes.len() < 2 {
        return Err("Frame too short".to_string());
    }
    let mut cobs_output = [0; 256];
    let mut cobs_decoder = cobs::CobsDecoder::new(&mut cobs_output);
    let res = cobs_decoder.push(&frame_bytes);
    match res {
        Ok(offset) => {
            match offset {
                None => {
                    return Err("COBS more data needed.".to_string());
                },
                Some((n, m)) => {
                    if m != frame_bytes.len() {
                        return Err("COBS decoding error".to_string());
                    }
                    let bytes = cobs_output[0..n].to_vec();
                    info!("Decoded : {:02X?}", bytes);

                    let crc16 = Crc::<u16>::new(&CRC_16_IBM_SDLC);
                    let crc = crc16.checksum(&bytes[0..(bytes.len() - 2)]);
                    let crc_received =
                        (bytes[bytes.len() - 1] as u16) << 8 | bytes[bytes.len() - 2] as u16;
                    if crc != crc_received {
                        info!("CRC error : {:04X} != {:04X}", crc, crc_received);
                        return Err("CRC error".to_string());
                    }

                    let mut decoder = minicbor::decode::Decoder::new(&bytes);
                    let mut ctx = 1;
                    let msg = Message::decode(&mut decoder, &mut ctx).unwrap();
                    Ok(msg)
                }
            }
        },
        Err(j) => {
            info!("COBS decoding error : {:?}", j);
            Err("COBS decoding error".to_string())
        },
    }
}

#[cfg(test)]


mod tests {
    use super::*;
    #[test]
    fn test_log() {
        let log = Message::new_log("Hello, World!");
        let bytes = make_frame(log).unwrap();
        let decoded = decode_frame(bytes).unwrap();
        match decoded {
            Message::Log(log) => {
                assert_eq!(log.message, "Hello, World!");
            },
            _ => {
                assert!(false);
            },
        }
    }

    #[test]
    fn test_session_open() {
        let session_open = Message::new_session_open(1500, 1460);
        let bytes = make_frame(session_open).unwrap();
        let decoded = decode_frame(bytes).unwrap();
        match decoded {
            Message::SessionOpen(session_open) => {
                assert_eq!(session_open.mtu, 1500);
                assert_eq!(session_open.mss, 1460);
            },
            _ => {
                assert!(false);
            },
        }
    }

    #[test]
    fn test_session_close() {
        let session_close = Message::new_session_close("Goodbye, World!");
        let bytes = make_frame(session_close).unwrap();
        let decoded = decode_frame(bytes).unwrap();
        match decoded {
            Message::SessionClose(session_close) => {
                assert_eq!(session_close.reason.unwrap(), "Goodbye, World!");
            },
            _ => {
                assert!(false);
            },
        }
    }

    #[test]
    fn test_session_register() {
        let session_register = Message::new_session_register("Hello, World!", 0, Kind::Publisher);
        let bytes = make_frame(session_register).unwrap();
        let decoded = decode_frame(bytes).unwrap();
        match decoded {
            Message::SessionRegister(session_register) => {
                assert_eq!(session_register.name, "Hello, World!");
                assert_eq!(session_register.id, 0);
                assert_eq!(session_register.kind.unwrap(), Kind::Publisher);
            },
            _ => {
                assert!(false);
            },
        }
    }

 }