use std::sync::Arc;

use dashmap::DashMap;
use log::{debug, info, warn};
use tokio::{signal, sync::mpsc::Sender};
use limeros::{logger, msgs::UdpMessage, scout::Subscription, UdpMessageHandler, UdpNode};

struct Forwarder {
    sender: Sender<UdpMessage>,
    subscriptions: Arc<DashMap<String, Vec<Subscription>>>,
}

#[async_trait::async_trait]
impl UdpMessageHandler for Forwarder {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        debug!(
            "Forwarder received message of type {:?} from {:?}",
            udp_message.msg_type, udp_message.src
        );
        if let Some(msg_type) = &udp_message.msg_type {
            if let Some(dsts) = self.subscriptions.get(msg_type) {
                for dst in dsts.iter() {
                    self.sender
                        .send(UdpMessage {
                            dst: Some(dst.destination.clone()),
                            ..udp_message.clone()
                        })
                        .await?;
                    debug!(
                        "Forwarded message of type {:?} from {:?} to {:?}",
                        udp_message.msg_type,
                        udp_message.src,
                        dst.destination.clone()
                    );
                }
            } else {
                debug!(
                    "No subscriptions for message type {:?} from {:?} {}",
                    udp_message.msg_type,
                    udp_message.src,
                    self.subscriptions.len()
                );
            }
        } else {
            warn!(
                "Forwarder received message with no type from {:?}",
                udp_message.src
            );
        }

        Ok(())
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    logger::init();
    info!("Starting BROKER...");
    // Bind Unicast to 8080. Multicast listens on 5000 automatically via socket2 logic in lib.
    let node = UdpNode::new("broker", "239.0.0.1:50000").await?;
    let sender = node.sender();
    let subscriptions = node.get_subscriptions().await;
    let forwarder = Forwarder {
        sender,
        subscriptions,
    };
    node.add_generic_handler(forwarder).await;

    tokio::spawn(async move {
        loop {
            node.display_subscriptions().await;
            let eps = node.get_endpoints().await.clone();
            info!(
                "Endpoints {:?}",
                eps.iter()
                    .map(|r| (
                        r.key().clone(),
                        r.value().addr.to_string(),
                        r.value().last_seen.elapsed().as_millis()
                    ))
                    .collect::<Vec<(String, String, u128)>>()
            );
            tokio::time::sleep(tokio::time::Duration::from_secs(5)).await;
        }
    });

    signal::ctrl_c().await?;
    Ok(())
}
