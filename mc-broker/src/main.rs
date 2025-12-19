use minicbor::{Decode, Encode};
use std::error::Error;
use std::net::SocketAddr;
use std::time::Instant;
use tokio::net::UdpSocket;

mod logger;
use log::info;
use log::error;

const NUM_MESSAGES: usize = 1000;

#[derive(Debug, Encode, Decode)]
#[cbor(map)]
struct UdpPayload {
    #[n(0)]
    dst: String,
    #[n(1)]
    src: String,
    #[n(2)]
    message_type: String,
    #[n(3)]
    payload: Vec<u8>,
}

#[derive(Debug, Encode, Decode)]
enum Message {
    #[n(0)]
    Alive,
    #[n(1)]
    Subscribe {
        #[n(0)]
        dst_pattern: String,
        #[n(1)]
        src_pattern: String,
        #[n(2)]
        message_type: String,
    },
    #[n(2)]
    Unsubscribe {
        #[n(0)]
        dst_pattern: String,
        #[n(1)]
        src_pattern: String,
        #[n(2)]
        message_type: String,
    },
    #[n(3)  ]
    Ping(#[n(0)] u32),
    #[n(4)  ]
    Pong(#[n(0)] u32),
}

async fn udp_sender(
    send_addr: SocketAddr,
    target_addr: SocketAddr,
) -> Result<(), Box<dyn Error + Send + Sync>> {
    let socket = UdpSocket::bind(send_addr).await?;
    socket.connect(target_addr).await?;
    info!("Sender connected to {}", target_addr);

    let mut buf = [0; 1024];
    let start = Instant::now();

    for i in 0..NUM_MESSAGES {
        let message = UdpPayload{ 
            dst: "dst".to_string(),
            src: "src".to_string(),
            message_type: "type".to_string(),
            payload: minicbor::to_vec(&Message::Ping(i as u32)).map_err(|e| e.to_string())?,
        };
        let encoded = minicbor::to_vec(message).map_err(|e| e.to_string())?;
        socket.send(&encoded).await?;
        let message = Message::Ping(i as u32);
        let encoded = minicbor::to_vec(message).map_err(|e| e.to_string())?;
        socket.send(&encoded).await?;

        let (len, _) = socket.recv_from(&mut buf).await?;
        let response: Message = minicbor::decode(&buf[..len]).map_err(|e| e.to_string())?;

        if let Message::Pong(counter) = response {
            if counter != i as u32 {
                error!(
                    "Received wrong counter. Expected {}, got {}",
                    i, counter
                );
            }
        } else {
            error!("Received invalid response: {:?}", response);
        }
    }

    let duration = start.elapsed();
    info!(
        "Sent and received {} messages in {:?}",
        NUM_MESSAGES, duration
    );
    info!(
        "Average time per message: {:?}",
        duration / NUM_MESSAGES as u32
    );

    Ok(())
}

async fn udp_receiver(listen_addr: SocketAddr) -> Result<(), Box<dyn Error + Send + Sync>> {
    let socket = UdpSocket::bind(listen_addr).await?;
    info!("Receiver listening on {}", listen_addr);
    let mut buf = [0; 1024];
    loop {
        info!("Waiting for messages...");
        let (len, addr) = socket.recv_from(&mut buf).await?;
        info!("Received {}", minicbor::display(&buf[..len]));
        info!("From address: {}", addr);
        let result = minicbor::decode::<Message>(&buf[..len]);
        if let Ok(message) = &result {
        if let Message::Ping(counter) = message {
            let response = Message::Pong(*counter);
            let encoded = minicbor::to_vec(response).map_err(|e| e.to_string())?;
            socket.send_to(&encoded, addr).await?;
        }        } else {
            error!("Failed to decode message: {}", result.unwrap_err());
            continue;
        }

    }
    Ok(())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error + Send + Sync>> {
    logger::init();
    let sender_addr = "127.0.0.1:34254".parse::<SocketAddr>()?;
    let receiver_addr = "127.0.0.1:34255".parse::<SocketAddr>()?;

    let receiver_handle = tokio::spawn(udp_receiver(receiver_addr));
    // Give the receiver a moment to start up
    tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;
    let sender_handle = tokio::spawn(udp_sender(sender_addr, receiver_addr));

    sender_handle.await??;
    receiver_handle.await??;

    Ok(())
}

