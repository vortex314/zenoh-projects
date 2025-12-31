use clap::Parser;
use log::info;
use serde::{Deserialize, Serialize};
use std::time::Duration;
use tokio::time::sleep;
use udp_broker_lib::{
    UdpNode, logger, msgs::{Alive, Msg, SysEvent, TypedMessage, UdpMessage}
};

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(short = 'm', long, default_value_t = ("239.0.0.1:50000".to_string()))]
    multicast_addr: String,

    #[arg(short = 'u', long, default_value_t = 50001)]
    unicast_port: u16,

    #[arg(short = 'n', long, default_value_t = ("client".to_string()))]
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
    info!("Starting CLIENT...");
    let node = UdpNode::new(args.multicast_addr.clone().as_str()).await?;
    node.add_endpoint("client").await;
    node.add_subscription(Alive::MSG_TYPE).await;


    loop {
        let mut sys_event : SysEvent = SysEvent::default();
        sys_event.cpu_board = Some("linux-x86".to_string());
        node.sender().send(UdpMessage{
            src: Some("client".to_string()),
            dst: Some("broker".to_string()),
            msg_type: Some(SysEvent::MSG_TYPE.to_string()),
            payload: Some(sys_event.json_serialize().unwrap()),
        }).await?;
        sleep(Duration::from_secs(2)).await;
    }
}
