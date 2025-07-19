use indexmap::IndexMap;
use std::fmt;

struct Undefined {}

#[derive(Debug, Clone, PartialEq, Default)]
pub enum Value {
    #[default]
    Undefined,
    Null,
    Bool(bool),
    Int(i64),
    Float(f32),
    Double(f64),
    String(String),
    Bytes(Vec<u8>),
    Array(Vec<Value>),
    Object(IndexMap<String, Value>),
}

impl Value {
    // check type
    pub fn is<T: 'static>(&self) -> bool {
        match self {
            Value::Undefined => false,
            Value::Null => false,
            Value::Bool(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<bool>() => true,
            Value::Int(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<i64>() => true,
            Value::Float(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<f32>() => true,
            Value::Double(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<f64>() => {
                true
            }
            Value::String(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<String>() => {
                true
            }
            Value::Bytes(_) if std::any::TypeId::of::<T>() == std::any::TypeId::of::<Vec<u8>>() => {
                true
            }
            Value::Array(_)
                if std::any::TypeId::of::<T>() == std::any::TypeId::of::<Vec<Value>>() =>
            {
                true
            }
            Value::Object(_)
                if std::any::TypeId::of::<T>()
                    == std::any::TypeId::of::<IndexMap<String, Value>>() =>
            {
                true
            }
            _ => false,
        }
    }
    // convert to type
    pub fn as_<T: 'static>(&self) -> Option<&T> {
        use std::any::Any;
        match self {
            Value::Undefined => None,
            Value::Null => None,
            Value::Bool(b) => (b as &dyn Any).downcast_ref::<T>(),
            Value::Int(i) => (i as &dyn Any).downcast_ref::<T>(),
            Value::Float(f) => (f as &dyn Any).downcast_ref::<T>(),
            Value::Double(d) => (d as &dyn Any).downcast_ref::<T>(),
            Value::String(s) => (s as &dyn Any).downcast_ref::<T>(),
            Value::Bytes(b) => (b as &dyn Any).downcast_ref::<T>(),
            Value::Array(a) => (a as &dyn Any).downcast_ref::<T>(),
            Value::Object(o) => (o as &dyn Any).downcast_ref::<T>(),
        }
    }

    // handle with closure if it is of a type
    pub fn handle<T: 'static, F>(&self, mut f: F)
    where
        F: FnMut(&T),
    {
        if let Some(value) = self.as_::<T>() {
            f(value);
        }
    }
    // Constructors
    pub fn null() -> Self {
        Value::Null
    }

    pub fn bool(b: bool) -> Self {
        Value::Bool(b)
    }

    pub fn int(i: i64) -> Self {
        Value::Int(i)
    }

    pub fn float(f: f32) -> Self {
        Value::Float(f)
    }

    pub fn string<S: Into<String>>(s: S) -> Self {
        Value::String(s.into())
    }

    pub fn bytes<B: Into<Vec<u8>>>(b: B) -> Self {
        Value::Bytes(b.into())
    }

    pub fn from_array(arr: Vec<Value>) -> Self {
        Value::Array(arr)
    }

    pub fn array() -> Self {
        Value::Array(vec![])
    }

    pub fn object() -> Self {
        let hm = IndexMap::<String, Value>::new();
        Value::Object(hm)
    }

    // Type checking
    pub fn is_null(&self) -> bool {
        matches!(self, Value::Null)
    }

    pub fn is_bool(&self) -> bool {
        matches!(self, Value::Bool(_))
    }

    pub fn is_int(&self) -> bool {
        matches!(self, Value::Int(_))
    }

    pub fn is_float(&self) -> bool {
        matches!(self, Value::Float(_))
    }

    pub fn is_number(&self) -> bool {
        self.is_int() || self.is_float()
    }

    pub fn is_string(&self) -> bool {
        matches!(self, Value::String(_))
    }

    pub fn is_bytes(&self) -> bool {
        matches!(self, Value::Bytes(_))
    }

    pub fn is_array(&self) -> bool {
        matches!(self, Value::Array(_))
    }

    pub fn is_object(&self) -> bool {
        matches!(self, Value::Object(_))
    }

    // Getters with type conversion
    pub fn as_bool(&self) -> Option<bool> {
        if let Value::Bool(b) = self {
            Some(*b)
        } else {
            None
        }
    }

    pub fn as_int(&self) -> Option<i64> {
        match self {
            Value::Int(i) => Some(*i),
            Value::Float(f) => Some(*f as i64),
            _ => None,
        }
    }

    pub fn as_float(&self) -> Option<f32> {
        match self {
            Value::Int(i) => Some(*i as f32),
            Value::Float(f) => Some(*f),
            Value::Double(d) => Some(*d as f32),
            _ => None,
        }
    }

    pub fn as_string(&self) -> Option<&str> {
        if let Value::String(s) = self {
            Some(s)
        } else {
            None
        }
    }

    pub fn as_bytes(&self) -> Option<&[u8]> {
        if let Value::Bytes(b) = self {
            Some(b)
        } else {
            None
        }
    }

    pub fn as_array(&self) -> Option<&Vec<Value>> {
        if let Value::Array(a) = self {
            Some(a)
        } else {
            None
        }
    }

    pub fn as_object(&self) -> Option<&IndexMap<String, Value>> {
        if let Value::Object(o) = self {
            Some(o)
        } else {
            None
        }
    }

    // Mutable getters
    pub fn as_array_mut(&mut self) -> Option<&mut Vec<Value>> {
        if let Value::Array(a) = self {
            Some(a)
        } else {
            None
        }
    }

    pub fn as_object_mut(&mut self) -> Option<&mut IndexMap<String, Value>> {
        if let Value::Object(o) = self {
            Some(o)
        } else {
            None
        }
    }
}

impl From<i64> for Value {
    fn from(v: i64) -> Self {
        Value::Int(v)
    }
}

impl From<&str> for Value {
    fn from(v: &str) -> Self {
        Value::String(v.to_string())
    }
}

impl From<String> for Value {
    fn from(v: String) -> Self {
        Value::String(v)
    }
}

impl From<f32> for Value {
    fn from(v: f32) -> Self {
        Value::Float(v)
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Undefined => write!(f, "undefined"),
            Value::Null => write!(f, "null"),
            Value::Bool(b) => write!(f, "{}", b),
            Value::Int(i) => write!(f, "{}", i),
            Value::Float(n) => write!(f, "{}", n),
            Value::Double(n) => write!(f, "{}", n),
            Value::String(s) => write!(f, "\"{}\"", s.escape_debug()),
            Value::Bytes(b) => write!(f, "bytes[{}]", b.len()),
            Value::Array(a) => {
                write!(f, "[")?;
                for (i, v) in a.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", v)?;
                }
                write!(f, "]")
            }
            Value::Object(o) => {
                write!(f, "{{")?;
                for (i, (k, v)) in o.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "\"{}\": {}", k.escape_debug(), v)?;
                }
                write!(f, "}}")
            }
        }
    }
}

