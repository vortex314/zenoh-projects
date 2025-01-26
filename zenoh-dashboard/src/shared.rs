use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use anyhow::Result;

pub struct FieldInfo {
    pub name: String,
    pub desc: String,
}

fn get_shared() {
    unsafe {
        SHARED = Some(Arc::new(Mutex::new(Shared {
            registry: HashMap::new(),
        })));
    }
}

pub fn on_shared(f: impl FnOnce(&mut Shared))  {
    unsafe {
        if SHARED.is_none() {
            get_shared();
        };
        SHARED.as_ref().map(|shared| {
            if let Ok(mut guard) = shared.lock() {
                f(&mut *guard);
            }
        });
    }
}
pub struct Shared {
    pub registry: HashMap<String, FieldInfo>,
}

pub fn possible_topics () -> Vec<String> {
    let mut topics = Vec::new();
    on_shared(|shared| {
        for (topic, _field) in shared.registry.iter() {
            topics.push(topic.clone());
        }
    });
    topics
}

pub static mut SHARED: Option<Arc<Mutex<Shared>>> = None;
