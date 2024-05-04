
use alloc::rc::Rc;
use alloc::string::ToString;
use embassy_executor::Spawner;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::pubsub::publisher::Pub;
use embassy_sync::pubsub::PubSubChannel;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};
use embassy_time::{with_timeout, Duration, Timer};
use embassy_futures::select::select;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    embassy,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
    Uart,
};
use esp_println::println;
use log::info;
use alloc::collections::BTreeMap;
use alloc::string::String;

use crate::protocol;

use minicbor::decode::info;
use protocol::ProxyMessage;

static TXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
static RXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
static CMD_MSG : Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
static PUB_MSG : PubSubChannel< CriticalSectionRawMutex, ProxyMessage, 10,10,10 >= PubSubChannel::new();
pub const UART_BUFSIZE: usize = 127;
const RC_ACCEPTED: u8 = 0;
const RC_REJECTED_CONGESTION: u8 = 1;
const RC_REJECTED_INVALID_TOPIC_ID: u8 = 2;
const RC_REJECTED_NOT_SUPPORTED: u8 = 3;

/* 
    Client     ->  Server
    Subscribe -> NR
    Publish 0 ( dst/client/sys/time ) -> NR
    NR <- Publish  0 ( dst/client/sys/time )
    PubAck ( unknown id ) <- NR
    Register 0 ( dst/client/sys/time ) -> NR
    Register 1 ( dst/client/sys/time ) -> NR

    Table of topics
    0 -> dst/client/sys/time + registered flag
    1 -> src/client/sys/log + registered flag

    FSM Client
    -> Subscriber<f64> ( topic ) -> converts ProxyMessage::Publish to f64
        -> onMessage ( topic, value )
    -> PatternSubscriber ( pattern )
    -> Publisher ( topic ) --- serializes message to ProxyMessage::Publish
        -> onMessage ( topic, message )

    FSM Server
    - List Subscribers
    - List Publishers
    - List PatternSubscribers
    - List topics clients + registered flag
    - List topics servers + registered flag


    onConnReset => clear registered flags for client topics, remove all server topics
    at first use of topic , register it 
*/


/*
wait for message to send, encode and send to UART
*/
#[embassy_executor::task]
pub async fn uart_writer(mut tx: UartTx<'static, UART0>) {
    loop {
        let msg = TXD_MSG.receive().await;
        info!("Sending message {:?}", msg);
        let bytes = protocol::encode_frame(msg).unwrap();
        let _ = embedded_io_async::Write::write(&mut tx, bytes.as_slice()).await;
        let _ = embedded_io_async::Write::flush(&mut tx).await;
    }
}
/*
handle UART input, convert to message and send to channel
*/
#[embassy_executor::task]
pub async fn uart_reader(mut rx: UartRx<'static, UART0>) {
    // Declare read buffer to store Rx characters
    let mut rbuf = [0u8; UART_BUFSIZE];

    let mut message_decoder = protocol::MessageDecoder::new();
    loop {
        info!("Waiting for message");
        let r = embedded_io_async::Read::read(&mut rx, &mut rbuf[0..]).await;
        match r {
            Ok(_cnt) => {
                let v = message_decoder.decode(&mut rbuf);
                // Read characters from UART into read buffer until EOT
                for msg in v {
                    RXD_MSG.send(msg).await;
                }
            }
            Err(_) => {
                continue;
            }
        }
    }
}

#[embassy_executor::task]
pub async fn fsm_connection() {
    let mut message_handler = ProxyClient::new();
    message_handler.fsm_connection().await;
}

#[derive(PartialEq)]

enum State {
    Idle,
    WaitConnack,
    Connected,
}

struct TopicReg {
    topic : String,
    registered : bool,
}

pub struct ProxyClient {
    client_topics: BTreeMap<u16,String>,
    server_topics: BTreeMap<u16,String>,
    client_topics_registered: BTreeMap<u16,bool>,
    client_id: String,
    state: State,
    ping_timeouts: u32,
}

impl ProxyClient {
    fn new() -> ProxyClient {
        ProxyClient {
            client_topics: BTreeMap::new(),
            server_topics: BTreeMap::new(),
            client_topics_registered: BTreeMap::new(),
            client_id: "ESP32_1".to_string(),
            state: State::Idle,
            ping_timeouts: 0,
        }
    }

