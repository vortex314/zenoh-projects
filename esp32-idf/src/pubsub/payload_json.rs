use anyhow::Result;
use serde::{de::DeserializeOwned, Deserialize, Serialize};
use serde_json::{json, Value};

use super::PayloadCodec;

pub struct Json {
}

impl PayloadCodec for Json{
    fn as_f64(payload:&Vec<u8>) -> Result<f64> {
        let s = String::from_utf8_lossy(payload);
        let v: Value = serde_json::from_str(&s)?;
        let x: f64 = serde_json::from_value(v)?;
        Ok(x)    }

    fn to_string(payload:&Vec<u8>) -> String {
        String::from_utf8_lossy(payload).to_string()
    }

    fn decode<T>(payload:&Vec<u8>) -> Result<T>
    where
        T: DeserializeOwned,
    {
        let s = String::from_utf8_lossy(payload);
        let v: Value = serde_json::from_str(&s)?;
        let x: T = serde_json::from_value(v)?;
        Ok(x)    }

    fn encode<T>( value: &T) -> Vec<u8>
    where
        T: Serialize,
    {
        let x = serde_json::to_string(&value).unwrap();
        x.as_bytes().to_vec()
    }
}
