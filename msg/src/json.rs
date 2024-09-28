
use log::info;
use serde::{de::DeserializeOwned, Serialize};
use anyhow::Result;

pub fn decode<T>(data: &Vec<u8>) -> Result<T>
where
    T: DeserializeOwned,
{
    serde_json::from_slice(data).map_err(anyhow::Error::msg)
}

pub fn encode<T>(data: &T) -> Vec<u8>
where
    T: Serialize,
{
    serde_json::to_vec(data).map_err(anyhow::Error::msg).unwrap()
}

pub fn to_string(data: &Vec<u8>) -> String
{
    String::from_utf8_lossy(data).into_owned()
    //serde_json::to_string(data).map_err(anyhow::Error::msg).unwrap()
}

pub fn as_f64(data: &Vec<u8>) -> Result<f64>
{
    let json = String::from_utf8_lossy(data).into_owned();
    let value :f64= serde_json::from_str(json.as_str()).map_err(anyhow::Error::msg)?;
    Ok(value)
}