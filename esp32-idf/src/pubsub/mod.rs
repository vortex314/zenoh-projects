pub mod payload;
pub use payload::payload_decode as payload_decode;
pub use payload::payload_display as payload_display;
pub use payload::payload_encode as payload_encode;

pub mod payload_json;
pub use payload_json::payload_as_f64_json;
pub use payload_json::payload_decode_json;
pub use payload_json::payload_display_json;
pub use payload_json::payload_encode_json;




// =============== pubsub interface actor ===============    

#[derive(Clone, Debug)]
pub enum PubSubCmd {
    Publish { topic: String, payload : Vec<u8> },
    Subscribe { topic: String },
    Unsubscribe { topic: String },
    Connect { client_id: String },
    Disconnect,
}
#[derive(Clone, Debug)]
pub enum PubSubEvent {
    Publish { topic: String, payload: Vec<u8> },
    Connected,
    Disconnected,
}