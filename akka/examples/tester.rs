use clap::Parser;
use local_ip_address::local_ip;
use serde::{Deserialize, Serialize};
use serde_json;
use std::net::{Ipv4Addr, SocketAddrV4};
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::net::UdpSocket;
use tokio::time::{Duration, interval};
use log::info;

// import necessary modules

use akka::logger::{self, init};
use akka::Value;

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

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli {
    /// Optional name to operate on
    ///     #[arg(short, long, value_name = "FILE")]
    #[arg(short, long, default_value = "6504")]
    udp_port: Option<u16>,

}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    // Parse command line arguments
    logger::init();
    let cli = Cli::parse();
    let udp_port = cli.udp_port.unwrap_or(TESTER_PORT);
    info!("UDP Port: {}", udp_port);
    // Initialize UDP socket
    let socket = UdpSocket::bind(format!("0.0.0.0:{}", udp_port)).await?;
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
    info!("Subscribed to esp1/sys");

    // Spawn task for sending multicast announcements
    let socket_clone = socket.clone();
    tokio::spawn(async move {
        let mut interval = interval(Duration::from_secs(5));
        loop {
            interval.tick().await;
            let mut announce = Value::object();
            announce["src"] = Value::string(DEVICE_NAME);
            announce["type"] = Value::string("device");
            announce["ip"] = Value::string(local_ip().unwrap().to_string());
            announce["port"] = Value::int(udp_port as i64);
            announce["sub"] = Value::object();
            announce["sub"]["pattern"] = Value::array();
            announce["sub"]["pattern"]+= Value::string("esp1/sys");

            info!("MC  {}", announce.to_json());
            // Create and send multicast announcement
            let announce_bytes = announce.to_json().into_bytes();

            let mcast_addr =
                SocketAddrV4::new(MULTICAST_IP.parse::<Ipv4Addr>().unwrap(), MULTICAST_PORT);
            socket_clone
                .send_to(&announce_bytes, mcast_addr)
                .await
                .unwrap();
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
            let mut v = Value::object();
            v["src"] = Value::string(DEVICE_NAME);
            let mut p = Value::object();
            p["uptime"] = Value::int(uptime as i64);
            p["cpu"] = Value::string("x86_64");
            v["pub"] = p;
            info!("PUB {}", v.to_json());
            socket_clone.send_to(&v.to_json().as_bytes(), broker_addr).await.unwrap();
        }
    });

    // Main loop for receiving messages
    let mut buf = [0u8; 1500];
    loop {
        let (len, src) = socket.recv_from(&mut buf).await?;
        let data = &buf[..len];
        let str : &str = std::str::from_utf8(data).unwrap_or("");
        info!("Received from {}: {}", src, str);
        match serde_json::from_slice::<Message>(data) {
            Ok(Message::Publish(pub_msg)) => {
                info!(
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
                        info!("Sent response: rebooting");
                    }
                }
            }
            Ok(_) => {} // Ignore other messages
            Err(e) => info!("Failed to parse message: {}", e),
        }
    }
}
