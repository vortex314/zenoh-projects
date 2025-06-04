
#[derive(Debug, Eq, Hash, PartialEq)]
struct PropertyName {
    device : String,
    object : String,
    property : String,
}



// You can replace serde_json::Value with another type if needed
#[derive(Debug, Clone, Serialize, Deserialize)]
enum Value {
    String(String),
    Number(f64),
    Boolean(bool),
    Vector(Vec<Value>),
    Map(HashMap<String, Value>),
    Null,
}
use std::collections::HashMap;
use std::sync::{Arc, Mutex};

use serde::{Deserialize, Serialize};

struct PropertyCache {
    cache: HashMap<PropertyName, Value>,
}

impl PropertyCache {
    fn new() -> Self {
        PropertyCache {
            cache: HashMap::new(),
        }
    }

    fn get(&self, name: &PropertyName) -> Option<Value> {
        self.cache.get(name).cloned()
    }

    fn set(&mut self, name: PropertyName, value: Value) {
        self.cache.insert(name, value);
    }

    fn remove(&mut self, name: &PropertyName) {
        self.cache.remove(name);
    }

    fn pattern_match<F>(&self, pattern: &str, mut callback: F)
    where
        F: FnMut(&PropertyName, &Value),
    {
        for (name, value) in &self.cache {
            if name.device.contains(pattern)
                || name.object.contains(pattern)
                || name.property.contains(pattern)
            {
                callback(name, value);
            }
        }
    }
}