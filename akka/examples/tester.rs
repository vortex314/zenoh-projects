use actix::Actor;
use actix::Addr;
use actix::Context;
use actix::Handler;
use akka::ClientActor;
use akka::ClientCmd;
use akka::ClientEvent;
use akka::McCmd;
use akka::McEvent;
use akka::UdpActor;
use akka::UdpCmd;
use clap::Parser;
use local_ip_address::local_ip;
use log::info;
use serde_json;
use std::net::{Ipv4Addr, SocketAddrV4};
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::net::UdpSocket;
use tokio::net::unix::SocketAddr;
use tokio::time::{Duration, interval};

// import necessary modules

use akka::logger::{self, init};
use akka::{McActor, Value};

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
        Orchestrator {
            udp_actor,
            mc_actor,
        }
    }
}

impl Handler<McEvent> for Orchestrator {
    type Result = ();

    fn handle(&mut self, msg: McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            McEvent::ReceivedValue(value) => {
                value["type"].handle::<String, _>(|v: &String| {
                    if v == "device" {
                        info!("Received device announcement: {}", value.to_json());
                        // Handle device announcement
                    } else {
                        info!("Received unknown type: {}", v);
                    }
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

fn publish_msg() -> Value {
    let mut announce = Value::object();
    announce["src"] = Value::string(DEVICE_NAME);
    announce["type"] = Value::string("device");
    announce["ip"] = Value::string(BROKER_IP);
    announce["port"] = Value::int(BROKER_PORT as i64);
    announce["sub"] = Value::object();
    announce["sub"]["pattern"] = Value::array();
    announce["sub"]["pattern"] += Value::string("esp1/sys");
    announce
}

struct Tester {}
impl Actor for Tester {
    type Context = Context<Self>;
}
impl Handler<ClientEvent> for Tester {
    type Result = ();

    fn handle(&mut self, msg: ClientEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            ClientEvent::Publish(value) => {
                info!("Tester received publish: {}", value.to_json());
            }
            _ => {
                info!("Tester received unknown event: {:?}", msg);
            }
        }
    }
}

#[actix::main]
async fn main() -> std::io::Result<()> {
    // Parse command line arguments
    logger::init();
    let cli = Cli::parse();
    let object_name = cli.object_name.unwrap_or_else(|| DEVICE_NAME.to_string());
    let udp_port = cli.udp_port;
    let interface = local_ip()
        .unwrap_or(std::net::IpAddr::V4(Ipv4Addr::new(0, 0, 0, 0)))
        .to_string();
    let my_ip = local_ip().unwrap().to_string();
    let client_actor = ClientActor::new(
        object_name.clone(),
        MULTICAST_IP,
        MULTICAST_PORT,
        my_ip.clone().as_str(),
        udp_port,
    )
    .start();

    client_actor
        .send(ClientCmd::Subscribe {
            dst: Some(object_name.clone()),
            src: Some ("esp1".to_string()),
        })
        .await
        .unwrap();

    loop {
        // Wait for a while before sending the announce message
        tokio::time::sleep(Duration::from_millis(1000)).await;

        // Send an announce message to the multicast group
        let announce = publish_msg();
        info!("Sending announce: {}", announce.to_json());
        client_actor
            .send(ClientCmd::Publish(publish_msg()))
            .await
            .unwrap();
    }

    Ok(())
}
