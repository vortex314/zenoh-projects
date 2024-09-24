use minicbor::{Decode, Encode};


#[derive(Encode, Decode,Clone)]
#[cbor(array)]
pub struct EspNowHeader {
    #[n(0)]
    pub dst: Option<[u8; 6]>,
    #[n(1)]
    pub src: Option<[u8; 6]>,
    #[n(2)]
    pub channel: Option<u8>,
    #[n(3)]
    pub rssi: Option<u8>,
}

#[derive(Encode, Decode)]
#[cbor(index_only)]
pub enum LogLevel {
    #[n(0)]
    Debug,
    #[n(1)]
    Info,
    #[n(2)]
    Warn,
    #[n(3)]
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
    pub file_line: Option<String>,
    #[n(3)]
    pub level: Option<LogLevel>,
}

#[derive(Encode, Decode,Clone)]
#[cbor(array)]
pub struct SendCmd {
    #[n(0)]
    pub header: EspNowHeader,
    #[n(1)]
    pub payload: Vec<u8>,
}

#[derive(Encode, Decode,Clone)]
#[cbor(array)]
pub struct EspNowMessage {
    #[n(0)]
    pub header: EspNowHeader,
    #[n(1)]
    pub payload: Vec<u8>,
}

#[derive(Encode, Decode)]
#[cbor(array)]
pub enum ProxyMessage {
    #[n(0)]
    RecvEvent(#[n(0)] EspNowMessage),
    #[n(1)]
    SendCmd(#[n(0)] SendCmd),
    #[n(2)]
    Log(#[n(0)] Log),
}