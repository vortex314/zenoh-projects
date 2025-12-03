mod zenoh_pico;
use anyhow::Result;
use zenoh_pico::prelude::*;
use esp_idf_sys as _; // Ensure the ESP-IDF runtime is linked



fn main() -> anyhow::Result<()> {
    // Initialize ESP-IDF runtime (esp-idf-sys)
    esp_idf_sys::link_patches();

    let sess = SessionBuilder::new()?
        .mode_peer()
        .add_listen("udp/224.0.0.224:7447")? // multicast listen
        .add_connect("udp/224.0.0.224:7447")? // multicast connect
        .build()?;

    let key = KeyExpr::try_from_str("esp/demo")?;
    let pubr = Publisher::declare(&sess, &key)?;

    loop {
        pubr.put(b"Hello from ESP-IDF peer!")?;
        sess.poll()?; // drive I/O
        esp_idf_hal::delay::FreeRtos::delay_ms(100);
    }
}