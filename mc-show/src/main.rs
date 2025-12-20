use std::sync::Arc;
use std::time::Duration;
use udp_cbor_async::UdpCborClient;
mod udp_cbor_async;
use log::info;
use tokio;

mod logger;
mod limero;

#[tokio::main]
async fn main() -> std::io::Result<()> {
    logger::init();
    let mut client = UdpCborClient::bind(
        "0.0.0.0:50001",   //unicast
        "224.0.0.1:50000", // multicast (valid IPv4 multicast address)
        "node-a".to_string(),
    )
    .await?;

    let f = Arc::new(|msg:Vec<u8>, adr| {
        let udp_cbor = minicbor::decode::<udp_cbor_async::CborMessage>(&msg);
        match udp_cbor {
            Ok(cbor_msg) => {
                info!("Callback from {:?} {:?}", adr, cbor_msg.message_type);
                if cbor_msg.message_type == "Ping" {
                    // process status message
                    serde_json::from_slice(cbor_msg.payload.as_ref().unwrap()).map::<(), _>(|ping_msg: limero::Ping| {
                        info!(" {:?} => {:?}", adr, ping_msg);
                        ()
                    }).unwrap_or(());
                }
            }
            Err(e) => {
                info!("CBOR decode error from {:?} {:?} ", adr, e);
            }
        }
        info!("Callback from {:?} [{}]", adr, msg.len());
    });

    client.set_callback(f);


    client.register_message_type("command");
    client.register_message_type("status");
    client.announce_task(Duration::from_secs(5)).await;
    let mut client_1 = Arc::new(client);
    let client_2 = client_1.clone();


    tokio::spawn({
        async move {
            client_2
                .announce_task(Duration::from_secs(5))
                .await;
        }
    });

    loop {
        tokio::time::sleep(Duration::from_secs(60)).await;
    }
}
