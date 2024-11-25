use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use alloc::string::String;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};

use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex};
use embassy_time::{Duration, Instant};
// use esp_backtrace as _;
use esp_wifi::esp_now::EspNow;
use esp_wifi::esp_now::{EspNowManager, EspNowReceiver, EspNowSender, BROADCAST_ADDRESS};
use log::{debug, error, info};

use anyhow::Error;
use anyhow::Result;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::MsgHeader;
use msg::{fnv, MsgType};

#[derive(Clone, Debug)]
pub enum EspNowEvent {
    Rxd {
        peer: [u8; 6],
        data: Vec<u8>,
    },
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
    esp_now: EspNow,
    receiver : async_channel::Receiver<EspNowEvent>,
}

impl Actor<EspNowCmd, EspNowEvent> for EspNowActor {
    async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(
            TimerId::BroadcastTimer as u32,
            Duration::from_millis(1_000),
        ));
        loop {
            match select3(
                self.cmds.next(),
                self.timers.alarm(),
                self.receiver.recv(),
            )
            .await
            {
                First(msg) => {
                    if let Err(x) = self.on_cmd_message(msg.unwrap()).await {
                        error!("Error: {:?}", x);
                    }
                }
                Second(idx) => {
                    self.on_timeout(idx).await;
                }
                Third(msg) => {
                    self.events.handle(&msg);
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
    pub fn new(modem:Modem) -> Result<EspNowActor> {
        let nvs = EspDefaultNvsPartition::take()?;
        let sys_loop = EspSystemEventLoop::take()?;
        let mut wifi_driver = esp_idf_svc::wifi::WifiDriver::new(peripherals.modem, sys_loop.clone(), Some(nvs))?;
    info!("Wifi driver created");

    wifi_driver.start()?;
    info!("Wifi driver started");

    let _sub = {
        sys_loop
            .subscribe::<WifiEvent, _>(move |event| {
                info!("Wifi event ===> {:?}", event);
            })
            .unwrap()
    };

    let esp_now = esp_idf_svc::espnow::EspNow::take()?;

    let (sender,receiver) = async_channel::bounded(capacity);

    esp_now.register_recv_cb(|mac, data| {
        info!("Received {:?}: {}", mac, minicbor::display(data));
        sender.send(EspNowEvent::Rxd {
            peer: mac,
            data: data.to_vec(),
    });
    })?;
    esp_now.register_send_cb(|_data, status| {
        debug!("Send status: {:?}", status);
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
        EspNowActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            esp_now,
            receiver,
        }
    }

    async fn broadcast(&mut self) {
        let mut sender = self.sender.lock().await;
        let mut header = MsgHeader::default(); /*  {
                                                   dst: None,
                                                   src: Some(fnv("lm/motor")),
                                                   msg_type: MsgType::Alive,
                                                   msg_id: None,
                                                   qos:None,
                                                   return_code: None,
                                               };*/
        header.msg_type = MsgType::Alive;
        header.src = Some(fnv("lm1/motor"));
        let v = msg::cbor::encode(&header);
        let status = sender.send_async(&BROADCAST_ADDRESS, &v).await;
        if status.is_err() {
            error!("Send broadcast status: {:?}", status);
        };
    }

    async fn on_timeout(&mut self, _id: u32) {
        if _id == TimerId::BroadcastTimer as u32 {
            self.broadcast().await;
        }
    }

    async fn on_cmd_message(&mut self, msg: EspNowCmd) -> Result<()> {
        let now = Instant::now();
        match msg {
            EspNowCmd::Txd { peer, data } => {
                self.esp_now.send(peer,data)
            }
            EspNowCmd::Connect { peer: _ } => {}
            EspNowCmd::Disconnect => {}
            EspNowCmd::Broadcast { data } => {
                let mut sender = self.sender.lock().await;
                sender
                    .send_async(&BROADCAST_ADDRESS, &data)
                    .await
                    .map_err(|e| Error::msg(format!("Failed to send data: {:?}", e)))?;
            }
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


