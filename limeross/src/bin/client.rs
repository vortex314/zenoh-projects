use clap::Parser;
use serde::{Deserialize, Serialize};
use std::time::Duration;
use tokio::time::sleep;
use udp_broker_lib::{
    UdpNode, logger, msgs::{Alive, SysEvent, TypedMessage}
};
use log::info;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(short = 'a', long, default_value_t = ("224.0.0.1".to_string()))]
    multicast_addr: String,

    #[arg(short = 'p', long, default_value_t = 50000)]
    multicast_port: u16,

    #[arg(short = 'u', long, default_value_t = 50001)]
    unicast_port: u16,

    #[arg(short = 'n', long, default_value_t = ("node-a".to_string()))]
    node_name: String,

    #[arg(short = 'f', long, default_value_t = 1)]
    frequency: u64,

    #[arg(short = 'c', long, default_value_t = 1)]
    client_count: u64,
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let args = Args::parse();
    logger::init();
    let client_id = args.node_name.clone();
    info!("[{}] Starting CLIENT...", client_id);
    let (node, _) = UdpNode::new(&client_id, args.unicast_port).await?;
    node.add_endpoint("client");
    node.add_subscription(Alive::MSG_TYPE);

    node.on::<Alive, _, _>(move |source, msg| async move {
        info!("[{}] EVENT: {:?} from {}", "ME", msg.subscriptions, source);
    });

    let client_id_clone = client_id.clone();
    node.on::<udp_broker_lib::msgs::Alive, _, _>(move |source, msg| {
        let client_id_clone = client_id_clone.clone();
        async move {
            info!(
                "[{}] Detected alive from {}: {:?}",
                client_id_clone, source, msg
            );
        }
    });
    let client_id_clone = client_id.clone();
    node.on::<udp_broker_lib::msgs::Alive, _, _>(move |source, msg| {
        let client_id_clone = client_id_clone.clone();
        async move {
            info!(
                "[{}] Detected alive from {}: {:?}",
                client_id_clone, source, msg
            );
        }
    });

    info!("[{}] Waiting for Broker...", client_id);
    loop {
        if node.peers.contains_key("BROKER_01") {
            break;
        }
        sleep(Duration::from_secs(1)).await;
    }
    info!("[{}] Found Broker!", client_id);

    loop {
        if node.peers.contains_key("BROKER_01") {
            // info!("Sending data...");

            let sys_event = SysEvent::default();
            let _ = node.send_typed("BROKER_01", sys_event).await;
        }
        sleep(Duration::from_secs(2)).await;
    }
}
