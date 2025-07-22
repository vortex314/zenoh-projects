use actix::Addr;
use actix::Actor;
use akka::McCmd;
use akka::McEvent;
use akka::UdpActor;
use akka::UdpCmd;
use clap::builder::Str;
use clap::Parser;
use local_ip_address::local_ip;
use log::info;
use serde::{Deserialize, Serialize};
use serde_json;
use tokio::net::unix::SocketAddr;
use std::net::{Ipv4Addr, SocketAddrV4};
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::net::UdpSocket;
use tokio::time::{Duration, interval};

// import necessary modules

use akka::{McActor, Value};
use akka::logger::{self, init};

// Configuration
const DEVICE_NAME: &str = "pclenovo/tester";
const BROKER_IP: &str = "192.168.0.148";
const BROKER_PORT: u16 = 6503;
const MULTICAST_IP: &str = "225.0.0.1";
const MULTICAST_PORT: u16 = 6502;
const TESTER_PORT: u16 = 6504;
const TESTER_IP: &str = "0.0.0.0";

struct Orchestrator {
    udp_actor: Addr<UdpActor>,
    mc_actor: Addr<McActor>,
}

impl Actor for Orchestrator {
    type Context = Context<Self>;
}

impl Orchestrator {
    fn new(udp_actor: Addr<UdpActor>, mc_actor: Addr<McActor>) -> Self {
        Orchestrator { udp_actor, mc_actor }
    }
}

impl Handler<McEvent> for Orchestrator {
    type Result = ();

    fn handle(&mut self, msg: McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            McEvent::ReceivedValue(value) => {
                value["type"].handle::<String>().map(|v| {
                    if v == "device" {
                        info!("Received device announcement: {}", value.to_json());
                        // Handle device announcement
                    } else {
                        info!("Received unknown type: {}", v);
                    }
                }).unwrap_or_else(|_| {
                    error!("Failed to parse value type");
                });
            }
        }
    }
}


#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli {
    /// Optional name to operate on
    ///     #[arg(short, long, value_name = "FILE")]
    #[arg(default_value_t=TESTER_PORT)]
    udp_port: u16,
    /// Optional multicast IP address
    #[arg(default_value_t = String::from("0.0.0.0"))]
    udp_ip: String,
    // optional own object name
    #[arg(short, long, default_value = "tester")]
    object_name: Option<String>,
}

#[actix::main]
async fn main() -> std::io::Result<()> {
    // Parse command line arguments
    logger::init();
    let cli = Cli::parse();
    let object_name = cli.object_name.unwrap_or_else(|| DEVICE_NAME.to_string());
    let udp_port = cli.udp_port;
    let interface = local_ip().unwrap_or(std::net::IpAddr::V4(Ipv4Addr::new(0, 0, 0, 0))).to_string();
    let my_ip = local_ip().unwrap().to_string();
    let mc_actor: Addr<McActor> = McActor::new(MULTICAST_IP,MULTICAST_PORT).start();
    let udp_actor = UdpActor::new(my_ip.as_str(),udp_port).start();


    // Spawn task for sending multicast announcements
    tokio::spawn(async move {
        let mut interval = interval(Duration::from_secs(5));
        loop {
            interval.tick().await;
            let mut announce = Value::object();
            announce["src"] = Value::string(object_name);
            announce["type"] = Value::string("device");
            announce["ip"] = Value::string(&my_ip);
            announce["port"] = Value::int(udp_port as i64);
            announce["sub"] = Value::object();
            announce["sub"]["pattern"] = Value::array();
            announce["sub"]["pattern"] += Value::string("esp1/sys");

            info!("MC  {}", announce.to_json());
            // Create and send multicast announcement
            mc_actor.do_send(McCmd::SendValue(announce.clone()));

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
            udp_actor.do_send(UdpCmd::SendValue {
                socket_addr: SocketAddr::new(
                    Ipv4Addr::from_str(&cli.udp_ip).unwrap(),
                    udp_port,
                ),
                value: v,
            });
        }
    });

    // Main loop for receiving messages
    let mut buf = [0u8; 1500];
    loop {
        let (len, src) = socket.recv_from(&mut buf).await?;
        let data = &buf[..len];
        let str: &str = std::str::from_utf8(data).unwrap_or("");
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

