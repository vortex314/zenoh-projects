use anyhow::Result;
use dashmap::DashMap;
use log::{debug, error, info};
use socket2::{Domain, Protocol, Socket, Type};
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::net::UdpSocket;
use tokio::task::JoinHandle;


use crate::msgs::{Alive, Msg, UdpMessage};

// --- CONSTANTS ---
pub const MULTICAST_PORT: u16 = 50000;
pub const HEARTBEAT_INTERVAL: u64 = 3;
pub const PEER_TIMEOUT: u64 = 6;

#[derive(Clone, Debug)]
pub struct Subscription {
    pub event_pattern: String,
    pub destination: String,
    pub last_seen: Instant,
}

#[derive(Clone, Debug)]
pub struct Endpoint {
    pub name: String,
    pub addr: SocketAddr,
    pub last_seen: Instant,
    pub subscribe: Vec<String>,
    pub publish: Vec<String>,
    pub services: Vec<String>,
}

//======================================================================================================================
pub struct Scout {
    multicast_addr: SocketAddr,
    multicast_socket: Arc<UdpSocket>,
    pub endpoints: Arc<DashMap<String, Endpoint>>,
    pub subscriptions: Arc<DashMap<String, Vec<Subscription>>>,
}

impl Scout {
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
            endpoints: Arc::new(DashMap::new()),
            subscriptions: Arc::new(DashMap::new()),
        }))
    }

    pub(crate) fn handle_multicast_packet(&self, data: &[u8], addr: SocketAddr) -> Result<()> {
        let data_vec = data.to_vec();
        let udp_message = UdpMessage::cbor_deserialize(&data_vec)?;

        debug!(
            "MC Recv {:?} from {:?}",
            udp_message.msg_type, udp_message.src
        );

        if Some("Alive") != udp_message.msg_type.as_deref() {
            return Ok(());
        }

        let src = udp_message
            .src
            .clone()
            .unwrap_or_else(|| "unknown".to_string());
        let payload = udp_message
            .payload
            .as_ref()
            .ok_or_else(|| anyhow::anyhow!("Alive message missing payload"))?;

    //    let subscriptions = self.subscriptions.clone();

        let alive = Alive::json_deserialize(payload)?;
        let subscribe = alive.subscribe.clone().unwrap_or_default();
        let publish = alive.publish.clone().unwrap_or_default();
        let services = alive.services.clone().unwrap_or_default();

        // Update endpoint
        self.endpoints.insert(
            src.clone(),
            Endpoint {
                name: src.clone(),
                addr,
                last_seen: Instant::now(),
                subscribe: subscribe.clone(),
                publish: publish.clone(),
                services: services.clone(),
            },
        );

        // Update subscriptions
        if let Some(subs) = alive.subscribe.as_ref() {
            for sub in subs {
                let subscription = Subscription {
                    event_pattern: sub.clone(),
                    destination: src.clone(),
                    last_seen: Instant::now(),
                };

                let mut entry = self
                    .subscriptions
                    .entry(sub.clone())
                    .or_insert_with(|| vec![]);
                if let Some(existing) = entry.iter_mut().find(|s| s.destination == src) {
                    existing.last_seen = Instant::now();
                } else {
                    entry.push(subscription);
                }
                debug!("Updated subscription for {} to {}", sub, src);
            }
        }

        Ok(())
    }

    fn start_multicast_receiver(node: Arc<Self>) -> JoinHandle<()> {
        let multicast_socket = node.multicast_socket.clone();
        let discovery_node = node.clone();
        tokio::spawn(async move {
            let mut buf = [0u8; 65535];
            loop {
                if let Ok((len, _addr)) = multicast_socket.recv_from(&mut buf).await {
                    let data: Vec<u8> = buf[..len].to_vec();
                    debug!("MC Recv {} bytes from {:?}", len, _addr);
                    if let Err(e) = discovery_node.handle_multicast_packet(&data, _addr) {
                        error!("Failed to handle multicast packet from {:?}: {e}", _addr);
                    }
                }
            }
        })
    }

    async fn prune(scout: Arc<Self>) -> JoinHandle<()> {
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(HEARTBEAT_INTERVAL));
            loop {
                interval.tick().await;
                let peer_count = scout.endpoints.len();
                scout.endpoints.iter_mut().for_each(|mut entry| {
                    let endpoint = &mut *entry.value_mut();
                    if endpoint.last_seen.elapsed().as_secs() >= PEER_TIMEOUT {
                        info!("Endpoint {} timed out", entry.key());
                        // remove subscriptions where destination matches
                        scout.subscriptions.iter_mut().for_each(|mut sub_entry| {
                            sub_entry
                                .value_mut()
                                .retain(|s| s.destination != *entry.key());
                        });
                        // if no more subscriptions for a key, remove the key
                        scout.subscriptions.retain(|_k, v| !v.is_empty());
                    }
                });
                scout
                    .endpoints
                    .retain(|_, endpoint| endpoint.last_seen.elapsed().as_secs() < PEER_TIMEOUT);
                let pruned_count = peer_count - scout.endpoints.len();
                if pruned_count > 0 {
                    info!("pruned {} dead endpoints", pruned_count);
                }
                debug!(
                    "Endpoints {:?}",
                    scout
                        .endpoints
                        .iter()
                        .map(|r| (r.key().clone(), r.value().addr.to_string()))
                        .collect::<Vec<(String, String)>>()
                );
                debug!(
                    "Subscriptions {:?}",
                    scout
                        .subscriptions
                        .iter()
                        .map(|r| (r.key().clone(), format!("{:?}", r.value())))
                        .collect::<Vec<(String, String)>>()
                );
            }
        })
    }

    pub fn start(self: Arc<Self>) -> JoinHandle<()> {
        let mc_receiver = Scout::start_multicast_receiver(self.clone());
        let pruner = Scout::prune(self.clone());

        tokio::spawn(async move {
            let _ = tokio::join!(mc_receiver, pruner);
        })
    }

    pub fn endpoint_to_addr(&self, endpoint: &str) -> Option<SocketAddr> {
        if let Some(entry) = self.endpoints.get(endpoint) {
            Some(entry.value().addr)
        } else {
            None
        }
    }
}
