use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use alloc::string::String;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::{First, Second, Third};

use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex};
use embassy_time::Duration;
use esp_backtrace as _;
use esp_wifi::esp_now::EspNow;
use esp_wifi::esp_now::{EspNowManager, EspNowReceiver, EspNowSender, PeerInfo, BROADCAST_ADDRESS};
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
        rssi: u8,
        channel: u8,
        data: Vec<u8>,
    },
    Broadcast {
        peer: [u8; 6],
        rssi: u8,
        channel: u8,
        data: Vec<u8>,
    },
}

#[derive(Clone)]
pub enum EspNowCmd {
    Txd { peer: [u8; 6], data: Vec<u8> },
    Broadcast { data: Vec<u8> },
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
    manager: &'static mut EspNowManager<'static>,
    sender: &'static mut Mutex<NoopRawMutex, EspNowSender<'static>>,
    receiver: EspNowReceiver<'static>,
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
                listener(&self.manager, &mut self.receiver),
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
    pub fn new(esp_now: EspNow<'static>) -> EspNowActor {
        info!("esp-now version {}", esp_now.get_version().unwrap());
        let (manager, sender, receiver) = esp_now.split();

        let manager = Box::leak(Box::new(manager));
        let sender = Box::leak(Box::new(Mutex::<NoopRawMutex, _>::new(sender)));

  //      info!("Added peer {:?}", mac_to_string(&BROADCAST_ADDRESS));
        EspNowActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            manager,
            sender,
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
        header.src = Some(fnv("lm/motor"));
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
        match msg {
            EspNowCmd::Txd { peer, data } => {
                let mut sender = self.sender.lock().await;
                sender
                    .send_async(&peer, &data)
                    .await
                    .map_err(|e| Error::msg(format!("Failed to send data: {:?}", e)))?;
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
        Ok(())
    }
}

async fn listener(
    _manager: &EspNowManager<'static>,
    receiver: &mut EspNowReceiver<'static>,
) -> EspNowEvent {
    let r = receiver.receive_async().await;
    debug!("source {:?}", mac_to_string(&r.info.src_address));
    debug!("rx_control {:?}", r.info.rx_control);
    debug!("Received {:?}", r.get_data());
    if r.info.dst_address == BROADCAST_ADDRESS {
        /*if !manager.peer_exists(&r.info.src_address) {
            manager
                .add_peer(PeerInfo {
                    peer_address: r.info.src_address,
                    lmk: None,
                    channel: Some(r.info.rx_control.channel as u8),
                    encrypt: false,
                })
                .unwrap();
            info!("Added peer {:?}", mac_to_string(&r.info.src_address));
        }*/
        EspNowEvent::Broadcast {
            peer: r.info.src_address,
            rssi: r.info.rx_control.rssi as u8,
            channel: r.info.rx_control.channel as u8,
            data: r.data[..r.len as usize].to_vec(),
        }
    } else {
        EspNowEvent::Rxd {
            peer: r.info.src_address,
            rssi: r.info.rx_control.rssi as u8,
            channel: r.info.rx_control.channel as u8,
            data: r.data[..r.len as usize].to_vec(),
        }
    }
}

pub fn mac_to_string(mac: &[u8; 6]) -> String {
    mac.iter()
        .map(|b| format!("{:02X}", b))
        .collect::<Vec<_>>()
        .join(":")
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
