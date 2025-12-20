use anyhow::Result;
use log::info;
use minicbor::{Decode, Encode};
use std::collections::{HashMap, HashSet};
use std::io;
use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use std::sync::Arc;
use std::time::Duration;
use tokio::net::UdpSocket;
use tokio::time::interval;

#[derive(Debug, Encode, Decode)]
#[cbor(array)]
pub struct CborMessage {
    #[n(0)]  // can be null
    pub source: Option<String>,
    #[n(1)]
    pub destination: Option<String>,
    #[n(2)]
    pub message_type: String,
    #[cbor(n(3),with = "minicbor::bytes")]
    pub payload: Option<Vec<u8>>,
}

type RxCallback = Arc<dyn Fn(Vec<u8>, SocketAddr) + Send + Sync>;

struct UdpReceiver {
    unicast_recv_socket: Arc<UdpSocket>,
    multicast_recv_socket: Arc<UdpSocket>,
    callback: Option<RxCallback>,
}

impl UdpReceiver {
    pub fn new(unicast_recv_socket: Arc<UdpSocket>, multicast_recv_socket: Arc<UdpSocket>) -> Self {
        Self {
            unicast_recv_socket,
            multicast_recv_socket,
            callback: None,
        }
    }

    pub fn set_callback(&mut self, callback: RxCallback) {
        self.callback = Some(callback);
    }

    pub fn start(&self) {
        let uc_socket = self.unicast_recv_socket.clone();
        let mc_socket = self.multicast_recv_socket.clone();
        let callback = self.callback.clone();

        tokio::spawn(async move {
            let mut uc_buf = [0u8; 2048];
            let mut mc_buf = [0u8; 2048];

            loop {
                tokio::select! {
                    result = uc_socket.recv_from(&mut uc_buf) => {
                        let (len, addr) = result.unwrap();
                        info!("RXD unicast   [{} bytes] [{}] [{}]", len, addr, minicbor::display(&uc_buf[..len]));
                        if let Some(cb) = &callback {
                            cb(uc_buf[..len].to_vec(), addr);
                        }
                    }
                    result = mc_socket.recv_from(&mut mc_buf) => {
                        let (len, addr) = result.unwrap();
                         info!("RXD multicast   [{} bytes] [{}] [{}]", len, addr, minicbor::display(&mc_buf[..len]));
                        if let Some(cb) = &callback {
                            cb(mc_buf[..len].to_vec(), addr);
                        }
                    }
                }
            }
        });
    }
}

pub struct UdpCborClient {
    unicast_send_socket: Arc<UdpSocket>,
    unicast_recv_socket: Arc<UdpSocket>,
    multicast_send_socket: Arc<UdpSocket>,
    multicast_recv_socket: Arc<UdpSocket>,
    multicast_addr: SocketAddr,

    /// Logical source name -> socket address
    source_map: HashMap<String, SocketAddr>,

    /// Supported outbound message types
    supported_types: HashSet<String>,

    /// This node's logical name
    local_source: String,
    receiver: Option<UdpReceiver>,
}