    async fn handler(&mut self) {
        loop {
            let _res = select (CMD_MSG.receive(), RXD_MSG.receive());
            match _res {
                Ok((cmd, msg)) => {
                    self.on_cmd_message(cmd).await;
                    self.on_rxd_message(msg).await;
                }
                Err(_) => {
                    info!("Timeout");
                }
            }
        }
    }

    async fn on_cmd_message(&mut self, msg : ProxyMessage){
        match msg {
            ProxyMessage::Publish { topic_id, message } => {
                info!("Received message on topic {} : {}", topic_id, message);
                if self.server_topics.contains_key(&topic_id){
                    TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_ACCEPTED } ).await;
                } else {
                    TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_REJECTED_INVALID_TOPIC_ID } ).await;
                }
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    async fn on_rxd_message(&mut self , msg : ProxyMessage){
        match msg {
            ProxyMessage::Register { topic_id, topic_name } =>{
                info!("Registering topic {} with id {}", topic_name, topic_id);
                self.server_topics.insert(topic_id,topic_name);
            }
            ProxyMessage::Publish { topic_id, message } => {
                info!("Received message on topic {} : {}", topic_id, message);
                if self.server_topics.contains_key(&topic_id){
                    TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_ACCEPTED } ).await;

                } else {
                    TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_REJECTED_INVALID_TOPIC_ID } ).await;
                }
            }
            ProxyMessage::PubAck { topic_id, return_code } => {
                if return_code == RC_ACCEPTED {
                    info!("Received PubAck for topic {} with code {}", topic_id, return_code);
                } else {
                    TXD_MSG.send(ProxyMessage::Register { topic_id , topic_name:self.client_topics.get(&topic_id).unwrap().clone()}).await;
                    self.client_topics_registered.insert(topic_id,true);
                }
                info!("Received PubAck for topic {} with code {}", topic_id, return_code);
            }
            _ => {
                info!("Unexpected message {:?}", msg);
            }
        }
    }

    pub async fn fsm_connection(&mut self) {
        loop {
            let result = with_timeout(Duration::from_secs(5), RXD_MSG.receive()).await;
            match result {
                Ok(msg) => match msg {
                    ProxyMessage::ConnAck { return_code } => {
                        info!("Connected code  {}", return_code);
                        if self.state == State::WaitConnack {
                            self.state = State::Connected;
                        }
                    }
                    ProxyMessage::PingResp {} => {
                        info!("Ping response");
                    }
                    ProxyMessage::Disconnect {} => {
                        info!("Disconnected");
                        self.state = State::Idle;
                    }
                    ProxyMessage::Register { topic_id, topic_name } =>{
                        info!("Registering topic {} with id {}", topic_name, topic_id);
                        self.client_topics.insert(topic_id,topic_name);
                    }
                    ProxyMessage::Publish { topic_id, message } => {
                        info!("Received message on topic {} : {}", topic_id, message);
                        if self.server_topics.contains_key(&topic_id){
                            TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_ACCEPTED } ).await;
                        } else {
                            TXD_MSG.send(ProxyMessage::PubAck{ topic_id, return_code : RC_REJECTED_INVALID_TOPIC_ID } ).await;
                        }
                    }
                    ProxyMessage::PubAck { topic_id, return_code } => {
                        if return_code == RC_ACCEPTED {
                            info!("Received PubAck for topic {} with code {}", topic_id, return_code);
                        } else {
                            TXD_MSG.send(ProxyMessage::Register { topic_id , topic_name:self.client_topics.get(&topic_id).unwrap().clone()}).await;
                        }
                        info!("Received PubAck for topic {} with code {}", topic_id, return_code);
                    }

                    _ => {
                        info!("Unexpected message {:?}", msg);
                    }
                },
                Err(_) => match self.state {
                    State::Idle => {
                        TXD_MSG
                            .send(ProxyMessage::Connect {
                                protocol_id: 1,
                                duration: 100,
                                client_id: self.client_id.clone()  ,
                            })
                            .await;
                        self.state = State::WaitConnack;
                    }
                    State::Connected => {
                        if self.ping_timeouts > 5 {
                            TXD_MSG.send(ProxyMessage::Disconnect {}).await;
                            self.state = State::Idle;
                        } else {
                            self.ping_timeouts += 1;
                            TXD_MSG.send(ProxyMessage::PingReq {}).await;
                        }
                    }
                    State::WaitConnack => {
                        self.state = State::Idle;
                    }
                },
            }

            
        }
    }
}
