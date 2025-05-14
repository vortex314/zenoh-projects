use anyhow::Result;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};

use crate::value::Value;

pub struct FieldInfo {
    pub idx: usize,
    pub name: String,
    pub desc: String,
    pub last_value: String,
}

fn create_shared() {
    unsafe {
        SHARED = Some(Arc::new(Mutex::new(Shared {
            values: HashMap::new(),
        })));
    }
}

pub fn on_shared(f: impl FnOnce(&mut Shared)) {
    unsafe {
        if SHARED.is_none() {
            create_shared();
        };
        SHARED.as_ref().map(|shared| {
            if let Ok(mut guard) = shared.lock() {
                f(&mut *guard);
            }
        });
    }
}
pub struct Shared {
    pub values: HashMap<String, Value>,
}

pub fn get_possible_endpoints() -> Vec<String> {
    let mut endpoints = Vec::new();
    on_shared(|shared| {
        for (topic, _field) in shared.values.iter() {
            endpoints.push(topic.clone());
        }
    });
    endpoints
}

pub fn update_with_value(topic: &String, value: &Value) {
    update_single_value(topic, value);
    match value {
        Value::MapStr(map) => {
            for (field_name, field_value) in map.iter() {
                update_single_value(
                    &format!("{}.{}", topic, field_name).to_string(),
                    field_value,
                );
            }
        }
        Value::MapIdx(map) => {
            for (field_name, field_value) in map.iter() {
                update_single_value(
                    &format!("{}.{}", topic, field_name).to_string(),
                    field_value,
                );
            }
        }
        Value::List(list) => {
            for (i, field_value) in list.iter().enumerate() {
                update_single_value(&format!("{}.{}", topic, i).to_string(), field_value);
            }
        }

        _ => {}
    }
}

pub fn update_single_value(topic: &String, value: &Value) {
    on_shared(|shared| {
        if shared.values.get(topic).is_none() {
            shared.values.insert(topic.clone(), value.clone());
        }
        shared.values.get_mut(topic).map(|field| {
            *field = value.clone();
        });
    });
}

pub static mut SHARED: Option<Arc<Mutex<Shared>>> = None;
