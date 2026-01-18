use clap::Parser;
use limeros::{
    TypedUdpMessage, UdpMessage, UdpMessageHandler, UdpNode, logger, msgs::{PingRep, PingReq, SysEvent, TypedMessage}
};
use log::info;
use std::{sync::Arc, time::Duration};
use tokio::time::sleep;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(short = 'm', long, default_value_t = ("239.0.0.1:50000".to_string()))]
    multicast_addr: String,

    #[arg(short = 'u', long, default_value_t = 50001)]
    unicast_port: u16,

    #[arg(short = 'n', long, default_value_t = ("client".to_string()))]
    node_name: String,

    #[arg(short = 'f', long, default_value_t = 2)]
    frequency: u64,

    #[arg(short = 'c', long, default_value_t = 1)]
    client_count: u64,
}       

struct Handler {
    node : Arc<UdpNode>,
}
#[async_trait::async_trait]
impl UdpMessageHandler for Handler {
    
    async fn handle(& self, udp_message: & UdpMessage) -> anyhow::Result<()> {
        match udp_message.msg_type.as_deref() {
            Some(SysEvent::MSG_TYPE) => {
                let typed_msg = TypedUdpMessage::<SysEvent>::from(udp_message.clone())?;
                info!("Generic Handler received SysEvent: {:?} ", typed_msg);
            },
            Some(PingRep::MSG_TYPE) => {
                let typed_msg = TypedUdpMessage::<PingRep>::from(udp_message.clone())?;
                info!("Generic Handler received PingRep: {:?} ", typed_msg);
                self.node.send_msg_to(
                    typed_msg.src.as_deref().unwrap_or("unknown"),
                    PingReq {
                        number: typed_msg.payload.as_ref().and_then(|p| p.number).map(|n| n + 1),
                        ..Default::default()
                    },
                ).await;
            },
            Some(other) => {
                info!("Generic Handler received unknown message type: {} ", other);
            },
            None => {
                info!("Generic Handler received message with no type ");
            }
        }
        Ok(())
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let args = Args::parse();
    logger::init();
    info!("Starting CLIENT...");
    let node = UdpNode::new(
        args.node_name.as_str(),
        args.multicast_addr.clone().as_str(),
    )
    .await?;
    node.add_subscription(SysEvent::MSG_TYPE).await;
    let handler = Handler { node: node.clone() };      
    node.add_generic_handler(handler).await;

/* 
    let node_name = args.node_name.clone();

    let node_name_clone = node_name.clone();

    node.on::<SysEvent, _, _>(move |s, event| {
        let node_name = node_name.clone();
        async move {
            if s == node_name.as_str() {
                info!("Received SysEvent: {} => {:?} ", s, event);
                return;
            }
        }
    });

    node.on::<PingRep, _, _>(move |s, event| {
        let node_name = node_name_clone.clone();
        info!("Client handler for PingRep {}", s);
        async move {
            if s == node_name.as_str() {
                info!("Received PingReply: {} => {:?} ", s, event);
                return;
            }
        }
    });
*/
    loop {
        node.send_event(SysEvent {
            cpu_board: Some("linux-x86".to_string()),
            ..Default::default()
        })
        .await;
        node.send_msg_to(
            "esp1",
            PingReq {
                number: Some(42),
                ..Default::default()
            },
        )
        .await;
    info!("{} sent SysEvent and PingReq", args.node_name);
        sleep(Duration::from_secs(args.frequency)).await;
    }
}
