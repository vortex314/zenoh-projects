use alloc::string::String;
use alloc::vec::Vec;

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