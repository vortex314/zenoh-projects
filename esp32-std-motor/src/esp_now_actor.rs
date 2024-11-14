extern crate alloc;
use alloc::boxed::Box;
use alloc::format;
use alloc::vec::Vec;

use embassy_time::Duration;
use std::time::Instant;

use alloc::string::String;
use log::{debug, error, info};

use anyhow::Result;
use limero;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::MsgHeader;
use msg::{fnv, MsgType};

use embassy_futures::select::select3;
use embassy_futures::select::Either3;
use esp_idf_svc::espnow::EspNow;

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
    event_handlers: EventHandlers<EspNowEvent>,
    timers: Timers,
    esp_now: EspNow<'static>,
    data_receiver: async_channel::Receiver<EspNowEvent>,
    data_sender: async_channel::Sender<EspNowEvent>,
}

impl Actor<EspNowCmd, EspNowEvent> for EspNowActor {
    async fn run(&mut self) {
        loop {
            match select3(
                self.data_receiver.recv(),
                self.cmds.next(),
                self.timers.alarm(),
            )
            .await
            {
                Either3::First(msg) => {
                    let m = msg.clone();
                    info!("Received event");
                    match msg {
                        Ok(EspNowEvent::Rxd {
                            peer,
                            rssi,
                            channel,
                            data,
                        }) => {
                            info!(
                                "Received from {:?} {:?} {:?} {:?}",
                                mac_to_string(&peer),
                                rssi,
                                channel,
                                data
                            );
                            let header: MsgHeader = msg::cbor::decode(&data).unwrap();
                            info!("Header: {:?}", header);
                            self.event_handlers.handle(&EspNowEvent::Rxd {
                                peer,
                                rssi,
                                channel,
                                data,
                            });
                        }
                        Ok(EspNowEvent::Broadcast {
                            peer,
                            rssi,
                            channel,
                            data,
                        }) => {
                            info!(
                                "Received from {:?} {:?} {:?} {:?}",
                                mac_to_string(&peer),
                                rssi,
                                channel,
                                data
                            );
                            let header: MsgHeader = msg::cbor::decode(&data).unwrap();
                            info!("Header: {:?}", header);
                            self.event_handlers.handle(&EspNowEvent::Broadcast {
                                peer,
                                rssi,
                                channel,
                                data,
                            });
                        }
                        Err(e) => {
                            info!("Error: {:?}", e);
                        }
                    }
                    info!("Received {:?}", m);
                    embassy_time::Timer::after(embassy_time::Duration::from_secs(1)).await;
                    /*  let _ = msg.map(|msg| {
                        info!("Received {:?}", msg);
                        self.event_handlers.handle(&msg);
                    })
                    .map_err(|e| error!("Error: {:?}", e));*/
                }
                Either3::Second(cmd) => {
                    info!("Received command");
                    match cmd {
                        Some(cmd) => {
                            let _ = self.on_cmd_message(cmd).await;
                        }
                        _ => {
                            error!("Error: {:?}", "No command");
                        }
                    }
                }
                Either3::Third(_id) => {
                    info!("Timeout  {:?}", _id);
                    self.on_timeout(TimerId::BroadcastTimer as u32).await;
                }
            }
        }
    }
    fn add_listener(&mut self, listener: Box<dyn Handler<EspNowEvent>>) {
        self.event_handlers.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<EspNowCmd>> {
        self.cmds.handler()
    }
}

impl EspNowActor {
    pub fn new() -> Result<EspNowActor> {
        let (data_sender, data_receiver) = async_channel::unbounded();
        let sender = data_sender.clone();
        let esp_now = unsafe { EspNow::take_nonstatic() }?;
        info!("EspNow taken");
        match esp_now.register_recv_cb(|peer, data| {
            info!("Received from {:?} {}", peer, minicbor::display(data));
            let peer: [u8; 6] = peer[0..6].try_into().unwrap();
            sender.send(EspNowEvent::Rxd {
                peer,
                rssi: 0,
                channel: 0,
                data: data.to_vec(),
            }).map_err(|e| error!(" send failed : {:?}",e));
        }) {
            Ok(_) => {
                info!("Registered recv_cb");
            }
            Err(e) => {
                error!("Error: {:?}", e);
            }
        }
        let mut timers = Timers::new();
        timers.add_timer(Timer::new_repeater(
            TimerId::BroadcastTimer as u32,
            Duration::from_millis(1_000),
        ));
        //      info!("Added peer {:?}", mac_to_string(&BROADCAST_ADDRESS));
        Ok(EspNowActor {
            cmds: CmdQueue::new(5),
            event_handlers: EventHandlers::new(),
            timers,
            esp_now,
            data_receiver,
            data_sender,
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
            let _ = self
                .broadcast_alive()
                .await
                .map_err(|e| error!("Error: {:?}", e));
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
