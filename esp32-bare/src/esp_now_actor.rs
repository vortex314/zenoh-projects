use core::result;

use alloc::boxed::Box;
use alloc::format;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::vec::Vec;

use alloc::collections::BTreeMap;
use alloc::string::String;
use embassy_executor::Spawner;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};
use embassy_futures::select::{self};

use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex, mutex::MutexGuard};
use embassy_time::{with_timeout, Instant};
use embassy_time::{Duration, Ticker};
use esp_backtrace as _;
use esp_hal::peripherals::WIFI;
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
};
use esp_println::println;
use esp_wifi::esp_now::EspNow;
use esp_wifi::{
    esp_now::{EspNowManager, EspNowReceiver, EspNowSender, PeerInfo, BROADCAST_ADDRESS},
    initialize, EspWifiInitFor,
};
use log::{debug, info};

use crate::proxy_message::{Flags, ProxyMessage, ReturnCode, VecWriter};
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use pubsub::Cbor;
use pubsub::PayloadCodec;
use minicbor::Encode;
use minicbor::{decode, Decoder, Encoder, Decode};

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct EspNowActor {
    cmds: CmdQueue<ProxyMessage>,
    events: EventHandlers<ProxyMessage>,
    timers: Timers,
    manager: &'static mut EspNowManager<'static>,
    sender: &'static mut Mutex<NoopRawMutex, EspNowSender<'static>>,
    receiver: EspNowReceiver<'static>,
}

impl Actor<ProxyMessage, ProxyMessage> for EspNowActor {
    async fn run(&mut self) {
        loop {
            match select3(
                self.cmds.next(),
                self.timers.alarm(),
                listener(&self.manager, &mut self.receiver),
            )
            .await
            {
                First(msg) => {
                    // send data
                    self.on_cmd_message(msg.unwrap()).await;
                }
                Second(idx) => {
                    self.on_timeout(idx).await;
                }
                Third(msg) => {
                    self.on_rxd_message(msg).await;
                }
            }
        }
    }
    fn add_listener(&mut self, listener: Box<dyn Handler<ProxyMessage>>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<ProxyMessage>> {
        self.cmds.handler()
    }
}

impl EspNowActor {
    pub fn new(esp_now: EspNow<'static>) -> EspNowActor {
        info!("esp-now version {}", esp_now.get_version().unwrap());
        let (manager, sender, receiver) = esp_now.split();
        let manager = mk_static!(EspNowManager<'static>, manager);
        let sender = mk_static!(
            Mutex::<NoopRawMutex, EspNowSender<'static>>,
            Mutex::<NoopRawMutex, _>::new(sender)
        );
        EspNowActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            manager,
            sender,
            receiver,
        }
    }

    async fn broadcaster(sender: &'static Mutex<NoopRawMutex, EspNowSender<'static>>) {
        let mut ticker = Ticker::every(Duration::from_secs(1));
        loop {
            ticker.next().await;

            info!("Send Broadcast...");
            let mut sender = sender.lock().await;
            let status = sender.send_async(&BROADCAST_ADDRESS, b"Hello.").await;
            info!("Send broadcast status: {:?}", status);
        }
    }

    async fn on_timeout(&mut self, _id: u32) {}

    async fn on_rxd_message(&mut self, _data: Vec<u8>) {}

    pub async fn on_cmd_message(&mut self, msg: ProxyMessage) {
        let v = payload_encode(msg);
        let mut sender = self.sender.lock().await;
        let status = sender.send_async(&BROADCAST_ADDRESS, &v).await;
        info!("TXD >> {}", status);
    }
}

async fn listener(
    manager: &EspNowManager<'static>,
    receiver: &mut EspNowReceiver<'static>,
) -> Vec<u8> {
    let r = receiver.receive_async().await;
    debug!("source {:?}", mac_to_string(r.info.src_address));
    debug!("rx_control {:?}", r.info.rx_control);
    debug!("Received {:?}", r.get_data());
    if r.info.dst_address == BROADCAST_ADDRESS {
        if !manager.peer_exists(&r.info.src_address) {
            manager
                .add_peer(PeerInfo {
                    peer_address: r.info.src_address,
                    lmk: None,
                    channel: None,
                    encrypt: false,
                })
                .unwrap();
            info!("Added peer {:?}", r.info.src_address);
        }
    }
    r.data.to_vec()
}

fn mac_to_string(mac: [u8; 6]) -> String {
    mac.iter()
        .map(|b| format!("{:02X}", b))
        .collect::<Vec<_>>()
        .join(":")
}

pub fn payload_encode<X>(v: X) -> Vec<u8>
where
    X: Encode<()>,
{
    let mut buffer = Vec::<u8>::new();
    let mut encoder = Encoder::new(&mut buffer);
    let _x = encoder.encode(v);
    _x.unwrap().writer().to_vec()
}

pub fn payload_decode<'a, T>(v: &'a Vec<u8>) -> Result<T, decode::Error>
where
    T: Decode<'a, ()>,
{
    let mut decoder = Decoder::new(v);
    decoder.decode::<T>()
}

/*
let mut ticker = Ticker::every(Duration::from_millis(500));
    loop {
        match embassy_futures::select::select(ticker.next(), pubsub_actor.run()).await {
            embassy_futures::select::Either::First(_) => {
                let peer = match manager.fetch_peer(false) {
                    Ok(peer) => peer,
                    Err(_) => {
                        if let Ok(peer) = manager.fetch_peer(true) {
                            peer
                        } else {
                            continue;
                        }
                    }
                };

                info!("Send hello to peer {:?}", peer.peer_address);
                let mut sender = sender.lock().await;
                let status = sender.send_async(&peer.peer_address, b"Hello Peer.").await;
                info!("Send hello status: {:?}", status);
            }
            embassy_futures::select::Either::Second(_) => {
                info!("pubsub_actor");
            }
        }
        ticker.next().await;
    } */