use base64::{Engine as _, engine::general_purpose};
use log::error;
use serde::de::{self, Visitor};
use serde::{Deserialize, Deserializer, Serialize, Serializer};

struct ValueVisitor;

impl<'de> Visitor<'de> for ValueVisitor {
    type Value = Value;

    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        formatter.write_str("any valid JSON value")
    }

    fn visit_bool<E>(self, value: bool) -> Result<Self::Value, E> {
        Ok(Value::Bool(value))
    }

    fn visit_i64<E>(self, value: i64) -> Result<Self::Value, E> {
        Ok(Value::Int(value))
    }

    fn visit_u64<E>(self, value: u64) -> Result<Self::Value, E> {
        Ok(Value::Int(value as i64))
    }

    fn visit_f64<E>(self, value: f64) -> Result<Self::Value, E> {
        Ok(Value::Double(value))
    }

    fn visit_f32<E>(self, value: f32) -> Result<Self::Value, E> {
        Ok(Value::Float(value))
    }

    fn visit_str<E>(self, value: &str) -> Result<Self::Value, E>
    where
        E: de::Error,
    {
        Ok(Value::String(value.to_owned()))
    }

    fn visit_string<E>(self, value: String) -> Result<Self::Value, E> {
        Ok(Value::String(value))
    }

    fn visit_bytes<E>(self, value: &[u8]) -> Result<Self::Value, E> {
        Ok(Value::Bytes(value.to_vec()))
    }

    fn visit_byte_buf<E>(self, value: Vec<u8>) -> Result<Self::Value, E> {
        Ok(Value::Bytes(value))
    }

    fn visit_none<E>(self) -> Result<Self::Value, E> {
        Ok(Value::Null)
    }

    fn visit_unit<E>(self) -> Result<Self::Value, E> {
        Ok(Value::Null)
    }

    fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
    where
        A: de::SeqAccess<'de>,
    {
        let mut vec = Vec::new();
        while let Some(elem) = seq.next_element()? {
            vec.push(elem);
        }
        Ok(Value::Array(vec))
    }

    fn visit_map<A>(self, mut map: A) -> Result<Self::Value, A::Error>
    where
        A: de::MapAccess<'de>,
    {
        let mut hashmap = IndexMap::new();
        while let Some((key, value)) = map.next_entry()? {
            hashmap.insert(key, value);
        }
        Ok(Value::Object(hashmap))
    }
}

