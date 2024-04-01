#![allow(unused_imports)]
#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(unused_mut)]
use std::{cell::RefCell, rc::Rc};

use ciborium::cbor;
use log::info;
use cobs::CobsEncoder;
use crc::{poly::{CRC_16, CRC_16_ANSI}, Crc, CRC_16_IBM_SDLC};
use serde::Serialize;
mod protocol;

#[derive(Clone)]
struct VecWriter{

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

impl ciborium_io::Write for VecWriter {
    type Error = String;
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.borrow_mut().extend_from_slice(buf);
        Ok(())
    }
    fn flush(&mut self) -> Result<(), Self::Error> {
        Ok(())
    }
}


fn encode_connect_request() -> Result<Vec<u8>, String>{
    let log_msg = protocol::Message::Log(protocol::Log::new("test"));
    let value = cbor!([1,"gg",3.14]);
    let mut buffer = VecWriter::new();
    let value=1;
    let mut buffer_clone = buffer.clone();
    let serializer = ciborium::ser::into_writer(&log_msg, buffer);
    let mut cobs_buffer = [0;256];
    let crc16 =  Crc::<u16>::new(&CRC_16_IBM_SDLC);

    info!("Encoded length : {}", buffer_clone.len());
    let crc = crc16.checksum(&buffer_clone.to_bytes());
    buffer_clone.push((crc & 0xFF) as u8);
    buffer_clone.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer_clone.to_bytes()).unwrap();
    let size  = cobs_encoder.finalize().unwrap();
    cobs_buffer[size] = 0;
    Ok(cobs_buffer[0..(size+1)].to_vec())
}

fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_svc::sys::link_patches();

    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    let x = encode_connect_request().unwrap();
    info!("Encoded : {:02X?}", x);


    info!("Hello, world!");
}
