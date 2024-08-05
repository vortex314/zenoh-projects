pub mod payload;
use alloc::string::String;
use alloc::vec::Vec;
pub use payload::payload_decode as payload_decode;
pub use payload::payload_display as payload_display;
pub use payload::payload_encode as payload_encode;



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