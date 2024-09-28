extern crate alloc;

mod framer;
pub use framer::encode_frame;
pub use framer::FrameExtractor;

pub mod esp_now;
pub use esp_now::EspNowHeader as EspNowHeader;
pub use esp_now::EspNowMessage as EspNowMessage;
pub use esp_now::Log;
pub use esp_now::LogLevel;

pub mod pubsub;
pub use pubsub::PubSubCmd as PubSubCmd;
pub use pubsub::PubSubEvent as PubSubEvent;

pub mod cbor;
pub mod json;
#[cfg(feature = "cbor")]
pub use cbor::decode as payload_decode;
#[cfg(feature = "cbor")]
pub use cbor::encode as payload_encode;
#[cfg(feature = "cbor")]
pub use cbor::to_string as payload_display;
#[cfg(feature = "cbor")]
pub use cbor::as_f64 as payload_as_f64;
#[cfg(feature = "json")]
pub use json::decode as payload_decode;
#[cfg(feature = "json")]
pub use json::encode as payload_encode;
#[cfg(feature = "json")]
pub use json::to_string as payload_display;
#[cfg(feature = "json")]
pub use json::as_f64 as payload_as_f64;


