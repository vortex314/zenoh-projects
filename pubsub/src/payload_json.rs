use alloc::string::String;
use alloc::vec::Vec;
use anyhow::Result;
use serde::{de::DeserializeOwned, Deserialize, Serialize};
use crate::alloc::string::ToString;

use super::PayloadCodec;

pub struct Json {}

impl PayloadCodec for Json {
    fn as_f64(payload: &Vec<u8>) -> Result<f64> {
        serde_json_core::from_slice(payload)
        .map(|(v,_size)| v)
        .map_err(anyhow::Error::msg)
    }

    fn to_string(payload: &Vec<u8>) -> String {
        String::from_utf8_lossy(payload).to_string()
    }

    fn decode<T>(payload: &Vec<u8>) -> Result<T>
    where
        T: DeserializeOwned,
    {
        serde_json_core::from_slice(payload)
        .map(|(v,_size)| v)
        .map_err(anyhow::Error::msg)
    }

    fn encode<T>(value: &T) -> Vec<u8>
    where
        T: Serialize,
    {
        let buffer = &mut [0u8;256];
        serde_json_core::to_slice(value,buffer).map(|size| buffer[..size].to_vec()).unwrap()
    }
}
