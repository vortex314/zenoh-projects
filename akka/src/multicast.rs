use log::debug;
use log::error;
use log::{info, logger};

use serde::{Deserialize, Serialize};
use std::str::FromStr;
use std::{
    collections::{HashMap, VecDeque},
    error::Error,
    net::{Ipv4Addr, SocketAddr, SocketAddrV4},
    sync::Arc,
    time::Duration,
};
use tokio::{
    net::UdpSocket,
    select,
    sync::{mpsc, Mutex},
    time::{self, Instant},
};

use crate::value::Value;

use crate::actor::Actor;
use crate::actor::ActorContext;
use crate::actor::ActorRef;
// Configuration
const UDP_LISTEN_ADDR: &str = "0.0.0.0:6502";
const UDP_LISTEN_PORT: u16 = 6502;
const UDP_MULTICAST_ADDR: &str = "225.0.0.1";
const UDP_SEND_ADDR: &str = "225.0.0.1:6502"; // Example multicast address
const HTTP_SERVER_ADDR: &str = "0.0.0.0:3000";
const MAX_CACHED_MESSAGES: usize = 100;

fn str_to_ip4_addr(ip4_str: &str) -> Result<Ipv4Addr> {
    Ipv4Addr::from_str(ip4_str)
}

enum McMessage {
    AddListener(ActorRef<McActor>),
    Received(Value),
    ReceivedInternal(Value),
    Send(Value),
}

pub struct McActor {
    udp_socket: Mutex<Option<UdpSocket>>,
    listeners: Vec<ActorRef<McMessage>>,
}

impl McActor {
    pub fn new() {

    }
}



#[cfg_attr(feature = "async-trait", ractor::async_trait)]
impl Actor for McActor {
    // An actor has a message type
    type Message = McMessage;
        type Error=String;

    // and (optionally) internal state

    async fn prestart(&mut self) -> Result<(), Self::Error> {
        let udp_socket = UdpSocket::bind(UDP_LISTEN_ADDR)
            .await
            .expect("Failed to create std UDP socket");
        udp_socket
            .join_multicast_v4(
                Ipv4Addr::from_str("225.0.0.1").unwrap(),
                Ipv4Addr::from_str("0.0.0.0").unwrap(),
            )
            .expect("Failed to join multicast group");
        // reuse addr to allow multiple instances to bind to the same port

        let udp_socket = Arc::new(udp_socket);
        // Clone for UDP receiver task
        let udp_receiver_socket = udp_socket.clone();

        tokio::spawn(async move {
            let mut buf = [0; 1024];
            loop {
                select! {
                    r = udp_receiver_socket.recv_from(&mut buf) => {
                        match r {
                    Ok((len, src)) => {
                         let message = String::from_utf8_lossy(&buf[..len]).to_string();
                            info!("Received UDP from {}: {}", src, message);

                        let value = Value::from_json(message);
                        myself.cast(McMessage::ReceivedInternal(value.unwrap()));


                    },
                    Err(e) => error!("UDP receive error: {}", e),
                }
                    },
                    _ = time::sleep(Duration::from_secs(1)) => {

                    }
                };
            }
        });

        Ok(McActor {
            udp_socket,
            listeners: Vec::new(),
        });
        Ok(())
    }
        async fn handle_message(&mut self, msg: Self::Message, ctx: &mut ActorContext<Self>) -> Result<(), Self::Error>
 {
        match msg {
            McMessage::Received(v) => {} // only as output
            McMessage::ReceivedInternal(v) => {
                // broadcast
                for listener in self.listeners {
                    listener.send_message(McMessage::Received(v));
                }
            }
            McMessage::Send(v) => {}
            McMessage::AddListener(ar) => state.listeners.push(ar),
        }
        Ok(())
    }

    #[doc = " Invoked when an actor is being started by the system."]
    #[doc = ""]
    #[doc = " Any initialization inherent to the actor\'s role should be"]
    #[doc = " performed here hence why it returns the initial state."]
    #[doc = ""]
    #[doc = " Panics in `pre_start` do not invoke the"]
    #[doc = " supervision strategy and the actor won\'t be started. [Actor]::`spawn`"]
    #[doc = " will return an error to the caller"]
    #[doc = ""]
    #[doc = " * `myself` - A handle to the [ActorCell] representing this actor"]
    #[doc = " * `args` - Arguments that are passed in the spawning of the actor which might"]
    #[doc = " be necessary to construct the initial state"]
    #[doc = ""]
    #[doc = " Returns an initial [Actor::State] to bootstrap the actor"]
    #[cfg(feature = "async-trait")]
    #[must_use]
    #[allow(
        elided_named_lifetimes,
        clippy::type_complexity,
        clippy::type_repetition_in_bounds
    )]
    fn pre_start<'life0, 'async_trait>(
        &'life0 self,
        myself: ActorRef<Self::Msg>,
        args: Self::Arguments,
    ) -> ::core::pin::Pin<
        Box<
            dyn ::core::future::Future<Output = Result<Self::State, ActorProcessingErr>>
                + ::core::marker::Send
                + 'async_trait,
        >,
    >
    where
        'life0: 'async_trait,
        Self: 'async_trait,
    {
        todo!()
    }
    
}
