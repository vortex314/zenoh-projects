use alloc::string::String;
use alloc::vec::Vec;
use minicbor::{Decode, Encode};
use minicbor_derive::*;

#[derive(Encode, Decode)]
#[cbor(array)]

pub struct EspNowHeader {
    #[n(0)]
    pub dst: Option<[u8; 6]>,
    #[n(1)]
    pub src: Option<[u8; 6]>,
    #[n(2)]
    pub channel: u8,
    #[n(3)]
    pub rssi: u8,
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

#[derive(Encode, Decode)]
#[cbor(array)]
pub struct SendCmd {
    #[n(0)]
    pub header: EspNowHeader,
    #[n(1)]
    pub payload: Vec<u8>,
}

#[derive(Encode, Decode)]
#[cbor(array)]
pub struct RecvEvent {
    #[n(0)]
    pub header: EspNowHeader,
    #[n(1)]
    pub payload: Vec<u8>,
}

#[derive(Encode, Decode)]
#[cbor(array)]
pub enum ProxyMessage {
    #[n(0)]
    RecvEvent(#[n(0)] RecvEvent),
    #[n(1)]
    SendCmd(#[n(0)] SendCmd),
    #[n(2)]
    Log(#[n(0)] Log),
}
/*
decode Recv message to Pub and Info message
- register info messages to translate id's to names
- send pub messages to broker
- message from broker that meet subs criteria are sent to the device
*/

#[test]
fn test() {
    let msg = ProxyMessage::Recv(Recv {
        header: EspNowHeader {
            dst: [0, 0, 0, 0, 0, 0],
            src: [0, 0, 0, 0, 0, 0],
            channel: 0,
            rssi: 0,
        },
        payload: [0; 250],
    });

    let mut buf = [0; 300];
    let mut writer = minicbor::Write::new(&mut buf);
    msg.encode(&mut writer).unwrap();
    let len = writer.as_slice().len();
    println!("Encoded message: {:?}", &buf[..len]);
    mincbor::display(buf);

    let mut reader = minicbor::Read::new(&buf[..len]);
    let decoded = ProxyMessage::decode(&mut reader).unwrap();
    println!("Decoded message: {:?}", decoded);
}
