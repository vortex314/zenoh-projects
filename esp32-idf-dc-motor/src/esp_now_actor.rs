use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use std::time::Duration;
use std::time::Instant;

use alloc::string::String;
use log::{debug, error, info};

use anyhow::Result;
use esp32_limero_std as limero;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::MsgHeader;
use msg::{fnv, MsgType};

use esp_idf_svc::espnow::EspNow;
use futures::*;
const BROADCAST_ADDRESS: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

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
    esp_now: EspNow<'static>,
    data_receiver: async_channel::Receiver<EspNowEvent>,
}



impl Actor<EspNowCmd, EspNowEvent> for EspNowActor {
    async fn run(&mut self) {

        loop {
            select! {
                msg = self.data_receiver.recv().fuse() => {
                    info!("Received {:?}", msg);
                    self.events.handle(&msg.unwrap());
                }
                cmd = self.cmds.next().fuse() => {
                    if let Err(x) = self.on_cmd_message(cmd.unwrap()).await {
                        error!("Error: {:?}", x);
                    }
                }
                _ = self.timers.alarm().fuse() => {
                    self.on_timeout(TimerId::BroadcastTimer as u32).await;
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
    pub fn new() -> Result<EspNowActor> {

        let (data_sender, data_receiver) = async_channel::unbounded();
        let esp_now = EspNow::take().unwrap();
        esp_now.register_recv_cb(|peer, data| {
            info!("Received from {:?} {:?}", peer, data);
            let peer: [u8; 6] = peer[0..6].try_into().unwrap();
            data_sender.send(EspNowEvent::Rxd {
                peer,
                rssi: 0,
                channel: 0,
                data: data.to_vec(),
            });
        });
        let mut timers = Timers::new();
        timers.add_timer(Timer::new_repeater(
            TimerId::BroadcastTimer as u32,
            Duration::from_millis(1_000),
        ));
        //      info!("Added peer {:?}", mac_to_string(&BROADCAST_ADDRESS));
        Ok(EspNowActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers,
            esp_now,
            data_receiver,
        })
    }

    async fn broadcast_alive(&mut self) -> Result<()> {
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
        self.esp_now.send(BROADCAST_ADDRESS, &v)?;
        Ok(())
    }

    async fn on_timeout(&mut self, _id: u32) {
        if _id == TimerId::BroadcastTimer as u32 {
            let _ = self.broadcast_alive().await.map_err(|e| error!("Error: {:?}", e));
        }
    }

    async fn on_cmd_message(&mut self, msg: EspNowCmd) -> Result<()> {
        let now = Instant::now();
        match msg {
            EspNowCmd::Txd { peer, data } => {
                self.esp_now.send(peer, &data)?;
            }
            EspNowCmd::Connect { peer: _ } => {}
            EspNowCmd::Disconnect => {}
            EspNowCmd::Broadcast { data } => {
                self.esp_now.send(BROADCAST_ADDRESS, &data)?;
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
