use core::time;
use std::any::Any;
use std::ptr::null;
use std::rc::Rc;
use std::cell::RefCell;

use minicbor::{Decode, Encode};

#[derive(Clone)]
pub struct VecWriter{

    buffer:Rc<RefCell<Vec<u8>>>,
}

impl VecWriter  {
    fn new() -> Self {
        VecWriter { buffer: Rc::new(RefCell::new(Vec::new()))}
    }

    fn len(&self) -> usize {
        self.buffer.borrow().len()
    }

    fn to_bytes(&self) -> Vec<u8> {
        self.buffer.borrow().clone()
    }

    fn push(&mut self, data: u8) {
        self.buffer.borrow_mut().push(data);
    }

    fn clear(&mut self) {
        self.buffer.borrow_mut().clear();
    }

}

impl minicbor::encode::write::Write for VecWriter {
    type Error = String;
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.borrow_mut().extend_from_slice(buf);
        Ok(())
    }

}


pub enum MessageType {
    Log = 1,
    SessionOpen = 2,
    SessionClose = 3,
    SessionRegister = 4,
    Publish = 5,
    Subscribe = 6,
}

impl Encode<()> for MessageType {
    fn encode(&self, e: &mut minicbor::Encoder<VecWriter>) -> Result<(), minicbor::encode::Error<String>> {
        e.u8(*self as u8)
    }
    fn is_nil(&self) -> bool {
        false
    }
}

impl Decode<'_,()> for MessageType {
    fn decode(d: &mut minicbor::Decoder) -> Result<Self, minicbor::decode::Error> {
        Ok(match d.u8()? {
            1 => MessageType::Log,
            2 => MessageType::SessionOpen,
            3 => MessageType::SessionClose,
            4 => MessageType::SessionRegister,
            5 => MessageType::Publish,
            6 => MessageType::Subscribe,
            _ => return Err(minicbor::decode::Error::Message("Invalid message type".to_string())),
        })
    }
}

fn message_type(foo: &MessageType) -> u8 {
    *foo as u8
}

pub enum LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
}

fn log_level(foo: &LogLevel) -> u8 {
    *foo as u8
}

impl Encode<()> for LogLevel {
    fn encode(&self, e: &mut minicbor::Encoder<VecWriter>) -> Result<(), minicbor::encode::Error<String>> {
        e.u8(*self as u8)
    }
}

impl Decode<'_,()> for LogLevel {
    fn decode(d: &mut minicbor::Decoder,ctx:&mut ()) -> Result<Self, minicbor::decode::Error> {
        Ok(match d.u8()? {
            0 => LogLevel::Trace,
            1 => LogLevel::Debug,
            2 => LogLevel::Info,
            3 => LogLevel::Warning,
            4 => LogLevel::Error,
            _ => return Err(Error::new("Invalid log level".to_string())),
        })
    }

    fn nil() -> Option<Self> {
        Some(LogLevel::Trace)
    }
}

type Id = u16;

pub struct Log {
    pub timestamp: u64,
    pub message: String,
    pub level: Option<LogLevel>,
    pub component: Option<String>,
    pub file: Option<String>,
    pub line: Option<u32>,
}

impl Log {
    pub fn new(message: &str) -> Log {
        Log {
            timestamp: 5,
            message: message.to_string(),
            level: None,
            component: None,
            file: None,
            line: None,
        }
    }
}

fn encode<T>(&x:Option<T> ,e: &mut minicbor::Encoder<VecWriter>) -> Result<(), minicbor::encode::Error<String>> 
where T: Encode<()>
  {
    if x.is_nil() {
        e.null()?
    } else {
        x.encode(e)?
    }
}

impl Encode<()> for Log {
    fn encode(&self, e: &mut minicbor::Encoder<VecWriter>) -> Result<(), minicbor::encode::Error<String>> {
        e.array(6)?;
        e.u8(message_type(&MessageType::Log))?;
        e.u64(self.timestamp)?;
        e.str(&self.message)?;
        encode(self.level,e)?;
        encode(self.component,e)?;
        encode(self.file,e)?;
        encode(self.line,e) 
    }
}
/* 
impl Serialize for Log {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        let mut seq = serializer.serialize_seq(None)?;
        seq.serialize_element(&messageType(&MessageType::Log))?;
        seq.serialize_element(&self.timestamp)?;
        seq.serialize_element(&self.message.as_str())?;
        seq.serialize_element(&self.level)?;
        seq.serialize_element(&self.component)?;
        seq.serialize_element(&self.file)?;
        seq.serialize_element(&self.line)?;
        seq.end()
    }
}*/

pub enum Kind {
    Publisher,
    Subscriber,
    Queryable,
}

pub struct SessionOpen {
    pub mtu: u16,
    pub mss: u16,
}

pub struct SessionClose {
    pub reason: String,
}

pub struct SessionRegister {
    pub name: String,
    pub id: Id,
    pub kind: Kind,
}

pub struct Publish {
    pub topic: Id,
    pub payload: Vec<u8>,
}

pub struct Subscribe {
    pub topic: Id,
}

#[repr(u8)]
pub enum Message {
    Log(Log) = 0,
    SessionOpen(SessionOpen),
    SessionClose(SessionClose),
    SessionRegister(SessionRegister),
    Publish(Publish),
    Subscribe(Subscribe),
}



impl SessionOpen {
    pub fn new(mtu: u16, mss: u16) -> SessionOpen {
        SessionOpen { mtu: mtu, mss: mss }
    }
}

impl SessionClose {
    pub fn new(reason: &str) -> SessionClose {
        SessionClose {
            reason: reason.to_string(),
        }
    }
}

impl SessionRegister {
    pub fn new(name: &str, id: Id, kind: Kind) -> SessionRegister {
        SessionRegister {
            name: name.to_string(),
            id: id,
            kind: kind,
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
    pub fn new(topic: Id) -> Subscribe {
        Subscribe { topic: topic }
    }
}

impl Message {
    pub fn new_log(message: &str) -> Message {
        Message::Log(Log::new(message))
    }

    pub fn new_session_open(mtu: u16, mss: u16) -> Message {
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

    pub fn new_subscribe(topic: Id) -> Message {
        Message::Subscribe(Subscribe::new(topic))
    }
}
