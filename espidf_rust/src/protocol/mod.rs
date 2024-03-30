use std::any::Any;

use serde::{Deserialize, Serialize};
pub enum LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
}

impl Serialize for LogLevel {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        serializer.serialize_i8(*self as i8)
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
        serializer.serialize_u64(self.timestamp)?;
        serializer.serialize_str(self.message.as_str())?;
        self.level.serialize(serializer)?;

        if self.component.is_none() {
            serializer.serialize_none()?;
        } else {
            serializer.serialize_str(self.component.as_ref().unwrap().as_str())?;
        }
        if self.file.is_none() {
            serializer.serialize_none()?;
        } else {
            serializer.serialize_str(self.file.as_ref().unwrap().as_str())?;
        }
        if self.line.is_none() {
            serializer.serialize_none()?;
        } else {
            serializer.serialize_u32(self.line.unwrap())?;
        }
        Ok(S::Ok)
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
#[derive(Serialize, Deserialize, Debug)]

struct Query {
    pub dst_topic: Id,
    pub src_topic: Id,
    pub payload: Vec<u8>,
}

#[repr(u8)]
pub enum Message {
    Log(Log) = 0,
    SessionOpen(SessionOpen),
    SessionClose(SessionClose),
    SessionRegister(SessionRegister),
    Publish(Publish),
    Subscribe(Subscribe),
    Query(Query),
}

impl Serialize for Message {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        match self {
            Message::Log(log) => {
                let seq = serializer.serialize_seq(None)?;
                log.serialize(seq);
                seq.end();
                Ok(())
            }
            _ => Ok(()),
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

impl Query {
    pub fn new(dst_topic: Id, src_topic: Id, payload: Vec<u8>) -> Query {
        Query {
            dst_topic: dst_topic,
            src_topic: src_topic,
            payload: payload,
        }
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

    pub fn new_query(dst_topic: Id, src_topic: Id, payload: Vec<u8>) -> Message {
        Message::Query(Query::new(dst_topic, src_topic, payload))
    }
}
