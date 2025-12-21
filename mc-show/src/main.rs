#[allow(unused_imports)]
#[allow(unused)]
#[allow(dead_code)]
use std::sync::Arc;
use std::time::Duration;
use udp_cbor_async::UdpCborClient;
mod udp_cbor_async;
use log::info;
use tokio;

use crate::{limero::{Announce, Msg, SysCmd}, udp_cbor_async::SendMessage};

mod limero;
mod logger;

fn handle<T>(message_type: &String, bytes: &Vec<u8>, f: impl FnOnce(T) + Send + 'static)
where
    T: serde::de::DeserializeOwned + Msg + Send + 'static,
{
    if message_type == T::NAME {
        let msg = serde_json::from_slice::<T>(&bytes);
        match msg {
            Ok(m) => {
                f(m);
            }
            Err(e) => {
                info!("Deserialize error for type {} : {:?}", T::NAME, e);
            }
        }
    }
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    logger::init();
    let mut client = UdpCborClient::bind(
        "0.0.0.0:50001",   //unicast
        "224.0.0.1:50000", // multicast (valid IPv4 multicast address)
        "node-a".to_string(),
    )
    .await?;

    let sender = client.sender();
    let sender_cb = sender.clone();

    let f = Arc::new( move |msg: Vec<u8>, adr: std::net::SocketAddr| {
        let udp_cbor = minicbor::decode::<udp_cbor_async::CborMessage>(&msg);
        match udp_cbor {
            Ok(cbor_msg) => {
                let msg_type = cbor_msg.message_type.clone();
                let payload = cbor_msg.payload.as_ref().unwrap().clone();
                // Pre-clone sender so the outer callback remains Fn (not FnOnce)
                let sender_for_ping = sender_cb.clone();
                handle::<limero::Ping>(&msg_type, &payload, move |ping_msg| {
                    info!("Received Ping from {:?} ", adr);
                    let pong = limero::Pong {
                        number: ping_msg.number,
                    };
                    let udp_msg = udp_cbor_async::CborMessage {
                        destination: Some(adr.to_string()),
                        source: Some("node-a".to_string()),
                        message_type: limero::Pong::NAME.to_string(),
                        payload: Some(serde_json::to_vec(&pong).unwrap()),
                    };
                    let msg = SendMessage::Unicast(adr, udp_msg);
                    info!("Sending Pong to {:?} ", adr);
                    let _ = sender_for_ping.try_send(msg);
                });
                handle::<limero::SysEvent>(&msg_type, &payload, move |sys_event| {
                    info!("Received SysEvent from {:?} ", adr);
                });
                handle::<limero::WifiEvent>(&msg_type, &payload, move |wifi_event| {
                    info!("Received WifiEvent from {:?} ", adr);
                });
                /*
                if cbor_msg.message_type == "Ping" {
                    // process status message
                    serde_json::from_slice(cbor_msg.payload.as_ref().unwrap())
                        .map::<(), _>(|ping_msg: limero::Ping| ())
                        .unwrap_or(());
                } else if cbor_msg.message_type == "SysEvent" {
                    // process command message
                    serde_json::from_slice(cbor_msg.payload.as_ref().unwrap())
                        .map::<(), _>(|pong_msg: limero::SysEvent| ())
                        .unwrap_or(());
                } else if cbor_msg.message_type == "WifiEvent" {
                    // process command message
                    serde_json::from_slice(cbor_msg.payload.as_ref().unwrap())
                        .map::<(), _>(|pong_msg: limero::WifiEvent| ())
                        .unwrap_or(());
                } else {
                    info!(
                        "Unknown message type from {:?} {:?}",
                        adr, cbor_msg.message_type
                    );
                }*/
            }
            Err(e) => {
                info!("CBOR decode error from {:?} {:?} ", adr, e);
            }
        }
    });

    client.set_callback(f);

    client.register_message_type(Announce::NAME);
    client.register_message_type(SysCmd::NAME);
    client.announce_task(Duration::from_secs(5)).await;
    info!("Client started =========================================> ");
    let client_1 = Arc::new(client);
    let client_2 = client_1.clone();

    tokio::spawn({
        async move {
            client_2.announce_task(Duration::from_secs(5)).await;
        }
    });

    loop {
        tokio::time::sleep(Duration::from_secs(60)).await;
    }
}
