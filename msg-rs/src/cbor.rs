use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;
use anyhow::Result;
use minicbor::{data::Token, Decode, Decoder, Encode, Encoder};

pub fn as_f64(payload: &Vec<u8>) -> Result<f64> {
    let mut decoder = Decoder::new(payload);
    let v = decoder
        .tokens()
        .collect::<Result<Vec<Token>, _>>()
        .map_err(anyhow::Error::msg)?;
    match v[0] {
        Token::F16(f) => Ok(f as f64),
        Token::F32(f) => Ok(f as f64),
        Token::F64(f) => Ok(f),
        Token::I16(i) => Ok(i as f64),
        Token::I32(i) => Ok(i as f64),
        Token::I64(i) => Ok(i as f64),
        Token::U16(i) => Ok(i as f64),
        Token::U32(i) => Ok(i as f64),
        Token::U64(i) => Ok(i as f64),
        Token::I8(i) => Ok(i as f64),
        Token::U8(i) => Ok(i as f64),
        Token::Bool(b) => Ok(if b { 1.0 } else { 0.0 }),
        _ => anyhow::bail!("type mismatch"),
    }
}

pub fn to_string(payload: &Vec<u8>) -> String {
    let line: String = payload.iter().map(|b| format!("{:02X} ", b)).collect();
    let s = format!("{}", minicbor::display(payload.as_slice()));
    if s.len() == 0 {
        line
    } else {
        s
    }
}

pub fn decode<'a, T>(payload: &'a Vec<u8>) -> Result<T>
where
    T: Decode<'a, ()>,
{
    Decoder::new(payload).decode().map_err(anyhow::Error::msg)
}

pub fn encode<T>(value: &T) -> Vec<u8>
where
    T: Encode<()>,
{
    let mut encoder = Encoder::new(Vec::new());
    match encoder.encode(value) {
        Ok(_) => encoder.into_writer(),
        Err(_) => return Vec::new(),
    }
}
