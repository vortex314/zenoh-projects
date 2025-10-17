use serde::{Deserialize, Serialize};



#[derive(Serialize, Deserialize, Debug)]
pub struct ReturnCode {
    pub code: u16,
    pub msg : String,
}

// Define the Publish structure
#[derive(Serialize, Deserialize, Debug)]
pub struct Publish {
    pub topic: String,
    pub payload: serde_json::Value,
}

// Define the Subscribe structure
#[derive(Serialize, Deserialize, Debug)]
pub struct Subscribe {
    pub topic: String,
}

// Define the Save structure
#[derive(Serialize, Deserialize, Debug)]
pub struct Save {
    pub key: String,
    pub payload : serde_json::Value,
}
#[derive(Serialize, Deserialize, Debug)]
pub struct SaveReply {
    pub key: String,
    pub rc : ReturnCode,
}

// Define the Load structure
#[derive(Serialize, Deserialize, Debug)]
pub struct Load {
    pub key: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct LoadReply {
    pub key: String,
    pub payload : serde_json::Value,
    pub rc : ReturnCode,
}



// Define the List structure
#[derive(Serialize, Deserialize, Debug)]
pub struct List {
    pub prefix: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct ListReply {
    pub prefix: String,
    pub keys: Vec<String>,
    pub rc: ReturnCode,
}

// Define the Message enum to support multiple types
#[derive(Serialize, Deserialize, Debug)]
#[serde(tag = "type")] // Use "type" field to differentiate message types
pub enum Message {
    Publish(Publish),
    Subscribe(Subscribe),
    Save(Save),
    SaveReply(SaveReply),
    Load(Load),
    LoadReply(LoadReply),
    List(List),
    ListReply(ListReply),
}

pub fn serialize_message(msg: &Message) -> Result<String, serde_json::Error> {
    serde_json::to_string(msg)
}

pub fn deserialize_message(json_str: &str) -> Result<Message, serde_json::Error> {
    serde_json::from_str(json_str)
}

pub fn test_serialization() {
    let publish_msg = Message::Publish(Publish {
        topic: "example/topic".to_string(),
        payload: serde_json::json!({"key": "value"}),
    });

    let serialized = serialize_message(&publish_msg).unwrap();
    println!("Serialized Publish Message: {}", serialized);

    let deserialized: Message = deserialize_message(&serialized).unwrap();
    println!("Deserialized Publish Message: {:?}", deserialized);
}