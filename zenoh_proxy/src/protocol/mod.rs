use minicbor::{Decode, Encode};

type Id = u16;

#[derive(Encode, Decode)]
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

#[derive(Encode, Decode)]
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
#[derive(Encode, Decode)]
#[cbor(index_only)]

pub enum Kind {
    #[n(0)]
    Publisher = 0,
    #[n(1)]
    Subscriber,
    #[n(2)]
    Queryable,
}
#[derive(Encode, Decode)]
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
#[derive(Encode, Decode)]
#[cbor(array)]
pub struct SessionClose {
    #[n(0)]
    pub reason: Option<String>,
}
#[derive(Encode, Decode)]
#[cbor(array)]
pub struct SessionRegister {
    #[n(0)]
    pub name: String,
    #[n(1)]
    pub id: Id,
    #[n(2)]
    pub kind: Option<Kind>,
}
#[derive(Encode, Decode)]
#[cbor(array)]
pub struct Publish {
    #[n(0)]
    pub topic: Id,
    #[n(1)]
    pub payload: Vec<u8>,
}
#[derive(Encode, Decode)]
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

#[derive(Encode, Decode)]
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
