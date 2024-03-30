use serde::{Deserialize, Serialize};
#[derive(Serialize, Deserialize, Debug)]
pub enum LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
}

#[derive(Serialize, Deserialize, Debug)]

pub enum Kind {
    Publisher,
    Subscriber,
    Queryable,
}

type Id = u16;
#[derive(Serialize, Deserialize, Debug)]

pub struct Log {
    pub timestamp: u64,
    pub message: String,
    pub level: Option<LogLevel>,
    pub component: Option<String>,
    pub file: Option<String>,
    pub line: Option<u32>,
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
#[derive(Serialize, Deserialize, Debug)]

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

#[derive(Serialize, Deserialize, Debug)]

pub enum Message {
    Log(Log),
    SessionOpen(SessionOpen),
    SessionClose(SessionClose),
    SessionRegister(SessionRegister),
    Publish(Publish),
    Subscribe(Subscribe),
    Query(Query),
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

fn main() {
    let log = Message::new_log("Hello, World!");
    let session_open = Message::new_session_open(1500, 1460);
    let session_close = Message::new_session_close("Goodbye, World!");
    let session_register = Message::new_session_register("Hello, World!", 0, Kind::Publisher);
    let publish = Message::new_publish(0, vec![0, 1, 2, 3, 4]);
    let subscribe = Message::new_subscribe(0);
    let query = Message::new_query(0, 1, vec![0, 1, 2, 3, 4]);

    println!("{:?}", log);
    println!("{:?}", session_open);
    println!("{:?}", session_close);
    println!("{:?}", session_register);
    println!("{:?}", publish);
    println!("{:?}", subscribe);
    println!("{:?}", query);
}
