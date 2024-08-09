use core::borrow::BorrowMut;
use core::cell::RefCell;
use core::result;

use alloc::boxed::Box;
use alloc::format;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::sync::Arc;
use alloc::vec::Vec;
use client::client::MqttClient;
use client::client_config::ClientConfig;
use embassy_executor::Spawner;

use embassy_futures::block_on;
use embassy_net::dns::Error;
use embassy_net::tcp::ConnectError;
use embassy_net::IpAddress;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{DynamicReceiver, DynamicSender, Sender};
use embassy_sync::pubsub::publisher::Pub;
use embassy_sync::pubsub::PubSubChannel;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use alloc::collections::BTreeMap;
use alloc::string::String;
use embassy_futures::select::select3;
use embassy_futures::select::Either3::First;
use embassy_futures::select::Either3::Second;
use embassy_futures::select::Either3::Third;
use embassy_net::{
    tcp::TcpSocket,
    {dns::DnsQueryType, Config, Stack, StackResources},
};
use embassy_time::{with_timeout, Duration, Instant};
use embedded_svc::mqtt;
use esp_hal::rng::Rng;
use esp_wifi::wifi::{WifiDevice, WifiStaDevice};
use minicbor::decode::info;
use packet::v5::mqtt_packet;
use packet::v5::publish_packet::QualityOfService;
use packet::v5::reason_codes::ReasonCode;
use rust_mqtt::*;

use log::{debug, error, info};
use utils::rng_generator::CountingRng;

use crate::limero::{timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::{timer, ActorTrait, PubSubCmd, PubSubEvent};

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

#[derive(Debug)]
enum MqttError {
    Network(ReasonCode),
    ConnectionError(ConnectError),
    Dns(embassy_net::dns::Error),
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

struct Buffers {
    rx_buffer: [u8; 1024],
    tx_buffer: [u8; 1024],
    write_buffer: [u8; 1024],
    recv_buffer: [u8; 1024],
}

async fn create_mqtt_client(
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
) -> Result<MqttClient<'static, TcpSocket<'static>, 5, CountingRng>, MqttError> {
    let rx_buffer = Box::leak(Box::new([0u8; 1024]));
    let tx_buffer = Box::leak(Box::new([0u8; 1024]));
    let write_buffer = Box::leak(Box::new([0u8; 1024]));
    let recv_buffer = Box::leak(Box::new([0u8; 1024]));

    let mut socket = TcpSocket::new(stack, rx_buffer.as_mut(), tx_buffer);
    socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

    let mut config: ClientConfig<5, CountingRng> = ClientConfig::new(
        rust_mqtt::client::client_config::MqttVersion::MQTTv5,
        CountingRng(20000),
    );
    config.add_max_subscribe_qos(rust_mqtt::packet::v5::publish_packet::QualityOfService::QoS1);
    config.add_client_id("esp32");
    config.max_packet_size = 1023;

    info!("DNS Query ...");
    let server_address = stack
        .dns_query("test.mosquitto.org", DnsQueryType::A)
        .await
        .map(|a| a[0])
        .map_err(|e| MqttError::Dns(e))?;

    let server_endpoint = (server_address, 1883);
    socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

    info!("Connecting to socket server ...");
    socket
        .connect(server_endpoint)
        .await
        .map_err(|e| MqttError::ConnectionError(e))?;

    info!("MQTT connect ...");
    let mut client = MqttClient::new(socket, 
        write_buffer, 1024, 
        recv_buffer, 1024, 
        config);

    client
        .connect_to_broker()
        .await
        .map_err(|err| MqttError::Network(err))?;
    Ok(client)
}

pub struct MqttActor {
    cmds: Sink<PubSubCmd, 3>,
    events: Source<PubSubEvent>,
    ping_timeouts: u32,
    timers: Timers,
    mqtt_client: Option<MqttClient<'static, TcpSocket<'static>, 5, CountingRng>>,
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
}
impl MqttActor {
    pub fn new(stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>) -> MqttActor {
        MqttActor {
            cmds: Sink::new(),
            events: Source::new(),
            ping_timeouts: 0,
            timers: Timers::new(),
            mqtt_client: None,
            stack,
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub async fn handle_cmd(&mut self, cmd: PubSubCmd) {
        info!("MqttActor::handle_cmd {:?}", cmd);
        match cmd {
            PubSubCmd::Connect { client_id: cid } => {
                info!("MqttActor::handle_cmd Connect");
                let _res = create_mqtt_client(self.stack).await;
                if _res.is_err() {
                    info!("MqttActor::handle_cmd Connect failed {:?}", _res.err());
                    self.sink_ref().send(PubSubCmd::Connect { client_id: cid });
                } else {
                    info!("MqttActor::handle_cmd Connect ok");
                    self.mqtt_client = _res.ok();
                }
            }
            PubSubCmd::Disconnect => {
                let _r = self.mqtt_client.as_mut().expect("").disconnect().await;
            }
            PubSubCmd::Publish { topic, payload } => {
                let _ = self
                    .mqtt_client
                    .as_mut()
                    .expect("")
                    .send_message(topic.as_str(), &payload, QualityOfService::QoS1, false)
                    .await;
            }
            PubSubCmd::Subscribe { topic: _ } => {
                //               let _ = self.network.subscribe_to_topic(topic.as_str()).await;
            }
            PubSubCmd::Unsubscribe { topic: _ } => {
                //               let _ = self.network.unsubscribe_from_topic(topic.as_str()).await;
            }
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        embassy_time::Timer::after(Duration::from_millis(10000)).await;

        self.timers.add_timer(timer::Timer::new_repeater(
            TimerId::PingTimer as u32,
            Duration::from_secs(10),
        ));
        info!("MqttActor::run");
        loop {
            info!("MqttActor::loop");
            embassy_time::Timer::after(Duration::from_millis(1000)).await;
            match select3(
                self.timers.alarm(),
            //    self.mqtt_client.as_mut().receive_message(),
                 async {
                    if let Some(cl) = &mut self.mqtt_client {
                        let res = cl.receive_message().await;
                        match res {
                            Ok((topic, payload)) => {
                                self.events.emit(PubSubEvent::Publish {
                                    topic: topic.to_string(),
                                    payload: payload.to_vec(),
                                });
                            }
                            Err(_) => {
                                info!("MQTT Error {:?}", res);
                                embassy_time::Timer::after(Duration::from_millis(500)).await;
                            }
                        }
                    } else {
                        info!("sleep instead of receive");
                        embassy_time::Timer::after(Duration::from_millis(1000)).await
                    }
                },
                self.cmds.next(),
            )
            .await
            {
                First(_idx) => {
                    info!("Alarm -- connecting ");
                    //                  self.mqtt_client = self.network.connect().await.ok();
                    //                  let res = self.network.send_ping().await;
                    //                  info!("Ping sent {:?}", res);
                }
                Second(_msg) => {}
                Third(cmd) => {
                    info!("MqttCmd {:?}", cmd);
                    self.handle_cmd(cmd.unwrap()).await;
                }
            }
        }
    }

    fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    fn add_listener(&mut self, sink_ref: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink_ref);
    }
}

impl SourceTrait<PubSubEvent> for MqttActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink);
    }
}