impl<'de> Deserialize<'de> for Value {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        deserializer.deserialize_any(ValueVisitor)
    }
}

fn escape_json_string(s: &str) -> String {
    let mut result = String::with_capacity(s.len());
    for c in s.chars() {
        match c {
            '"' => result.push_str("\\\""),
            '\\' => result.push_str("\\\\"),
            '\x08' => result.push_str("\\b"),
            '\x0C' => result.push_str("\\f"),
            '\n' => result.push_str("\\n"),
            '\r' => result.push_str("\\r"),
            '\t' => result.push_str("\\t"),
            c if c.is_control() => result.push_str(&format!("\\u{:04x}", c as u32)),
            c => result.push(c),
        }
    }
    result
}

impl Value {
    pub fn to_json(&self) -> String {
        self.to_json_internal()
    }

    fn to_json_internal(&self) -> String {
        match self {
            Value::Undefined => "null".to_string(),
            Value::Null => "null".to_string(),
            Value::Bool(b) => b.to_string(),
            Value::Int(i) => i.to_string(),
            Value::Float(f) => {
                // Handle special float values
                if f.is_nan() || f.is_infinite() {
                    "null".to_string()
                } else {
                    let s = f.to_string();
                    // Ensure float doesn't get serialized as integer
                    if s.contains('.') || s.contains('e') || s.contains('E') {
                        s
                    } else {
                        format!("{}.0", s)
                    }
                }
            }
            Value::Double(d) => {
                // Handle special double values
                if d.is_nan() || d.is_infinite() {
                    "null".to_string()
                } else {
                    let s = d.to_string();
                    // Ensure double doesn't get serialized as integer
                    if s.contains('.') || s.contains('e') || s.contains('E') {
                        s
                    } else {
                        format!("{}.0", s)
                    }
                }
            }
            Value::String(s) => format!("\"{}\"", escape_json_string(s)),
            Value::Bytes(b) => {
                let base64 = general_purpose::STANDARD.encode(b);
                format!("\"{}\"", escape_json_string(&base64))
            }
            Value::Array(arr) => {
                let items: Vec<String> = arr.iter().map(|v| v.to_json_internal()).collect();
                format!("[{}]", items.join(","))
            }
            Value::Object(obj) => {
                let items: Vec<String> = obj
                    .iter()
                    .map(|(k, v)| format!("\"{}\":{}", escape_json_string(k), v.to_json_internal()))
                    .collect();
                format!("{{{}}}", items.join(","))
            }
        }
    }

    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(json)
    }
}

