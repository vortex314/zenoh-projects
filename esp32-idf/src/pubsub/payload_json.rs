use anyhow::Result;
use serde_json::{Value, json};
use serde::{Serialize, Deserialize};

pub fn payload_encode_json<X>( v: X) -> Vec<u8>
where
    X: serde::Serialize,
{
    let x = serde_json::to_string(&v).unwrap();
    x.as_bytes().to_vec()
}


pub fn payload_decode_json<T>(v: & Vec<u8>) -> Result<T>
where T : serde::de::DeserializeOwned
{
    let s = String::from_utf8_lossy(v);
    let v: Value = serde_json::from_str(&s)?;
    let x: T = serde_json::from_value(v)?;
    Ok(x)
}


pub fn payload_display_json(v: &Vec<u8>) -> String {
    String::from_utf8_lossy(v).to_string()
}


pub fn payload_as_f64_json (payload: &Vec<u8>) -> Result<f64> {
    let s = String::from_utf8_lossy(payload);
    let v: Value = serde_json::from_str(&s)?;
    let x: f64 = serde_json::from_value(v)?;
    Ok(x)
}
