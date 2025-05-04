use anyhow::anyhow;
use anyhow::Result;
use log::debug;
use log::info;
use minicbor::display;
use minicbor::Encoder;
use minicbor::{data::Token, Decoder};
use mlua::FromLua;
use mlua::FromLuaMulti;
use mlua::IntoLua;
use mlua::IntoLuaMulti;
use std::collections::HashMap;
use std::fmt;
use std::fmt::Display;
use std::fmt::Formatter;

#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    MapStr(HashMap<String, Value>),
    MapIdx(HashMap<usize, Value>),
    List(Vec<Value>),
    String(String),
    Bytes(Vec<u8>),
    Number(f64),
    Bool(bool),
    Null,
}

impl Value {
    pub fn from_cbor_token(token: &Token) -> Result<Value> {
        match token {
            Token::Map(len) => {
                let mut map = HashMap::new();
                for _ in 0..*len {
                    let key = Value::from_cbor_token(token)?;
                    let value = Value::from_cbor_token(token)?;
                    map.insert(
                        key.as_f64().ok_or_else(|| anyhow!("Key not found"))? as usize,
                        value,
                    );
                }
                Ok(Value::MapIdx(map))
            }
            Token::Array(len) => {
                let mut list = Vec::new();
                for _ in 0..*len {
                    list.push(Value::from_cbor_token(token)?);
                }
                Ok(Value::List(list))
            }
            Token::F16(f) => Ok(Value::Number(*f as f64)),
            Token::F64(f) => Ok(Value::Number(*f)),
            Token::F32(f) => Ok(Value::Number(f64::from(*f))),
            Token::U8(u) => Ok(Value::Number(f64::from(*u))),
            Token::U16(u) => Ok(Value::Number(f64::from(*u))),
            Token::U32(u) => Ok(Value::Number(f64::from(*u))),
            Token::U64(u) => Ok(Value::Number(*u as f64)),
            Token::I8(i) => Ok(Value::Number(f64::from(*i))),
            Token::I16(i) => Ok(Value::Number(f64::from(*i))),
            Token::I32(i) => Ok(Value::Number(f64::from(*i))),
            Token::I64(i) => Ok(Value::Number(*i as f64)),
            Token::Simple(16) => Ok(Value::Null),
            Token::Simple(20) => Ok(Value::Bool(false)),
            Token::Simple(21) => Ok(Value::Bool(true)),
            Token::Bool(b) => Ok(Value::Bool(*b)),
            Token::String(s) => Ok(Value::String(s.to_string())),
            Token::Null => Ok(Value::Null),
            Token::Bytes(bytes) => {
                let vec = bytes.iter().map(|b| *b).collect();
                Ok(Value::Bytes(vec))
            }
            _ => Err(anyhow!("Unsupported other token type {:?} ", token)),
        }
    }
    pub fn from_cbor(bytes: Vec<u8>) -> Result<Value> {
        debug!("from_cbor {}", display(&bytes));
        let mut decoder = Decoder::new(&bytes);
        let tokens = decoder.tokens().collect::<Result<Vec<Token>, _>>()?;
        let mut iter = tokens.iter();
        let first_token = iter.next().unwrap();
        match first_token {
            Token::BeginMap => {
                let mut map = HashMap::new();
                loop {
                    let key = iter.next().ok_or_else(|| anyhow!("Key not found"))?;
                    if key == &Token::Break {
                        break;
                    }
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    let key = match key {
                        Token::U32(i) => i.to_string(),
                        Token::U8(i) => i.to_string(),
                        Token::U16(i) => i.to_string(),
                        Token::F16(f) => f.to_string(),
                        Token::F32(f) => f.to_string(),
                        Token::String(s) => s.to_string(),
                        Token::Bool(b) => b.to_string(),
                        _ => return Err(anyhow!("Unsupported key type {}", key)),
                    };
                    map.insert(key, Value::from_cbor_token(value)?);
                }
                Ok(Value::MapStr(map))
            }
            Token::Map(len) => {
                let mut map = HashMap::new();
                for _ in 0..*len {
                    let key = iter
                        .next()
                        .ok_or_else(|| anyhow::anyhow!("Key not found"))?;
                    let value = iter
                        .next()
                        .ok_or_else(|| anyhow::anyhow!("Value not found"))?;
                    let key = match key {
                        Token::U32(i) => i.to_string(),
                        Token::U8(i) => i.to_string(),
                        Token::U16(i) => i.to_string(),
                        Token::String(s) => s.to_string(),
                        Token::I32(i) => i.to_string(),
                        Token::I8(i) => i.to_string(),
                        Token::I16(i) => i.to_string(),
                        Token::I64(i) => i.to_string(),
                        Token::U64(i) => i.to_string(),
                        _ => return Err(anyhow::anyhow!("Unsupported key type {}", key)),
                    };
                    map.insert(key, Value::from_cbor_token(value)?);
                }
                Ok(Value::MapStr(map))
            }
            Token::BeginArray => {
                let mut list = Vec::new();
                loop {
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    if value == &Token::Break {
                        break;
                    }
                    list.push(Value::from_cbor_token(value)?);
                }
                Ok(Value::List(list))
            }
            Token::Array(len) => {
                let mut list = Vec::new();
                for _ in 0..*len {
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    list.push(Value::from_cbor_token(value)?);
                }
                Ok(Value::List(list))
            }
            Token::F64(f) => Ok(Value::Number(*f)),
            Token::F32(f) => Ok(Value::Number(f64::from(*f))),
            Token::U16(u) => Ok(Value::Number(f64::from(*u))),
            Token::U32(u) => Ok(Value::Number(f64::from(*u))),
            Token::U64(u) => Ok(Value::Number(*u as f64)),
            Token::I16(i) => Ok(Value::Number(f64::from(*i))),
            Token::I32(i) => Ok(Value::Number(f64::from(*i))),
            Token::I64(i) => Ok(Value::Number(*i as f64)),

            Token::Simple(16) => Ok(Value::Null),
            Token::Simple(20) => Ok(Value::Bool(false)),
            Token::Simple(21) => Ok(Value::Bool(true)),
            _ => Err(anyhow::anyhow!(
                "Unsupported first token type {:?} ",
                first_token
            )),
        }
    }

