use anyhow::Result;
use dashmap::DashMap;
use log::{debug, error, info};
use serde::{Deserialize, Serialize};
use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::ops::Mul;
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::net::UdpSocket;
use tokio::sync::{mpsc, Mutex};
use tokio::task::JoinHandle;

pub mod logger;
pub mod msgs;
use msgs::TypedMessage;

use crate::msgs::{Alive, Msg, UdpMessage};

mod scout;
pub use scout::MulticastScout;

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
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()>;
}
pub struct UdpHandlerWrapper<T, F> {
    callback: F,
    message_types: Vec<String>,
    sources: Vec<String>,
    destination: String,
    _marker: std::marker::PhantomData<T>,
}

impl<T, F> UdpHandlerWrapper<T, F> {
    pub fn new(destination: String, callback: F) -> Self {
        Self {
            callback,
            message_types: vec![],
            sources: vec![],
            destination,
            _marker: std::marker::PhantomData,
        }
    }

    pub fn filter(mut self, message_types: Vec<String>, sources: Vec<String>) -> Self {
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
pub struct UdpNode {
    multicast_addr: SocketAddr,
    multicast_scout: Arc<MulticastScout>,
    unicast_socket: Arc<UdpSocket>,
    my_endpoints: Arc<Mutex<Vec<String>>>,
    my_subscriptions: Arc<Mutex<Vec<String>>>,
    handlers: Arc<DashMap<String, Box<dyn MessageHandler>>>,
    /// Maps Peer ID -> (Data Address, Last Seen)
    tx_queue_sender: mpsc::Sender<UdpMessage>,
    tx_queue_receiver: Arc<Mutex<mpsc::Receiver<UdpMessage>>>,
    broker_addr: Option<SocketAddr>,
    generic_handlers: Arc<Mutex<Vec<Box<dyn UdpMessageHandler>>>>,
    tasks: Mutex<Vec<JoinHandle<()>>>,
}

impl UdpNode {
    pub async fn new(multicast_addr: &str) -> Result<Arc<Self>> {
        let multicast_addr = multicast_addr
            .parse::<SocketAddr>()
            .map_err(|e| anyhow::anyhow!("Invalid multicast address: {}", e))?;
        let multicast_scout = MulticastScout::new(multicast_addr.to_string()).await?;
        let unicast_socket = Arc::new(UdpSocket::bind("0.0.0.0:0").await?); // give random port
        info!("Unicast socket bound to {}", unicast_socket.local_addr()?);
        let (tx_queue_sender, tx_queue_receiver) = mpsc::channel::<UdpMessage>(100);

        let node = Arc::new(Self {
            multicast_addr,
            multicast_scout,
            unicast_socket: unicast_socket.clone(),
            my_endpoints: Arc::new(Mutex::new(vec![])),
            my_subscriptions: Arc::new(Mutex::new(vec![])),

            handlers: Arc::new(DashMap::new()),
            tx_queue_sender,
            tx_queue_receiver: Arc::new(Mutex::new(tx_queue_receiver)),
            broker_addr: None,
            generic_handlers: Arc::new(Mutex::new(Vec::new())),
            tasks: Mutex::new(Vec::new()),
        });

        *node.tasks.lock().await = UdpNode::start_tasks(node.clone());

        Ok(node)
    }

    fn start_tasks(node: Arc<Self>) -> Vec<JoinHandle<()>> {
        vec![
            UdpNode::start_unicast_sender(node.clone()),
            UdpNode::start_unicast_receiver(node.clone()),
            MulticastScout::start(node.multicast_scout.clone()),
            UdpNode::start_multicast_sender(node.clone()),
        ]
    }

    /*
       send my Alive periodically via multicast
    */
    fn start_multicast_sender(node: Arc<Self>) -> JoinHandle<()> {
        // To be implemented if needed
        let my_endpoints = node.my_endpoints.clone();
        let my_subscriptions = node.my_subscriptions.clone();
        let unicast_socket = node.unicast_socket.clone();
        let multicast_addr = node.multicast_addr.clone();
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(5));
            // Target address for multicast

            loop {
                interval.tick().await;

                // 1. Send Heartbeat
                let mut alive = Alive::default();
                let my_endpoints = my_endpoints.lock().await;
                let my_subscriptions = my_subscriptions.lock().await;
                alive.endpoints = Some(my_endpoints.clone());
                alive.subscriptions = Some(my_subscriptions.clone());

                if let Ok(payload) = alive.json_serialize() {
                    let packet = UdpMessage {
                        src: None,
                        dst: None,
                        msg_type: Some(Alive::MSG_TYPE.to_string()),
                        payload: Some(payload),
                    };
                    // We send this via our standard TX queue, aiming at the Multicast Addr
                    debug!("MC Send Alive {:?}", alive);
                    let data = UdpMessage::cbor_serialize(&packet).unwrap();
                    let _ = unicast_socket.send_to(&data, &node.multicast_addr).await;
                }
            }
        })
    }

    fn start_unicast_sender(node: Arc<Self>) -> JoinHandle<()> {
        let send_socket = node.unicast_socket.clone();
        let tx_queue_receiver = node.tx_queue_receiver.clone();
        tokio::spawn(async move {
            while let Some(udp_message) = tx_queue_receiver.lock().await.recv().await {
                let target = if let Some(dst_endpoint) = &udp_message.dst {
                    if let Some(addr) = node.multicast_scout.endpoint_to_addr(dst_endpoint) {
                        addr
                    } else {
                        info!("Unknown endpoint: {}", dst_endpoint);
                        continue;
                    }
                } else {
                    info!("No destination endpoint specified");
                    continue;
                };
                info!("UC Send {:?} to {:?}", udp_message.msg_type, target);
                if let Ok(data) = udp_message.cbor_serialize() {
                    if let Err(e) = send_socket.send_to(&data, target).await {
                        error!("Send error: {}", e);
                    }
                }
            }
        })
    }

    fn start_unicast_receiver(node: Arc<Self>) -> JoinHandle<()> {
        let generic_handlers = node.generic_handlers.clone();
        let recv_socket = node.unicast_socket.clone();
        let handlers = node.handlers.clone();
        tokio::spawn(async move {
            let mut buf = [0u8; 65535];
            loop {
                if let Ok((len, addr)) = recv_socket.recv_from(&mut buf).await {
                    let data: Vec<u8> = buf[..len].to_vec();
                    if let Ok(packet) = UdpMessage::cbor_deserialize(&data) {
                        let gen_handlers = generic_handlers.clone();
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
                        if let Some(handler) = handlers.get(&packet.msg_type.unwrap()) {
                            // We spawn here so we don't block the receive loop
                            let handler_clone = node.clone(); // Usually handler logic needs state
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
        })
    }

    pub fn add_handler<H>(&self, msg_type: &str, handler: H)
    where
        H: MessageHandler + 'static,
    {
        self.handlers
            .insert(msg_type.to_string(), Box::new(handler));
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
        src: &str,
        dest_id: &str,
        msg: T,
    ) -> anyhow::Result<()> {
        let payload = msg.json_serialize()?;

        let packet = UdpMessage {
            src: Some(src.to_string()),
            dst: Some(dest_id.to_string()),
            msg_type: Some(T::MSG_TYPE.to_string()),
            payload: Some(payload),
        };

        self.tx_queue_sender
            .send(packet)
            .await
            .map_err(|_| anyhow::anyhow!("Queue Closed"))?;
        Ok(())
    }

    pub fn sender(&self) -> mpsc::Sender<UdpMessage> {
        self.tx_queue_sender.clone()
    }

    pub async fn add_generic_handler<H>(&self, handler: H)
    where
        H: UdpMessageHandler + 'static,
    {
        self.generic_handlers.lock().await.push(Box::new(handler));
    }

    pub async fn get_subscriptions(&self) -> Vec<String> {
        let subs = self.my_subscriptions.lock().await;
        subs.clone()
    }

    pub async fn add_endpoint(&self, endpoint: &str) {
        let mut my_endpoints = self.my_endpoints.lock().await;
        if !my_endpoints.contains(&endpoint.to_string()) {
            my_endpoints.push(endpoint.to_string());
        }
    }

    pub async fn add_subscription(&self, subscription: &str) {
        let mut my_subscriptions = self.my_subscriptions.lock().await;
        if !my_subscriptions.contains(&subscription.to_string()) {
            my_subscriptions.push(subscription.to_string());
        }
    }
}
