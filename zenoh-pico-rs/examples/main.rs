use std::sync::Arc;
use zenoh_pico_rs::prelude::*;
use esp_idf_sys as _;

fn main() -> anyhow::Result<()> {
    esp_idf_sys::link_patches();

    let sess = SessionBuilder::new()?
        .mode_peer()
        .add_listen("udp/224.0.0.224:7447")?
        .add_connect("udp/224.0.0.224:7447")?
        .build()?;

    let key = KeyExpr::try_from_str("esp/demo")?;
    let pubr = Publisher::declare(&sess, &key)?;
    let _sub = Subscriber::declare(&sess, &key, Arc::new(|data| {
        println!("<< received: {}", String::from_utf8_lossy(data));
    }))?;

    loop {
        pubr.put(b"Hello from ESP-IDF peer!")?;
        sess.poll()?;
        esp_idf_hal::delay::FreeRtos::delay_ms(100);
    }
}