    pub fn to_cbor(&self) -> Result<Vec<u8>> {
        let mut writer = Vec::<u8>::new();
        let mut encoder = Encoder::new(&mut writer);
        self.encode(&mut encoder)?;
        Ok(writer)
    }

    pub fn encode(&self, encoder: &mut Encoder<&mut Vec<u8>>) -> Result<()> {
        match self {
            Value::MapStr(map) => {
                encoder.begin_map()?;
                for (key, value) in map {
                    encoder.str(key)?;
                    value.encode(encoder)?;
                }
                encoder.end()?
            }
            Value::MapIdx(map) => {
                encoder.begin_map()?;
                for (key, value) in map {
                    encoder.u32(*key as u32)?;
                    value.encode(encoder)?;
                }
                encoder.end()?
            }
            Value::List(list) => {
                encoder.begin_array()?;
                for value in list {
                    value.encode(encoder)?;
                }
                encoder.end()?
            }
            Value::String(s) => encoder.str(s)?,
            Value::Number(n) => encoder.f32(*n as f32)?,
            Value::Bool(b) => encoder.bool(*b)?,
            Value::Bytes(bytes) => encoder.bytes(bytes)?,
            Value::Null => encoder.null()?,
        };
        Ok(())
    }

    pub fn from_text(text: &String) -> Value {
        if let Ok(v) = text.parse::<f32>() {
            Value::Number(v as f64)
        } else if let Ok(v) = text.parse::<f64>() {
            Value::Number(v)
        } else if let Ok(v) = text.parse::<i32>() {
            Value::Number(v as f64)
        } else if let Ok(v) = text.parse::<u32>() {
            Value::Number(v as f64)
        } else if let Ok(v) = text.parse::<bool>() {
            Value::Bool(v)
        } else if let Ok(v) = text.parse::<u64>() {
            Value::Number(v as f64)
        } else {
            Value::String(text.clone())
        }
    }

    pub fn as_f64(&self) -> Option<f64> {
        match self {
            Value::Number(n) => Some(*n),
            _ => None,
        }
    }
    pub fn as_str(&self) -> Option<&str> {
        match self {
            Value::String(s) => Some(s),
            _ => None,
        }
    }
    pub fn as_bool(&self) -> Option<bool> {
        match self {
            Value::Bool(b) => Some(*b),
            _ => None,
        }
    }
    pub fn at_idx(&self, idx: usize) -> Option<&Value> {
        match self {
            Value::List(list) => list.get(idx),
            Value::MapIdx(map) => map.get(&idx),
            _ => None,
        }
    }

