use anyhow::Context;
use anyhow::Error;
use anyhow::Result;
use hashbrown::HashMap;
use log::debug;
use log::error;
use log::info;
use minicbor::data::Token;
use minicbor::data::Type;
use minicbor::decode::info;
use minicbor::Decode;
use minicbor::Decoder;
use minicbor::Encode;
use msg::InfoProp;
use msg::InfoTopic;
use msg::Msg;
use serde::ser::SerializeMap;
use serde::ser::SerializeSeq;
use serde::Serialize;
use serde::Serializer;
use serde_json_core_fmt::ser::to_fmt;

#[derive(Debug)]
enum Value {
    U8(u8),
    U32(u32),
    F32(f32),
    String(String),
    Array(Vec<Value>),
    Simple(u8),
    Bool(bool),
    Null,
    Undefined,
}

impl Serialize for Value {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        match self {
            Value::U8(v) => serializer.serialize_u8(*v),
            Value::U32(v) => serializer.serialize_u32(*v),
            Value::F32(v) => serializer.serialize_f32(*v),
            Value::String(v) => serializer.serialize_str(v),
            Value::Array(v) => {
                let mut seq = serializer.serialize_seq(Some(v.len()))?;
                for e in v {
                    seq.serialize_element(e)?;
                }
                seq.end()
            }
            Value::Simple(v) => serializer.serialize_u8(*v),
            Value::Bool(v) => serializer.serialize_bool(*v),
            Value::Null => serializer.serialize_none(),
            Value::Undefined => serializer.serialize_none(),
        }
    }
}

#[derive(Debug)]
struct JsonObject {
    map: HashMap<String, Value>,
}

impl JsonObject {
    fn new() -> Self {
        JsonObject {
            map: HashMap::new(),
        }
    }
    fn insert(&mut self, key: String, value: Value) {
        self.map.insert(key, value);
    }
}

impl Serialize for JsonObject {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::ser::Serializer,
    {
        let mut map = serializer.serialize_map(Some(self.map.len()))?;
        for (key, value) in self.map.iter() {
            map.serialize_entry(key, value)?;
        }
        map.end()
    }
}

struct TopicInfo {
    id: u32,
    name: String,
    props: HashMap<u8, InfoProp>,
}

pub struct Translator {
    topics: HashMap<u32, TopicInfo>,
}

impl Translator {
    pub fn new() -> Self {
        Translator {
            topics: HashMap::new(),
        }
    }
    pub fn analyze(&mut self, msg: &Msg) {
        msg.src.map(|src| {
            msg.info_prop.as_ref().map(|info_prop| {
                self.update_property_info(src, info_prop);
            });
            msg.info_topic.as_ref().map(|info_topic| {
                self.update_topic_info(src, info_topic);
            });
        });
    }
    fn update_property_info(&mut self, topic_id: u32, info_prop: &InfoProp) {
        let topic_info = self.topics.entry(topic_id).or_insert(TopicInfo {
            id: topic_id,
            name: "".to_string(),
            props: HashMap::new(),
        });
        match topic_info.props.entry(info_prop.id) {
            hashbrown::hash_map::Entry::Occupied(mut entry) => {
                let mut prop = entry.get_mut();
                prop.name = info_prop.name.clone();
                prop.desc = info_prop.desc.clone();
                prop.prop_mode = info_prop.prop_mode;
                prop.prop_type = info_prop.prop_type;
                prop.id = info_prop.id;// update if name or any other info changes 
            }
            hashbrown::hash_map::Entry::Vacant(entry) => {
                info!("Inserting new property {:?}", info_prop);
                entry.insert(info_prop.clone());
            }
        }
    }
    fn update_topic_info(&mut self, topic_id: u32, info_topic: &InfoTopic) {
        let topic_info = self.topics.entry(topic_id).or_insert(TopicInfo {
            id: topic_id,
            name: "".to_string(),
            props: HashMap::new(),
        });
        if topic_info.name.is_empty() {
            info!("Inserting new topic {:?}", info_topic);
        };
        info_topic
            .name
            .as_ref()
            .map(|name| topic_info.name = name.clone());
    }

