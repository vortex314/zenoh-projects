use dashmap::DashMap;
use log::{debug, error, info};
use serde::{Deserialize, Serialize};
use socket2::{Domain, Protocol, Socket, Type};
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::net::UdpSocket;
use tokio::sync::{Mutex, mpsc};

pub mod logger;
pub mod msgs;
use msgs::TypedMessage;

use crate::msgs::{Alive, Msg, UdpMessage};

// --- CONSTANTS ---
pub const MULTICAST_IP: &str = "239.0.0.1";
pub const MULTICAST_PORT: u16 = 50000;
pub const UNICAST_PORT: u16 = 50001;
pub const HEARTBEAT_INTERVAL: u64 = 2;
pub const PEER_TIMEOUT: u64 = 6;

// --- PROTOCOL ---
/*
#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Packet {
    pub source: String,
    pub destination: String,
    pub message_type: String,
    pub payload: Vec<u8>,
}
    */
/*
#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Discovery {
    pub id: String,
    /// The Unicast address where this node listens for DATA
    pub data_addr: SocketAddr,
    pub interests: Vec<String>,
}*/

// --- TRAIT DISPATCH ---

/*pub trait TypedMessage: for<'a> Deserialize<'a> + Send + Sync + 'static {
    const MSG_TYPE: &'static str;
}*/

#[async_trait::async_trait]
pub trait MessageHandler: Send + Sync {
    async fn handle(&self, source: &str, payload: &[u8]) -> anyhow::Result<()>;
}

pub struct HandlerWrapper<T, F> {
    callback: F,
    _marker: std::marker::PhantomData<T>,
}


impl<T, F> HandlerWrapper<T, F> {
    pub fn new(callback: F) -> Self {
        Self {
            callback,
            _marker: std::marker::PhantomData,
        }
    }
}

#[async_trait::async_trait]
pub trait UdpMessageHandler: Send + Sync {
    async fn handle(&self, udp_message : &UdpMessage) -> anyhow::Result<()>;
}
pub struct UdpHandlerWrapper<T, F> {
    callback: F,
    message_types: Vec<String>,
    sources : Vec<String>,
    destination : String,
    _marker: std::marker::PhantomData<T>,
}

impl<T, F> UdpHandlerWrapper<T, F> {
    pub fn new(destination: String, callback: F) -> Self {
        Self {
            callback,
            message_types : vec![],
            sources : vec![],
            destination,
            _marker: std::marker::PhantomData,
        }
    }

    pub fn filter(mut self, message_types : Vec<String>, sources : Vec<String>) -> Self {
        self.message_types = message_types;
        self.sources = sources;
        self
    }
}



#[async_trait::async_trait]
impl<T, F, Fut> MessageHandler for HandlerWrapper<T, F>
where
    T: TypedMessage + Msg,
    F: Fn(String, T) -> Fut + Send + Sync + 'static,
    Fut: std::future::Future<Output = ()> + Send,
{
    async fn handle(&self, source: &str, payload: &[u8]) -> anyhow::Result<()> {
        let msg: T = T::json_deserialize(&payload.to_vec())?;
        (self.callback)(source.to_string(), msg).await;
        Ok(())
    }
}

// --- NETWORKING NODE ---

pub struct UdpNode {
    pub id: String,
    /// Socket used for sending and receiving UNICAST data
    unicast_socket: Arc<UdpSocket>,
    handlers: DashMap<String, Box<dyn MessageHandler>>,
    /// Maps Peer ID -> (Data Address, Last Seen)
    pub peers: DashMap<String, (SocketAddr, Instant)>,
    tx_queue: mpsc::Sender<(SocketAddr, UdpMessage)>,
    multicast_addr: SocketAddr,
    endpoints : DashMap<String, ()>,
    subscriptions : DashMap<String, ()>,
    broker_addr: Option<SocketAddr>,
    generic_handlers : Arc<Mutex<Vec<Box<dyn UdpMessageHandler>>>>,
}