use serde_cbor::error::Error as CborError;
use serde_cbor::{Deserializer as CborDeserializer, Serializer as CborSerializer};

fn encode_unsigned(n: u64, major: u8, buf: &mut Vec<u8>) {
    if n < 24 {
        buf.push(major | (n as u8));
    } else if n < 256 {
        buf.push(major | 24);
        buf.push(n as u8);
    } else if n < 65536 {
        buf.push(major | 25);
        buf.extend_from_slice(&(n as u16).to_be_bytes());
    } else if n < 4294967296 {
        buf.push(major | 26);
        buf.extend_from_slice(&(n as u32).to_be_bytes());
    } else {
        buf.push(major | 27);
        buf.extend_from_slice(&n.to_be_bytes());
    }
}

fn encode_value_to_cbor(value: &Value, buf: &mut Vec<u8>) {
    match value {
        Value::Null | Value::Undefined => buf.push(0xf6), // CBOR null
        Value::Bool(true) => buf.push(0xf5),
        Value::Bool(false) => buf.push(0xf4),
        Value::Int(i) => {
            if *i >= 0 {
                encode_unsigned(*i as u64, 0x00, buf);
            } else {
                encode_unsigned((-1 - *i) as u64, 0x20, buf);
            }
        }
        Value::Float(f) => {
            buf.push(0xfa);
            buf.extend_from_slice(&f.to_bits().to_be_bytes());
        }
        Value::Double(d) => {
            buf.push(0xfb);
            buf.extend_from_slice(&d.to_bits().to_be_bytes());
        }
        Value::String(s) => {
            encode_unsigned(s.len() as u64, 0x60, buf);
            buf.extend_from_slice(s.as_bytes());
        }
        Value::Bytes(b) => {
            encode_unsigned(b.len() as u64, 0x40, buf);
            buf.extend_from_slice(b);
        }
        Value::Array(arr) => {
            encode_unsigned(arr.len() as u64, 0x80, buf);
            for v in arr {
                encode_value_to_cbor(v, buf);
            }
        }
        Value::Object(obj) => {
            encode_unsigned(obj.len() as u64, 0xa0, buf);
            for (k, v) in obj {
                encode_unsigned(k.len() as u64, 0x60, buf);
                buf.extend_from_slice(k.as_bytes());
                encode_value_to_cbor(v, buf);
            }
        }
    }
}

impl Value {
    pub fn to_cbor(&self) -> Vec<u8> {
        let mut buf = Vec::new();
        encode_value_to_cbor(self, &mut buf);
        buf
    }

    pub fn from_cbor(data: &[u8]) -> Result<Self, CborError> {
        let mut deserializer = CborDeserializer::from_slice(data);
        Deserialize::deserialize(&mut deserializer)
    }
}

pub fn tester() {
    // Create a complex value
    let mut value = Value::Object(IndexMap::new());
    if let Value::Object(ref mut map) = value {
        map.insert("name".to_string(), Value::string("John Doe"));
        map.insert("age".to_string(), Value::int(42));
        map.insert("active".to_string(), Value::bool(true));

        let mut address = Value::Object(IndexMap::new());
        if let Value::Object(ref mut addr_map) = address {
            addr_map.insert("street".to_string(), Value::string("123 Main St"));
            addr_map.insert("city".to_string(), Value::string("Anytown"));
        }
        map.insert("address".to_string(), address);
        let mut v = Value::array();
        v += Value::int(95);
        v += Value::int(87);
        v += Value::int(91);
        map.insert("scores".to_string(), v);

        map.insert(
            "binary".to_string(),
            Value::bytes(vec![0x01, 0x02, 0x03, 0xFF]),
        );
    }

    // Serialize to CBOR
    let cbor = value.to_cbor();
    println!("CBOR size: {} bytes", cbor.len());

    // Deserialize from CBOR
    let parsed_from_cbor = Value::from_cbor(&cbor).unwrap();
    println!("Parsed from CBOR: {}", parsed_from_cbor);
}

