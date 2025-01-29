use std::collections::HashMap;
use std::fmt;
use std::fmt::Display;
use std::fmt::Formatter;
use anyhow::Result;
use anyhow::anyhow;
use minicbor::{data::Token, Decoder};

#[derive(Debug,Clone)]
pub enum Value {
    MapStr(HashMap<String, Value>),
    MapIdx(HashMap<usize, Value>),
    List(Vec<Value>),
    String(String),
    Number(f64),
    Bool(bool),
    Null,
}

impl Value {
    pub fn from_token(token: &Token) -> Result<Value> {
        match token {
            Token::Map(len) => {
                let mut map = HashMap::new();
                for _ in 0..*len {
                    let key = Value::from_token(token)?;
                    let value = Value::from_token(token)?;
                    map.insert(key.as_f64().ok_or_else(|| anyhow!("Key not found"))? as usize, value);
                }
                Ok(Value::MapIdx(map))
            }
            Token::Array(len) => {
                let mut list = Vec::new();
                for _ in 0..*len {
                    list.push(Value::from_token(token)?);
                }
                Ok(Value::List(list))
            }
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
            _ => Err(anyhow!("Unsupported other token type {:?} ", token)),
        }
    }
    pub fn from_cbor( bytes : Vec<u8> ) -> Result<Value> {
        let mut decoder = Decoder::new(&bytes);
        let tokens = decoder.tokens().collect::<Result<Vec<Token>, _>>()?;
        let mut iter = tokens.iter();
        let first_token = iter.next().unwrap();
        match first_token {
            Token::BeginMap => {
                let mut map = HashMap::new();
                loop {
                    let key = iter.next().ok_or_else(|| anyhow!("Key not found"))?;
                    if  key == &Token::Break  {
                        break;
                    }
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    let key = match key {
                        Token::U32(i) => *i as usize,
                        Token::U8(i) => *i as usize,
                        Token::U16(i) => *i as usize,
                        _ => return Err( anyhow!("Unsupported key type")),
                    };
                    map.insert(key, Value::from_token(value)?);
                }
                Ok(Value::MapIdx(map))
            }
            Token::Map(len) => {
                let mut map = HashMap::new();
                for _ in 0..*len {
                    let key = iter.next().ok_or_else(|| anyhow::anyhow!("Key not found"))?;
                    let value = iter.next().ok_or_else(|| anyhow::anyhow!("Value not found"))?;
                    let key = match key {
                        Token::U32(i) => *i as usize,
                        _ => return Err( anyhow::anyhow!("Unsupported key type")),
                    };
                    map.insert(key, Value::from_token(value)?);
                }
                Ok(Value::MapIdx(map))
            }
            Token::BeginArray => {
                let mut list = Vec::new();
                loop {
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    if value == &Token::Break {
                        break;
                    }
                    list.push(Value::from_token(value)?);
                }
                Ok(Value::List(list))
            }
            Token::Array(len) => {
                let mut list = Vec::new();
                for _ in 0..*len {
                    let value = iter.next().ok_or_else(|| anyhow!("Value not found"))?;
                    list.push(Value::from_token(value)?);
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
            _ => Err(anyhow::anyhow!("Unsupported first token type {:?} ", first_token)),
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
            Value::String(s) => write!(f, "\"{}\"", s),
            Value::Number(n) => write!(f, "{}", n),
            Value::Bool(b) => write!(f, "{}", b),
            Value::Null => write!(f, "null"),
        }
    }
}

impl Default for Value {
    fn default() -> Self {
        Value::Null
    }
}