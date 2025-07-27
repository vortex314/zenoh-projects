use actix::Actor;
use actix::Arbiter;
use actix::AsyncContext;
use actix::Context;
use actix::Handler;
use actix::Message;
use actix::Recipient;
use akka::ClientActor;
use akka::ClientCmd;
use akka::ClientEvent;

use clap::Parser;
use local_ip_address::local_ip;
use log::info;

use std::net::Ipv4Addr;
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::time::{interval, Duration};

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

struct Tester {
    name: String,
    client: Recipient<ClientCmd>,
    counter: i64,
}
#[derive(Debug, Message)]
#[rtype(result = "()")]
enum TesterCmd {
    Publish,
}
impl Tester {
    fn new(name: String, client: Recipient<ClientCmd>) -> Self {
        Tester {
            name,
            client,
            counter: 0,
        }
    }

    fn publish(&mut self) {
        let mut value = Value::object();
        value["src"] = self.name.clone().into();
        value["pub"] = Value::object();
        value["pub"]["index"] = self.counter.into();
        self.client.do_send(ClientCmd::Publish(value));
        self.counter += 1;
    }
}
impl Actor for Tester {
    type Context = Context<Self>;

    fn started(&mut self, ctx: &mut Self::Context) {
        self.client.do_send(ClientCmd::Register({
            name:self.name,
            dst: Some(self.name.clone()),
            src: Some("esp1".to_string()),
        });
        let self_ref = ctx.address();
        let publisher = async move {
            loop {
                tokio::time::sleep(Duration::from_millis(1000)).await;
                self_ref.do_send(TesterCmd::Publish);
            }
        };
        Arbiter::current().spawn(publisher);
    }
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
impl Handler<TesterCmd> for Tester {
    type Result = ();

    fn handle(&mut self, msg: TesterCmd, _ctx: &mut Self::Context) -> Self::Result {
        match msg {
            TesterCmd::Publish => {
                self.publish();
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
        my_ip.as_str(),
        udp_port,
    )
    .start();

    let client_actor_1 = client_actor.clone();

    let tester_actor = Tester::new(object_name.clone(), client_actor_1.recipient()).start();

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