impl UdpNode {
    pub async fn new(
        id: &str,
        port: u16,
    ) -> anyhow::Result<(Arc<Self>, mpsc::Sender<(SocketAddr, UdpMessage)>)> {
        // 1. Setup Data Socket (Unicast)
        // We use standard binding here.
        let unicast_socket = Arc::new(UdpSocket::bind(format!("0.0.0.0:{}", port)).await?);
        let local_data_addr = unicast_socket.local_addr()?;

        info!("[{}] Data listening on {:?}", id, local_data_addr);

        // 2. Setup Multicast Discovery Socket (Rx Only)
        // We must use socket2 to set SO_REUSEADDR so multiple nodes run on one machine.
        let multi_addr: Ipv4Addr = MULTICAST_IP.parse()?;
        let multicast_addr = SocketAddrV4::new(multi_addr, MULTICAST_PORT);

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
        socket2.join_multicast_v4(&multi_addr, &Ipv4Addr::UNSPECIFIED)?;
        socket2.set_nonblocking(true)?;

        let multicast_socket = UdpSocket::from_std(socket2.into())?;

        // 3. Setup Internal State
        let (tx, mut rx) = mpsc::channel::<(SocketAddr, UdpMessage)>(100);

        let node = Arc::new(Self {
            id: id.to_string(),
            unicast_socket: unicast_socket.clone(),
            handlers: DashMap::new(),
            peers: DashMap::new(),
            tx_queue: tx.clone(),
            multicast_addr: SocketAddr::V4(multicast_addr),
            endpoints : DashMap::new(),
            subscriptions : DashMap::new(),
            broker_addr: None,
            generic_handlers : Arc::new(Mutex::new(Vec::new())),
        });

        // --- LOOP 1: Outgoing Data Sender ---
        let send_socket = unicast_socket.clone();
        tokio::spawn(async move {
            while let Some((target, packet)) = rx.recv().await {
                debug!("UC Send {:?} to {}", packet.msg_type, target);
                if let Ok(data) = packet.cbor_serialize() {
                    if let Err(e) = send_socket.send_to(&data, target).await {
                        error!("Send error: {}", e);
                    }
                }
            }
        });

        // --- LOOP 2: Incoming Data Receiver (Unicast) ---
        let recv_node = node.clone();
        let recv_socket = unicast_socket.clone();
        tokio::spawn(async move {
            let mut buf = [0u8; 65535];
            loop {
                if let Ok((len, addr)) = recv_socket.recv_from(&mut buf).await {
                    let data: Vec<u8> = buf[..len].to_vec();
                    if let Ok(packet) = UdpMessage::cbor_deserialize(&data) {
                        let gen_handlers = recv_node.generic_handlers.clone();
                        let handlers_guard = gen_handlers.lock().await;
                        for i in 0..handlers_guard.len() {
                            let packet = packet.clone();
                            let gen_handlers_clone = gen_handlers.clone();
                            tokio::spawn(async move {
                                let handlers = gen_handlers_clone.lock().await;
                                if let Some(handler) = handlers.get(i) {
                                    if let Err(e) = handler.handle(&packet).await {
                                        error!("Generic handler error: {}", e);
                                    }
                                }
                            });
                        }
                        drop(handlers_guard);
                        // Data packet handling
                        if let Some(handler) = recv_node.handlers.get(&packet.msg_type.unwrap()) {
                            // We spawn here so we don't block the receive loop
                            let handler_clone = recv_node.clone(); // Usually handler logic needs state
                                                                   // Note: We can't easily clone the trait object, so we execute inline or need Arc structure
                                                                   // For simplicity, we execute strictly serially here, or rely on the handler to spawn
                            if let Err(e) = handler
                                .handle(&packet.src.unwrap(), &packet.payload.unwrap())
                                .await
                            {
                                error!("Handler error: {}", e);
                            }
                        }
                    } else {
                         info!("Failed to decode CBOR packet from {}", addr);
                    }
                }
            }
        });

        // --- LOOP 3: Discovery Receiver (Multicast) ---
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
                            if let Ok(_alive) =
                                Alive::json_deserialize(&udp_message.payload.clone().unwrap())
                            {
                                info!(
                                    "MC Alive from {:?}@{:?} {:?}",
                                    udp_message.src, _addr, _alive
                                );
                                if let Some(endpoints) = _alive.endpoints.as_ref() {
                                    for ep in endpoints {
                                        if !discovery_node.peers.contains_key(ep) {
                                            info!(
                                                "[{}] Discovered peer '{}' @ {:?}",
                                                discovery_node.id, ep, _addr
                                            );
                                        }
                                        discovery_node
                                            .peers
                                            .insert(ep.clone(), (_addr, Instant::now()));
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
        });

        // --- LOOP 4: Maintenance (Heartbeat & Pruning) ---
        let main_node = node.clone();
        let main_tx = tx.clone();
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(HEARTBEAT_INTERVAL));
            // Target address for multicast
            let m_addr: SocketAddr = format!("{}:{}", MULTICAST_IP, MULTICAST_PORT)
                .parse()
                .unwrap();

            loop {
                interval.tick().await;

                // 1. Send Heartbeat
                let mut alive = Alive::default();
                alive.endpoints = Some(
                    main_node
                        .endpoints
                        .iter()
                        .map(|entry| entry.key().clone())
                        .collect(),
                );
                alive.subscriptions = Some(
                    main_node
                        .subscriptions
                        .iter()
                        .map(|entry| entry.key().clone())
                        .collect(),
                );

                if let Ok(payload) = alive.json_serialize() {
                    let packet = UdpMessage {
                        src: Some(main_node.id.clone()),
                        dst: None,
                        msg_type: Some(Alive::MSG_TYPE.to_string()),
                        payload: Some(payload),
                    };
                    // We send this via our standard TX queue, aiming at the Multicast Addr
                    info!("MC Send Alive [{}] {:?}", main_node.id, alive);
                    let _ = main_tx.send((m_addr, packet)).await;
                }

                // 2. Prune Dead Peers
                let peer_count = main_node.peers.len();
                main_node.peers.iter_mut().for_each(|mut entry| {
                    let (_addr, last_seen) = &mut *entry.value_mut();
                    if last_seen.elapsed().as_secs() >= PEER_TIMEOUT {
                        info!("[{}] Peer {} timed out", main_node.id, entry.key());
                    }
                });
                main_node
                    .peers
                    .retain(|_, (_, last_seen)| last_seen.elapsed().as_secs() < PEER_TIMEOUT);
                let pruned_count = peer_count - main_node.peers.len();
                if pruned_count > 0 {
                    info!("[{}] Pruned {} dead peers", main_node.id, pruned_count);
                }
            }
        });

        Ok((node, tx))
    }


    pub fn add_handler<H>(&self, msg_type: &str, handler: H)
    where
        H: MessageHandler + 'static,
    {
        self.handlers.insert(msg_type.to_string(), Box::new(handler));
    }

    pub fn on<T, F, Fut>(&self, callback: F)
    where
        T: TypedMessage + Msg,
        F: Fn(String, T) -> Fut + Send + Sync + 'static,
        Fut: std::future::Future<Output = ()> + Send,
    {
        self.handlers.insert(
            T::MSG_TYPE.to_string(),
            Box::new(HandlerWrapper::new(callback)),
        );
    }

    pub async fn send_typed<T: TypedMessage + Msg + Serialize>(
        &self,
        dest_id: &str,
        msg: T,
    ) -> anyhow::Result<()> {
        if let Some(peer_entry) = self.peers.get(dest_id) {
            let (addr, _) = *peer_entry;
            let payload = msg.json_serialize()?;

            let packet = UdpMessage {
                src: Some(self.id.clone()),
                dst: Some(dest_id.to_string()),
                msg_type: Some(T::MSG_TYPE.to_string()),
                payload: Some(payload),
            };

            self.tx_queue
                .send((addr, packet))
                .await
                .map_err(|_| anyhow::anyhow!("Queue Closed"))?;
            Ok(())
        } else {
            Err(anyhow::anyhow!("Peer {} not found", dest_id))
        }
    }

    pub async fn send_multicast_typed<T: TypedMessage + Msg + Serialize>(
        &self,
        msg: T,
    ) -> anyhow::Result<()> {
            let payload = msg.json_serialize()?;

            let packet = UdpMessage {
                src: Some(self.id.clone()),
                dst: None,
                msg_type: Some(T::MSG_TYPE.to_string()),
                payload: Some(payload),
            };

            self.tx_queue
                .send((self.multicast_addr, packet))
                .await
                .map_err(|_| anyhow::anyhow!("Queue Closed"))?;
            Ok(())
        
    }

    pub fn add_endpoint(&self, endpoint: &str) {
        self.endpoints.insert(endpoint.to_string(), ());
    }

    pub fn add_subscription(&self, subscription: &str) {
        self.subscriptions.insert(subscription.to_string(), ());
    }

    pub async fn add_generic_handler<H>(&self,  handler: H)
    where
        H: UdpMessageHandler + 'static,
    {
        self.generic_handlers.lock().await.push(Box::new(handler));
    }
}
