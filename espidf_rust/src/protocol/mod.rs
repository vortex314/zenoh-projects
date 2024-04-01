use std::any::Any;
use std::ptr::null;

use serde::ser::SerializeSeq;
use serde::{Deserialize, Serialize};

#[derive(Deserialize, Clone,Copy)]
pub enum LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
}

fn value(foo: &LogLevel) -> u8 {
    *foo as u8
}

impl Serialize for LogLevel {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        serializer.serialize_u8(value(self))
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
            timestamp: 0,
            message: message.to_string(),
            level: None,
            component: None,
            file: None,
            line: None,
        }
    }
}

impl Serialize for Log {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        let mut seq = serializer.serialize_seq(None)?;
        seq.serialize_element(&self.timestamp)?;
        seq.serialize_element(&self.message.as_str())?;
        seq.serialize_element(&self.level)?;
        seq.serialize_element(&self.component)?;
        seq.serialize_element(&self.file)?;
        seq.serialize_element(&self.line)?;
        seq.end()
    }
}

pub enum Kind {
    Publisher,
    Subscriber,
    Queryable,
}

#[derive(Serialize, Deserialize, Debug)]

pub struct SessionOpen {
    pub mtu: u16,
    pub mss: u16,
}
#[derive(Serialize, Deserialize, Debug)]

pub struct SessionClose {
    pub reason: String,
}

pub struct SessionRegister {
    pub name: String,
    pub id: Id,
    pub kind: Kind,
}
#[derive(Serialize, Deserialize, Debug)]

pub struct Publish {
    pub topic: Id,
    pub payload: Vec<u8>,
}
#[derive(Serialize, Deserialize, Debug)]

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

impl Serialize for Message {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        match self {
            Message::Log(log) => {
                log.serialize(serializer)
            }
            _ => serializer.serialize_i32(1),
        }
    }
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
