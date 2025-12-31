use anyhow::Result;
use dashmap::DashMap;
use log::{debug, error, info};
use serde::{Deserialize, Serialize};
use socket2::{Domain, Protocol, Socket, Type};
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::ops::Mul;
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::net::UdpSocket;
use tokio::sync::{mpsc, Mutex};
use tokio::task::JoinHandle;

use crate::msgs::TypedMessage;

use crate::msgs::{Alive, Msg, UdpMessage};

// --- CONSTANTS ---
pub const MULTICAST_PORT: u16 = 50000;
pub const HEARTBEAT_INTERVAL: u64 = 5;
pub const PEER_TIMEOUT: u64 = 6;

#[derive(Clone, Debug)]
pub struct Subscription {
    msg_type: String,
    destination: String,
    last_seen: Instant,
}

//======================================================================================================================
pub struct MulticastScout {
    multicast_addr: SocketAddr,
    multicast_socket: Arc<UdpSocket>,
    pub endpoints: DashMap<String, (SocketAddr, Instant)>,
    pub subscriptions: DashMap<String, Subscription>,
}

impl MulticastScout {
    pub async fn new(multicast_str: String) -> Result<Arc<Self>> {
        let multicast_addr: SocketAddrV4 = multicast_str.parse()?;
        let socket2 = Socket::new(Domain::IPV4, Type::DGRAM, Some(Protocol::UDP))?;
        socket2.set_reuse_address(true)?;
        #[cfg(not(windows))]
        socket2.set_reuse_port(true)?; // Unix specific, helps with load balancing/binding

        // Bind to 0.0.0.0:5000 (or multicast IP directly on Windows sometimes)
        socket2.bind(
            &format!("0.0.0.0:{}", MULTICAST_PORT)
                .parse::<SocketAddr>()
                .unwrap()
                .into(),
        )?;

        // Join the group
        socket2.join_multicast_v4(&multicast_addr.ip(), &Ipv4Addr::UNSPECIFIED)?;
        socket2.set_nonblocking(true)?;

        let multicast_socket = UdpSocket::from_std(socket2.into())?;
        info!("Multicast socket bound to {:?}", multicast_addr);
        Ok(Arc::new(Self {
            multicast_addr: SocketAddr::V4(multicast_addr),
            multicast_socket: Arc::new(multicast_socket),

            endpoints: DashMap::new(),
            subscriptions: DashMap::new(),
        }))
    }

    fn start_multicast_receiver(node: Arc<Self>) -> JoinHandle<()> {
        let multicast_socket = node.multicast_socket.clone();
        let discovery_node = node.clone();
        tokio::spawn(async move {
            let mut buf = [0u8; 65535];
            loop {
                if let Ok((len, _addr)) = multicast_socket.recv_from(&mut buf).await {
                    let data: Vec<u8> = buf[..len].to_vec();
                    // Discovery packets are just JSON inside CBOR, or raw JSON?
                    // Let's assume the protocol is consistent: Packet struct via CBOR
                    debug!("MC Recv {} bytes from {:?}", len, _addr);
                    if let Ok(udp_message) = UdpMessage::cbor_deserialize(&data) {
                        debug!(
                            "MC Recv {:?} from {:?}",
                            udp_message.msg_type, udp_message.src
                        );
                        if Some("Alive") == udp_message.msg_type.as_deref() {
                            let src= udp_message.src.clone().unwrap_or("unknown".to_string());
                            if let Ok(_alive) =
                                Alive::json_deserialize(&udp_message.payload.clone().unwrap())
                            {
                                debug!(
                                    "MC Alive from {:?} \t{:?}",
                                    _addr,
                                    udp_message.src.unwrap()
                                );
                                if let Some(endpoints) = _alive.endpoints.as_ref() {
                                    for ep in endpoints {
                                        if !discovery_node.endpoints.contains_key(ep) {
                                            info!(" Discovered peer '{}' @ {:?}", ep, _addr);
                                        }
                                        discovery_node
                                            .endpoints
                                            .insert(ep.clone(), (_addr, Instant::now()));
                                    }
                                }
                                if let Some(subs) = _alive.subscriptions.as_ref() {
                                    for sub in subs {
                                        let subscription = Subscription {
                                            msg_type: sub.clone(),
                                            destination: src.clone(),
                                            last_seen: Instant::now(),
                                        };
                                        discovery_node
                                            .subscriptions
                                            .insert(sub.clone(), subscription);
                                    }
                                }
                            } else {
                                let payload = udp_message.payload.as_ref().clone().unwrap();
                                error!(
                                    "Failed to deserialize Alive message from {} : {:?} ",
                                    udp_message.src.unwrap_or("unknown".to_string()),
                                    std::str::from_utf8(&payload)
                                );
                            }
                        } else {
                            error!("Unknown multicast message type");
                        }
                    }
                }
            }
        })
    }

   

    fn prune(scout: Arc<Self>) -> JoinHandle<()> {
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(HEARTBEAT_INTERVAL));
            loop {
                interval.tick().await;
                let peer_count = scout.endpoints.len();
                scout.endpoints.iter_mut().for_each(|mut entry| {
                    let (_addr, last_seen) = &mut *entry.value_mut();
                    if last_seen.elapsed().as_secs() >= PEER_TIMEOUT {
                        info!("Endpoint {} timed out", entry.key());
                    }
                });
                scout
                    .endpoints
                    .retain(|_, (_, last_seen)| last_seen.elapsed().as_secs() < PEER_TIMEOUT);
                let pruned_count = peer_count - scout.endpoints.len();
                if pruned_count > 0 {
                    info!("pruned {} dead endpoints", pruned_count);
                }
                info!(
                    "Endpoints {:?}",
                    scout
                        .endpoints
                        .iter()
                        .map(|r| (r.key().clone(), r.value().0.to_string()))
                        .collect::<Vec<(String, String)>>()
                );
                info!(
                    "Subscriptions {:?}",
                    scout
                        .subscriptions
                        .iter()
                        .map(|r| (r.key().clone(), r.value().destination.clone()))
                        .collect::<Vec<(String, String)>>()
                );
            }
        })
    }

    pub fn start(self: Arc<Self>) -> JoinHandle<()> {
        let mc_receiver = MulticastScout::start_multicast_receiver(self.clone());
        let pruner = MulticastScout::prune(self.clone());

        tokio::spawn(async move {
            let _ = tokio::join!(mc_receiver, pruner);
        })
    }

    pub fn endpoint_to_addr(&self, endpoint: &str) -> Option<SocketAddr> {
        if let Some(entry) = self.endpoints.get(endpoint) {
            Some(entry.value().0)
        } else {
            None
        }
    }


}
