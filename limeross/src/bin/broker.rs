use log::{debug, info};
use serde::{Deserialize, Serialize};
use tokio::signal;
use udp_broker_lib::{
    logger,
    msgs::{Alive, Msg, SysEvent, TypedMessage, UdpMessage},
    MessageHandler, UdpMessageHandler, UdpNode,
};

struct Filter {
    source: Option<String>,
    message_types: Vec<String>,
    destination: String,
}

impl Filter {
    fn new(destination: String) -> Self {
        Self {
            source: None,
            message_types: vec![],
            destination,
        }
    }
    fn matches(&self, udp_message: &UdpMessage) -> Option<String> {
        debug!("Filter checking message: {:?}", udp_message);
        if let Some(ref src) = self.source {
            if let Some(ref msg_src) = udp_message.src {
                if src != msg_src {
                    return None;
                }
            } else {
                return None;
            }
        }
        if !self.message_types.is_empty() {
            if let Some(ref msg_type) = udp_message.msg_type {
                if !self.message_types.contains(msg_type) {
                    return None;
                }
            } else {
                return None;
            }
        }
        Some(self.destination.clone())
    }
}

#[async_trait::async_trait]
impl UdpMessageHandler for Filter {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        if let Some(dest) = self.matches(udp_message) {
            info!("Filter matched! Forwarding to {}", dest);
            // Here you would implement the forwarding logic
        }
        Ok(())
    }
}

struct AllFilters {
    filters: Vec<Filter>,
}

#[async_trait::async_trait]
impl UdpMessageHandler for AllFilters {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        for filter in &self.filters {
            filter.handle(udp_message).await?;
        }
        Ok(())
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    logger::init();
    info!("Starting BROKER...");
    // Bind Unicast to 8080. Multicast listens on 5000 automatically via socket2 logic in lib.
    let node = UdpNode::new("broker","239.0.0.1:50000").await?;

    tokio::spawn(async move {
        loop {

            tokio::time::sleep(tokio::time::Duration::from_secs(5)).await;
        }
    });

    let mut f = Filter::new("broker".to_string());
    f.message_types.push(SysEvent::MSG_TYPE.to_string());
    let af = AllFilters { filters: vec![f] };

    node.add_generic_handler(af).await;

    signal::ctrl_c().await?;
    Ok(())
}


//======================================================================================================================
struct UdpMessageFilter {
    message_types: Vec<String>,
    sources: Vec<String>,
    destination: String,
}

impl UdpMessageFilter {
    fn new(destination: String) -> Self {
        Self {
            message_types: vec![],
            sources: vec![],
            destination,
        }
    }

    fn matches(&self, udp_message: &UdpMessage) -> Option<String> {
        info!("Filter checking message: {:?}", udp_message);
        if !self.sources.is_empty() {
            if let Some(ref msg_src) = udp_message.src {
                if !self.sources.contains(msg_src) {
                    return None;
                }
            } else {
                return None;
            }
        }
        if !self.message_types.is_empty() {
            if let Some(ref msg_type) = udp_message.msg_type {
                if !self.message_types.contains(msg_type) {
                    return None;
                }
            } else {
                return None;
            }
        }
        Some(self.destination.clone())
    }
}