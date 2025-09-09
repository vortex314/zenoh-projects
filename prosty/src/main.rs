
mod message {
    include!(concat!(env!("OUT_DIR"), "/_.rs"));
}

use message::HoverboardCmd;
use prost::Message;

fn main() {
    // --- Decode received bytes (from MCU) ---


    // --- Encode to send back ---
    let reply = HoverboardCmd {
        speed: Some(7),
        turn: Some(220000),
    };
    let mut buf = Vec::new();
//   reply.encode(&mut buf).unwrap();
    reply.encode(&mut buf).unwrap();
    println!("Rust encoded {} bytes to send back {:02X?}", buf.len(), buf);
}