use std::ops::{Index, IndexMut};

// For immutable indexing
impl Index<&str> for Value {
    type Output = Value;

    fn index(&self, index: &str) -> &Self::Output {
        match self {
            Value::Object(map) => map.get(index).unwrap_or(&Value::Undefined),
            _ => &Value::Undefined,
        }
    }
}

impl Index<usize> for Value {
    type Output = Value;

    fn index(&self, index: usize) -> &Self::Output {
        match self {
            Value::Array(vec) => &vec[index],
            _ => &Value::Undefined,
        }
    }
}

// For mutable indexing
impl IndexMut<&str> for Value {
    fn index_mut(&mut self, index: &str) -> &mut Self::Output {
        if self.is_null() {
            *self = Value::Object(IndexMap::new());
        }

        match self {
            Value::Object(map) => {
                if !map.contains_key(index) {
                    map.insert(index.to_string(), Value::default());
                }
                map.get_mut(index).unwrap()
            }
            _ => panic!("Cannot index non-object with &str"),
        }
    }
}

// permit += value for a Value Array

use std::ops::AddAssign;

impl AddAssign<Value> for Value {
    fn add_assign(&mut self, rhs: Value) {
        match self {
            Value::Array(arr) => {
                arr.push(rhs);
            }
            _ => panic!("+= operation only supported for Value::Array"),
        }
    }
}

impl AddAssign<&Value> for Value {
    fn add_assign(&mut self, rhs: &Value) {
        match self {
            Value::Array(arr) => {
                arr.push(rhs.clone());
            }
            _ => panic!("+= operation only supported for Value::Array"),
        }
    }
}
pub fn tester2() {
    // Create an object
    let mut obj = Value::object();
    obj["name"] = "Alice".into();
    obj["age"] = 30.into();
    obj["pi"] = 3.14.into();

    // Create an array
    let mut arr = Value::from_array(vec![Value::int(1), Value::int(2), Value::int(3)]);

    // Access values
    println!("Name: {}", obj["name"].as_string().unwrap());
    println!("PI: {}", obj["pi"].as_float().unwrap());

    println!("First element: {}", arr[0].as_int().unwrap());

    // Modify values
    arr += Value::int(20);
    obj["age"] = Value::int(31);

    // This will panic:
    // let invalid = obj["invalid_key"]; // Key not found
    // let invalid = arr["string"]; // Wrong index type
}

impl Value {
    pub fn get(&self, key: &str) -> Option<&Value> {
        match self {
            Value::Object(map) => map.get(key),
            _ => None,
        }
    }

    pub fn get_mut(&mut self, key: &str) -> Option<&mut Value> {
        match self {
            Value::Object(map) => map.get_mut(key),
            _ => None,
        }
    }

    pub fn get_index(&self, index: usize) -> Option<&Value> {
        match self {
            Value::Array(vec) => vec.get(index),
            _ => None,
        }
    }

    pub fn get_index_mut(&mut self, index: usize) -> Option<&mut Value> {
        match self {
            Value::Array(vec) => vec.get_mut(index),
            _ => None,
        }
    }
    pub fn set(&mut self, key: &str, value: Value) {
        if let Value::Object(map) = self {
            map.insert(key.to_string(), value);
        } else {
            error!("Cannot set key on non-object value");
        }
    }

    pub fn push(&mut self, value: Value) {
        if let Value::Array(vec) = self {
            vec.push(value);
        } else {
            error!("Cannot push to non-array value");
        }
    }
}
