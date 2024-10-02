use alloc::string::String;
use alloc::vec::Vec;
use anyhow::Result;
use serde::Deserialize;
use serde::Serialize;
use serde_json_core::de::from_slice;
use serde_json_core::ser::to_vec;

pub fn decode<'a, T>(data: &'a Vec<u8>) -> Result<T>
where
    T: Deserialize<'a>,
{
    let r = from_slice::<T>(data);
    let r = r.map_err(anyhow::Error::msg)?;
    Ok(r.0)
}

pub fn encode<T>(value: &T) -> Vec<u8>
where
    T: Serialize + ?Sized,
{
    let r = to_vec::<T,256>(value);
    match r {
        Ok(r) => r.to_vec(),
        Err(_e) => Vec::new(),
    }
}

pub fn to_string(data: &Vec<u8>) -> String {
    String::from_utf8_lossy(data).into_owned()
    //serde_json::to_string(data).map_err(anyhow::Error::msg).unwrap()
}

pub fn as_f64<'a>(data:&'a Vec<u8>) -> Result<f64> {
    let r = from_slice::<f64>(data).map_err(anyhow::Error::msg)?;
    Ok(r.0)
}