    fn find_topic_name(&self, topic_id: u32) -> Result<String> {
        self.topics
            .get(&topic_id)
            .map(|topic_info| topic_info.name.clone())
            .context("Topic not found")
    }

    fn find_prop_name(&self, topic_id: u32, prop_id: u8) -> Option<String> {
        self.topics
            .get(&topic_id)?
            .props
            .get(&prop_id)?
            .name
            .clone()
    }

    pub fn translate_to_object(&self, msg: &Msg) -> Result<(String, String)> {
        let mut topic_str = String::new();
        let mut topic_id = 0;
        msg.src.map(|src| {
            let _ = self.find_topic_name(src).map(|name| {
                topic_str = "src/".to_owned() + &name;
                topic_id = src;
            });
        });
        msg.dst.map(|dst| {
            let _ = self.find_topic_name(dst).map(|name| {
                topic_str = "dst/".to_owned() + &name;
                topic_id = dst;
            });
        });

        let x = msg
            .publish
            .as_ref()
            .map(|bytes| {
                Decoder::new(bytes)
                    .tokens()
                    .collect::<Result<Vec<Token>, _>>()
                    .ok()
            })
            .flatten();
        let mut table: JsonObject = JsonObject {
            map: HashMap::new(),
        };
        x.map(|tokens| {
            for idx in (1..tokens.len() - 1).step_by(2) {
                let key = tokens
                    .get(idx)
                    .map(|token| match token {
                        Token::U8(idx) => self.find_prop_name(topic_id, *idx),
                        _ => None,
                    })
                    .flatten();
                let value = tokens.get(idx + 1).map(|token| match token {
                    Token::U8(idx) => Value::U8(*idx),
                    Token::U32(n) => Value::U32(*n),
                    Token::F32(f) => Value::F32(*f),
                    Token::String(t) => Value::String(t.to_string()),
                    Token::Array(_) => Value::Undefined,
                    Token::Map(_) => Value::Undefined,
                    Token::Tag(_) => Value::Undefined,
                    Token::Simple(_) => Value::Undefined,
                    Token::Bool(b) => Value::Bool(*b),
                    _ => Value::Undefined,
                });
                //   info!("key {:?} value {:?}", key, &value);
                key.map(|key| value.map(|value| table.insert(key.clone(), value)));
            }
            //  info!("table {:?}", &table);
        });
        let mut buffer = String::new();
        to_fmt(&mut buffer, &table)?;
        debug!("buffer {:?}", buffer);
        Ok((topic_str, buffer))
    }


    pub fn translate_to_array(&self, msg: &Msg) -> Result<Vec<(String, Vec<u8>)>> {
        let mut topic_str = String::new();
        let mut topic_id = 0;
        msg.src.map(|src| {
            let _ = self.find_topic_name(src).map(|name| {
                topic_str = "src/".to_owned() + &name;
                topic_id = src;
            });
        });
        msg.dst.map(|dst| {
            let _ = self.find_topic_name(dst).map(|name| {
                topic_str = "dst/".to_owned() + &name;
                topic_id = dst;
            });
        });
        info!("topic_str {:?}", &topic_str);

        let x = msg
            .publish
            .as_ref()
            .map(|bytes| {
                Decoder::new(bytes)
                    .tokens()
                    .collect::<Result<Vec<Token>, _>>()
                    .ok()
            })
            .flatten();

        let mut v = Vec::new();
        x.map(|tokens| {
            for idx in (1..tokens.len() - 1).step_by(2) {
                let key = tokens
                    .get(idx)
                    .map(|token| match token {
                        Token::U8(idx) => self.find_prop_name(topic_id, *idx),
                        _ => None,
                    })
                    .flatten();
                let buffer = tokens.get(idx + 1).map(|token| {
                    let mut buffer = Vec::<u8>::new();
                    let mut e = minicbor::Encoder::new(&mut buffer);
                    let _r = token.encode(&mut e, &mut ()).map_err(|e| error!("encode failure {}",e));
                    buffer
                });
                //   info!("key {:?} value {:?}", key, &value);
                key.as_ref().map(|key| buffer.map(|value| {
                    v.push((topic_str.clone()+"/" +&(key.clone()), value));
                }));
            }
            //  info!("table {:?}", &table);
        });

        Ok(v)
    }
}