    pub fn get_opt(&self, key: &Option<String>) -> Option<&Value> {
        match key {
            Some(key) => self.get(key.as_str()),
            None => Some(self),
        }
    }

    pub fn get(&self, key: &str) -> Option<&Value> {
        match self {
            Value::MapIdx(map) => {
                let idx = key.parse::<usize>().ok()?;
                map.get(&idx)
            }
            Value::MapStr(map) => map.get(key),
            Value::List(list) => {
                let idx = key.parse::<usize>().ok()?;
                list.get(idx)
            }
            _ => None,
        }
    }
    pub fn keys(&self) -> Option<Vec<String>> {
        match self {
            Value::MapStr(map) => Some(map.keys().cloned().collect()),
            Value::MapIdx(map) => Some(map.keys().map(|k| k.to_string()).collect()),
            Value::List(list) => Some((0..list.len()).map(|i| i.to_string()).collect()),
            _ => None,
        }
    }
}

impl Display for Value {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        match self {
            Value::MapIdx(map) => {
                write!(f, "{{")?;
                for (i, (key, value)) in map.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}: {}", key, value)?;
                }
                write!(f, "}}")
            }
            Value::MapStr(map) => {
                write!(f, "{{")?;
                for (i, (key, value)) in map.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}: {}", key, value)?;
                }
                write!(f, "}}")
            }
            Value::List(list) => {
                write!(f, "[")?;
                for (i, value) in list.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", value)?;
                }
                write!(f, "]")
            }
            Value::String(s) => write!(f, "[{}]", s.len()),
            Value::Number(n) => write!(f, "{}", n),
            Value::Bool(b) => write!(f, "{}", b),
            Value::Bytes(bytes) => write!(f, "[{}]", bytes.len()),
            Value::Null => write!(f, "null"),
        }
    }
}

impl Default for Value {
    fn default() -> Self {
        Value::Null
    }
}

impl IntoLua for Value {
    fn into_lua(self, lua: &mlua::Lua) -> mlua::Result<mlua::Value> {
        match self {
            Value::MapStr(map) => {
                let table = lua.create_table()?;
                for (key, value) in map {
                    table.set(key, value)?;
                }
                Ok(mlua::Value::Table(table))
            }
            Value::MapIdx(map) => {
                let table = lua.create_table()?;
                for (key, value) in map {
                    table.set(key, value)?;
                }
                Ok(mlua::Value::Table(table))
            }
            Value::List(list) => {
                let table = lua.create_table()?;
                for (i, value) in list.into_iter().enumerate() {
                    table.set(i + 1, value)?;
                }
                Ok(mlua::Value::Table(table))
            }
            Value::String(s) => Ok(mlua::Value::String(lua.create_string(&s)?)),
            Value::Number(n) => Ok(mlua::Value::Number(n)),
            Value::Bool(b) => Ok(mlua::Value::Boolean(b)),
            Value::Bytes(bytes) => Ok(mlua::Value::String(lua.create_string(&bytes)?)),
            Value::Null => Ok(mlua::Value::Nil),
        }
    }
}

impl FromLua for Value {
    fn from_lua(value: mlua::Value, lua: &mlua::Lua) -> mlua::Result<Value> {
        match value {
            mlua::Value::Table(table) => {
                let mut map_str = HashMap::new();
                let mut map_idx = HashMap::new();
                let mut list = Vec::new();
                let mut is_map_str = true;
                for pair in table.pairs::<mlua::Value, mlua::Value>() {
                    let (key, value) = pair?;
                    if let mlua::Value::String(key) = key {
                        if let Ok(idx) = key.to_str()?.parse::<usize>() {
                            map_idx.insert(idx, Value::from_lua(value, lua)?);
                            is_map_str = false;
                        } else {
                            map_str.insert(key.to_str()?.to_string(), Value::from_lua(value, lua)?);
                        }
                    } else {
                        list.push(Value::from_lua(value, lua)?);
                    }
                }
                if is_map_str {
                    Ok(Value::MapStr(map_str))
                } else {
                    Ok(Value::MapIdx(map_idx))
                }
            }
            mlua::Value::String(s) => Ok(Value::String(s.to_str()?.to_string())),
            mlua::Value::Number(n) => Ok(Value::Number(n)),
            mlua::Value::Boolean(b) => Ok(Value::Bool(b)),
            mlua::Value::Nil => Ok(Value::Null),
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to: "Value".to_string(),
                message: Some("unsupported type".to_string()),
            }),
        }
    }
}
