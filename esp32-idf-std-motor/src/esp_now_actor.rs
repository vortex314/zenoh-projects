extern crate alloc;

use std::mem::transmute;

use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use alloc::string::String;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};

use embassy_time::{Duration, Instant};
// use esp_backtrace as _;

use esp_idf_svc::espnow::{EspNow, PeerInfo};
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::hal::modem::Modem;
use esp_idf_svc::hal::task::block_on;
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::sys::{esp_now_recv_info, esp_now_recv_info_t, wifi_pkt_rx_ctrl_t};
use esp_idf_svc::wifi::*;

use log::{debug, error, info};

use anyhow::Result;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::fnv;
use msg::MsgHeader;

pub const MAC_BROADCAST: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

#[derive(Clone, Debug)]
pub enum EspNowEvent {
    Rxd { peer: [u8; 6], data: Vec<u8> },
}

#[derive(Clone)]
pub enum EspNowCmd {
    Txd { peer: [u8; 6], data: Vec<u8> },
    Connect { peer: [u8; 6] },
    Disconnect,
}

enum TimerId {
    BroadcastTimer = 1,
}

pub struct EspNowActor {
    cmds: CmdQueue<EspNowCmd>,
    events: EventHandlers<EspNowEvent>,
    timers: Timers,
    esp_now: EspNow<'static>,
    sender : async_channel::Sender<EspNowEvent>,
    receiver: async_channel::Receiver<EspNowEvent>,
    eventloop : EspSystemEventLoop,
    wifi_driver : esp_idf_svc::wifi::WifiDriver<'static>,
}

impl Actor<EspNowCmd, EspNowEvent> for EspNowActor {
    async fn run(&mut self) {
        info!("EspNowActor started");
        self.timers.add_timer(Timer::new_repeater(
            TimerId::BroadcastTimer as u32,
            Duration::from_millis(1_000),
        ));
        loop {
            match select3(self.cmds.next(), self.timers.alarm(), self.receiver.recv()).await {
                First(msg) => {
                    if let Err(x) = self.on_cmd_message(msg.unwrap()).await {
                        error!("Error: {:?}", x);
                    }
                }
                Second(idx) => {
                    self.on_timeout(idx).await;
                }
                Third(msg) => {
                    self.events.handle(&msg.unwrap());
                }
            }
        }
    }
    fn add_listener(&mut self, listener: Box<dyn Handler<EspNowEvent>>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<EspNowCmd>> {
        self.cmds.handler()
    }
}

impl EspNowActor {
    pub fn new(modem: Modem) -> Result<EspNowActor> {
        let nvs = EspDefaultNvsPartition::take()?;
        let eventloop = EspSystemEventLoop::take()?;
        let mut wifi_driver =
            esp_idf_svc::wifi::WifiDriver::new(modem, eventloop.clone(), Some(nvs))?;
        info!("Wifi driver created");

        wifi_driver.start()?;
        info!("Wifi driver started");

        let _sub = {
            eventloop
                .subscribe::<WifiEvent, _>(move |event| {
                    info!("Wifi event ===> {:?}", event);
                })
                .unwrap()
        };

        let esp_now = esp_idf_svc::espnow::EspNow::take()?;

        let (sender, receiver) = async_channel::bounded(5);
        let mut sender_clone = sender.clone();

        esp_now.register_recv_cb(move |mac, data| {
            let _ = sender_clone.try_send(EspNowEvent::Rxd {
                peer: MAC_BROADCAST,
                data: data.to_vec(),
            });
        })?;
        esp_now.register_send_cb(|_data, status| {
            info!("Send status: {:?}", status);
        })?;
        // add broadcast peer
        esp_now.add_peer(PeerInfo {
            peer_addr: MAC_BROADCAST,
            lmk: [0; 16],
            channel: 1,
            ifidx: 0,
            encrypt: false,
            priv_: std::ptr::null_mut(),
        })?;

        //      info!("Added peer {:?}", mac_to_string(&BROADCAST_ADDRESS));
        Ok(EspNowActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            esp_now,
            sender,
            receiver,
            eventloop,
            wifi_driver,
        })
    }

    async fn broadcast(&mut self) {
        info!("Broadcasting");
        let mut header = MsgHeader::default();
        header.src = Some(fnv("lm1/motor"));
        let v = msg::cbor::encode(&header);

        self.esp_now.send(MAC_BROADCAST, &v).unwrap();
    }

    async fn on_timeout(&mut self, _id: u32) {
        info!("Timeout");
        if _id == TimerId::BroadcastTimer as u32 {
            self.broadcast().await;
        }
    }

    async fn on_cmd_message(&mut self, msg: EspNowCmd) -> Result<()> {
        let now = Instant::now();
        match msg {
            EspNowCmd::Txd { peer, data } => self.esp_now.send(peer, &data)?,
            EspNowCmd::Connect { peer: _ } => {}
            EspNowCmd::Disconnect => {}
        }
        let elapsed = now.elapsed();
        debug!("Elapsed: {:?}", elapsed);
        Ok(())
    }
}

pub fn mac_to_string(mac: &[u8; 6]) -> String {
    mac.iter()
        .map(|b| format!("{:02X}", b))
        .collect::<Vec<_>>()
        .join(":")
}
