use local_ip_address::local_ip;
use serde::{Deserialize, Serialize};
use serde_json;
use std::net::{Ipv4Addr, SocketAddrV4};
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::net::UdpSocket;
use tokio::time::{Duration, interval};

// Configuration
const DEVICE_NAME: &str = "pclenovo/tester";
const BROKER_IP: &str = "192.168.0.148";
const BROKER_PORT: u16 = 6503;
const MULTICAST_IP: &str = "225.0.0.1";
const MULTICAST_PORT: u16 = 6502;
const TESTER_PORT: u16 = 6504;

// Message types
#[derive(Serialize, Deserialize, Debug)]
struct MulticastAnnounce {
    src: String,
    #[serde(rename = "type")]
    msg_type: String,
    ip: String,
    port: u16,
}

#[derive(Serialize, Deserialize, Debug)]
struct PublishMessage {
    src: String,
    #[serde(rename = "pub")]
    pub_data: serde_json::Value,
}

#[derive(Serialize, Deserialize, Debug)]
struct SubscribeMessage {
    dst: String,
    sub: String,
}

#[derive(Serialize, Deserialize, Debug)]
struct RequestMessage {
    dst: String,
    src: String,
    req: serde_json::Value,
}

#[derive(Serialize, Deserialize, Debug)]
struct ResponseMessage {
    dst: String,
    src: String,
    rep: serde_json::Value,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(untagged)]
enum Message {
    Announce(MulticastAnnounce),
    Publish(PublishMessage),
    Subscribe(SubscribeMessage),
    Request(RequestMessage),
    Response(ResponseMessage),
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    // Initialize UDP socket
    let socket = UdpSocket::bind(format!("0.0.0.0:{}", TESTER_PORT)).await?;
    socket.join_multicast_v4(
        MULTICAST_IP.parse::<Ipv4Addr>().unwrap(),
        Ipv4Addr::new(0, 0, 0, 0),
    )?;
    let socket = std::sync::Arc::new(socket);

    // Broker address
    let broker_addr = SocketAddrV4::new(BROKER_IP.parse::<Ipv4Addr>().unwrap(), BROKER_PORT);

    // Subscribe to esp1/sys
    let sub_msg = SubscribeMessage {
        dst: DEVICE_NAME.to_string(),
        sub: "esp1/sys".to_string(),
    };
    let sub_bytes = serde_json::to_vec(&sub_msg)?;
    socket.send_to(&sub_bytes, broker_addr).await?;
    println!("Subscribed to esp1/sys");

    // Spawn task for sending multicast announcements
    let socket_clone = socket.clone();
    tokio::spawn(async move {
        let mut interval = interval(Duration::from_secs(5));
        loop {
            interval.tick().await;
            let announce = MulticastAnnounce {
                src: DEVICE_NAME.to_string(),
                msg_type: "device".to_string(),
                ip: local_ip().unwrap().to_string(),
                port: TESTER_PORT,
            };
            let announce_bytes = serde_json::to_vec(&announce).unwrap();
            let mcast_addr =
                SocketAddrV4::new(MULTICAST_IP.parse::<Ipv4Addr>().unwrap(), MULTICAST_PORT);
            socket_clone
                .send_to(&announce_bytes, mcast_addr)
                .await
                .unwrap();
            println!("Sent multicast announcement");
        }
    });

    // Spawn task for publishing system info
    let socket_clone = socket.clone();
    tokio::spawn(async move {
        let mut interval = interval(Duration::from_secs(1));
        loop {
            interval.tick().await;
            let uptime = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap()
                .as_secs();
            let pub_data = serde_json::json!({
                "free_heap": 524288, // Simulated
                "uptime": uptime,
                "cpu": "x86_64"
            });
            let pub_msg = PublishMessage {
                src: DEVICE_NAME.to_string(),
                pub_data,
            };
            let pub_bytes = serde_json::to_vec(&pub_msg).unwrap();
            socket_clone.send_to(&pub_bytes, broker_addr).await.unwrap();
            println!("Published system info");
        }
    });

    // Main loop for receiving messages
    let mut buf = [0u8; 1500];
    loop {
        let (len, src) = socket.recv_from(&mut buf).await?;
        let data = &buf[..len];
        match serde_json::from_slice::<Message>(data) {
            Ok(Message::Publish(pub_msg)) => {
                println!(
                    "Received publish from {}: {}",
                    pub_msg.src, pub_msg.pub_data
                );
            }
            Ok(Message::Request(req_msg)) if req_msg.dst == DEVICE_NAME => {
                // Handle request (e.g., reset)
                if let Some(action) = req_msg.req.get("action") {
                    if action == "reset" {
                        let rep_data = serde_json::json!({ "action": "rebooting" });
                        let rep_msg = ResponseMessage {
                            dst: req_msg.src,
                            src: DEVICE_NAME.to_string(),
                            rep: rep_data,
                        };
                        let rep_bytes = serde_json::to_vec(&rep_msg)?;
                        socket.send_to(&rep_bytes, src).await?;
                        println!("Sent response: rebooting");
                    }
                }
            }
            Ok(_) => {} // Ignore other messages
            Err(e) => println!("Failed to parse message: {}", e),
        }
    }
}
