extern crate alloc;

use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use alloc::string::String;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};

use embassy_time::{Duration, Instant};
// use esp_backtrace as _;

use esp_idf_svc::espnow::{EspNow, PeerInfo, SendStatus};
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::hal::modem::Modem;
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::wifi::*;

use log::{debug, error, info};

use anyhow::Result;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use minicbor::{Decode, Encode};
use msg::fnv;

pub const MAC_BROADCAST: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

#[derive(Encode, Decode, Clone, Debug, Default)]
#[cbor(map)]
struct EspNowProps {
    #[n(0)]
    pub name: Option<String>,
}

#[derive(Clone, Debug)]
pub enum EspNowEvent {
    Rxd { peer: [u8; 6], data: Vec<u8> },
}

#[derive(Clone)]
pub enum EspNowCmd {
    Txd { peer: [u8; 6], data: Vec<u8> },
}

enum TimerId {
    BroadcastTimer = 1,
}

pub struct EspNowActor {
    cmds: CmdQueue<EspNowCmd>,
    events: EventHandlers<EspNowEvent>,
    timers: Timers,
    esp_now: EspNow<'static>,
    sender: async_channel::Sender<EspNowEvent>,
    receiver: async_channel::Receiver<EspNowEvent>,
    eventloop: EspSystemEventLoop,
    wifi_driver: esp_idf_svc::wifi::WifiDriver<'static>,
}

impl Actor<EspNowCmd, EspNowEvent> for EspNowActor {
    async fn run(&mut self) {
        info!("EspNowActor started");
        self.timers.add_timer(Timer::new_repeater(
            TimerId::BroadcastTimer as u32,
            Duration::from_millis(10000),
        ));
        loop {
            match select3(self.cmds.next(), self.timers.alarm(), self.receiver.recv()).await {
                First(msg) => {
                    if let Some(msg) = msg {
                        let _ = self.on_cmd_message(msg).await;
                    }
                }
                Second(idx) => {
                    self.on_timeout(idx).await;
                }
                Third(msg) => {
                    msg.map(|msg| self.events.handle(&msg));
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
              //      info!("Wifi event ===> {:?}", event);
                })
                .expect("Failed to subscribe to wifi events")
        };

        let esp_now = esp_idf_svc::espnow::EspNow::take()?;

        let (sender, receiver) = async_channel::bounded(5);
        let sender_clone = Box::leak(Box::new(sender.clone()));

        esp_now.register_recv_cb(move |mac, data| {
            let _ = sender_clone.try_send(EspNowEvent::Rxd {
                peer: <&[u8; 6]>::try_from(mac).unwrap().clone(),
                data: data.to_vec(),
            });
        })?;
        esp_now.register_send_cb(|_data, status| match status {
            SendStatus::SUCCESS => {}
            _ => {
                info!("Send status: {:?}", status);
            }
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
        let mut msg = msg::Msg::default();
        let mut pub_msg = EspNowProps::default();
        pub_msg.name = Some("esp_now_actor".to_string());
        msg.src = Some(fnv("lm1/esp_now"));
        msg.pub_req = Some(minicbor::to_vec(pub_msg).expect("Encode failed"));

        let v = msg::cbor::encode(&msg);
        info!("Broadcast : {}", msg);

        self.esp_now.send(MAC_BROADCAST, &v).expect("Send failed");
    }

    async fn send_info(&mut self) {
        let mut msg = msg::Msg::default();
        let mut info_map = msg::InfoMap::default();
        info_map.id = 0;
        info_map.name = Some("esp_now_actor".to_string());
        info_map.desc = Some("esp_now_actor as Actor".to_string());
        info_map.prop_type = Some(msg::PropType::STR);
        info_map.prop_mode = Some(msg::PropMode::Read);

        msg.src = Some(fnv("lm1/esp_now"));
        msg.info_reply = Some(info_map);

        let v = msg::cbor::encode(&msg);
        info!("Sending : {}", msg);

        self.esp_now.send(MAC_BROADCAST, &v).expect("Send failed");
    }

    async fn on_timeout(&mut self, _id: u32) {
        if _id == TimerId::BroadcastTimer as u32 {
            self.send_info().await;
        }
    }

    async fn on_cmd_message(&mut self, msg: EspNowCmd) -> Result<()> {
        let now = Instant::now();
        match msg {
            EspNowCmd::Txd { peer, data } => self.esp_now.send(peer, &data)?,
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
