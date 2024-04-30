use crate::protocol::ProxyMessage;
use crate::protocol::encode_frame;
use crate::protocol::decode_frame;
use minicbor::encode;
use tokio::io::split;
use tokio_serial::*;
use log::*;
use tokio::io::AsyncReadExt;
use tokio_util::codec::{Decoder, Encoder};
use bytes::BytesMut;
use std::io;

pub async fn port_handler(port_info : SerialPortInfo ) {
    info!("Port Handler started for port {}", port_info.port_name);

    let serial_stream = tokio_serial::new(port_info.port_name, 115200)
        .open_native_async()
        .unwrap();
    let x = CobsCodec.framed(serial_stream);
    let (mut writer, mut reader) = split(x);
    loop {
        let mut buf = [0; 256];
        let n = reader.read(&mut buf).await.unwrap();
        if n == 0 {
            info!("Port {} closed", port_info.port_name);
            break;
        }
        let mut queue = Vec::new();
        queue.extend_from_slice(&buf[0..n]);
        let msg = decode_frame(&queue);
        match msg {
            Some(msg) => {
                info!("Received message : {:?}", msg);
            }
            None => {
                info!("Invalid message");
            }
        }
    }
    
}

struct CobsCodec;

impl Decoder for CobsCodec {
    type Item = ProxyMessage;
    type Error = io::Error;

    fn decode(&mut self, _src: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        Ok(None)
    }
}

impl Encoder<ProxyMessage> for CobsCodec {
    type Error = io::Error;

    fn encode(&mut self, _item: ProxyMessage, _dst: &mut BytesMut) -> Result<(), Self::Error> {
        let bytes = encode_frame(_item);
         match bytes {
            Ok(bytes) => {
                let mut buf = BytesMut::with_capacity(bytes.len());
                buf.put(bytes);
            }
            Err(e) => {
                return Err(io::Error::new(io::ErrorKind::Other, e));
            }
        }
        Ok(())
    }
}