impl UdpCborClient {
    /// Bind unicast socket and join multicast group
    pub async fn bind(
        unicast_addr_str: &str,
        multicast_addr_str: &str,
        local_source: String,
    ) -> io::Result<Self> {
        let unicast_socket = UdpSocket::bind(unicast_addr_str).await?;
        info!("Unicast socket bound to {}", unicast_addr_str);
        let (multicast_ip_str, multicast_port_str) =
            multicast_addr_str.rsplit_once(':').ok_or_else(|| {
                io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "Invalid multicast address format",
                )
            })?;
        info!(
            "Multicast IP: {}, Port: {}",
            multicast_ip_str, multicast_port_str
        );
        let multicast_port: u16 = multicast_port_str
            .parse()
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))?;

        let multicast_ip = multicast_ip_str
            .parse::<Ipv4Addr>()
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))?;

        // Validate multicast range (IPv4 multicast is 224.0.0.0/4 → 224.0.0.0..=239.255.255.255)
        if !multicast_ip.is_multicast() {
            return Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                format!(
                    "{} is not a valid IPv4 multicast address (expected 224.0.0.0/4)",
                    multicast_ip
                ),
            ));
        }

        info!(
            "Multicast IP: {}, Port: {}",
            multicast_ip_str, multicast_port_str
        );

        let multicast_addr = SocketAddr::new(IpAddr::V4(multicast_ip), multicast_port);
        info!("Joining multicast group at {}", multicast_addr);

        let multicast_socket = UdpSocket::bind(SocketAddr::new(
            IpAddr::V4(Ipv4Addr::UNSPECIFIED),
            multicast_port,
        ))
        .await?;
        info!("Multicast socket bound to port {}", multicast_port);
        multicast_socket.join_multicast_v4(multicast_ip, Ipv4Addr::UNSPECIFIED)?;

        info!("Joined multicast group at {}", multicast_addr);

        multicast_socket.set_multicast_ttl_v4(1)?;

        let uc_socket = Arc::new(unicast_socket);
        let mc_socket = Arc::new(multicast_socket);

        let receiver = UdpReceiver::new(uc_socket.clone(), mc_socket.clone());

        Ok(Self {
            unicast_send_socket: uc_socket.clone(),
            unicast_recv_socket: uc_socket,
            multicast_send_socket: mc_socket.clone(),
            multicast_recv_socket: mc_socket,
            multicast_addr,
            source_map: HashMap::new(),
            supported_types: HashSet::new(),
            local_source,
            receiver: Some(receiver),
        })
    }

    pub fn set_callback(&mut self, callback: RxCallback) {
        if let Some(receiver) = &mut self.receiver {
            receiver.set_callback(callback);
            receiver.start();
        }
    }

    /// Register a supported outbound message type
    pub fn register_message_type(&mut self, message_type: &str) {
        self.supported_types.insert(message_type.to_string());
    }

    pub async fn send_unicast(&self, addr: SocketAddr, data: &[u8]) -> io::Result<usize> {
        self.unicast_send_socket.send_to(data, addr).await
    }

    pub async fn send_multicast(&self, data: &[u8]) -> io::Result<usize> {
        self.multicast_send_socket
            .send_to(data, self.multicast_addr)
            .await
    }

    /// Send a unicast CBOR message to a learned destination
    pub async fn send_logical(
        &self,
        destination: &str,
        message_type: &str,
        payload: &Vec<u8>,
    ) -> io::Result<()> {
        let addr = self.source_map.get(destination).ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::NotFound,
                format!("Unknown destination: {}", destination),
            )
        })?;

        let msg = CborMessage {
            source: Some(self.local_source.clone()),
            destination: if destination.len() > 0 { Some(destination.to_string()) } else { None },
            message_type: message_type.to_string(),
            payload: Some(payload.clone()),
        };

        let encoded =
            minicbor::to_vec(&msg).map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))?;

        self.unicast_send_socket.send_to(&encoded, addr).await?;
        Ok(())
    }

    fn decode_and_learn(
        map: &mut HashMap<String, SocketAddr>,
        data: &[u8],
        addr: SocketAddr,
    ) -> io::Result<CborMessage> {
        let msg: CborMessage =
            minicbor::decode(data).map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))?;

        if let Some(source) = &msg.source {
            map.insert(source.clone(), addr);
        }
        Ok(msg)
    }

    /// Periodically announce presence and supported message types via multicast
    pub async fn announce_task(self: &Self, period: Duration) {
        let mut ticker = interval(period);

        loop {
            ticker.tick().await;

            let payload = self
                .supported_types
                .iter()
                .cloned()
                .collect::<Vec<_>>()
                .join(",");

            let msg = CborMessage {
                source: Some(self.local_source.clone()),
                destination: Some("multicast".to_string()),
                message_type: "announce".to_string(),
                payload: Some(payload.into_bytes()),
            };

            if let Ok(encoded) = minicbor::to_vec(&msg) {
                let _ = self
                    .multicast_send_socket
                    .send_to(&encoded, self.multicast_addr)
                    .await;
            }
        }
    }

    /// Resolve logical source name to socket address
    pub fn resolve(&self, source: &str) -> Option<SocketAddr> {
        self.source_map.get(source).copied()
    }
}
