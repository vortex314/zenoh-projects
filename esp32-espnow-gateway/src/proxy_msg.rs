use minicbor::{Encode, Decode};

#[derive(Encode, Decode),cbor(array)]

struct EspNowHeader {
    #[n(0)] dst : [u8,6],
    #[n(1)] src: [u8,6],
    #[n(2)] channel : u8,
    #[n(3)] rssi : i8,
}

#[derive(Encode, Decode),cbor(index_only)]
enum LogLevel {
    Debug=0,
    Info,
    Warn,
    Error,
}

#[derive(Encode, Decode),cbor(array)]
struct Log {
    #[n(0)] timestamp : u64,
    #[n(1)] message : &str,
    #[n(2)] file_line : Option<&str>,
    #[n(3)] level : Option<LogLevel>,
}

#[derive(Encode, Decode),cbor(array)]
struct Send {
    #[n(0)]
    header : EspNowHeader;
    #[n(1)]
    payload : [u8; 250];
}

struct Recv {
    #[n(0)]
    header : EspNowHeader;
    #[n(1)]
    payload : [u8; 250];
}

#[derive(Encode, Decode),cbor(array)]

enum ProxyMessage {
    #[n(0)]
    Recv(Recv),
    #[n(1)]
    Send(Send), 
    #[n(2)]
    Log(Log)
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
        header : EspNowHeader {
            dst : [0,0,0,0,0,0],
            src : [0,0,0,0,0,0],
            channel : 0,
            rssi : 0,
        },
        payload : [0; 250],
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