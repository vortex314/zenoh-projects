#![allow(unused_imports)]
#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(unused_mut)]
use log::info;
use cobs::CobsEncoder;
use crc::{poly::{CRC_16, CRC_16_ANSI}, Crc, CRC_16_IBM_SDLC};
use serde::Serialize;
mod protocol;

struct VecWriter{
    buffer:Vec<u8>,
}

impl VecWriter  {
    fn new() -> Self {
        VecWriter { buffer: Vec::new()}
    }

    fn len(&self) -> usize {
        self.buffer.len()
    }

    fn to_bytes(&self) -> &[u8] {
        self.buffer.as_slice()
    }

    fn push(&mut self, data: u8) {
        self.buffer.push(data);
    }

    fn to_vec(&self) -> Vec<u8> {
        self.buffer.to_vec()
    }

    fn clear(&mut self) {
        self.buffer.clear();
    }

}

impl ciborium_io::Write for VecWriter {
    type Error = String;
    fn write_all(&mut self, buf: &[u8]) -> Result<(), Self::Error> {
        self.buffer.extend_from_slice(buf);
        Ok(())
    }
    fn flush(&mut self) -> Result<(), Self::Error> {
        Ok(())
    }
}


fn encode_connect_request() -> Result<Vec<u8>, String>{
    let log_msg = protocol::Message::Log(protocol::Log::new("test"));
    let mut buffer = VecWriter::new();
    let value=1;
    let serializer = ciborium::ser::into_writer(&log_msg, buffer);
    let mut cobs_buffer = [0;256];
    let crc16 =  Crc::<u16>::new(&CRC_16_IBM_SDLC);

    info!("Encoded length : {}", buffer.len());
    let crc = crc16.checksum(&buffer.to_bytes());
    buffer.push((crc & 0xFF) as u8);
    buffer.push(((crc >> 8) & 0xFF) as u8);
    let mut cobs_encoder = cobs::CobsEncoder::new(&mut cobs_buffer);
    let mut _res = cobs_encoder.push(&buffer.to_bytes()).unwrap();
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
