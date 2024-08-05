use core::result;

use alloc::boxed::Box;
use alloc::format;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::vec::Vec;
use client::client::MqttClient;
use client::client_config::ClientConfig;
use embassy_executor::Spawner;

use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{DynamicReceiver, DynamicSender, Sender};
use embassy_sync::pubsub::publisher::Pub;
use embassy_sync::pubsub::PubSubChannel;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use alloc::collections::BTreeMap;
use alloc::string::String;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};
use embassy_time::{with_timeout, Duration, Instant};
use esp_wifi::wifi::{WifiDevice, WifiStaDevice};
use embassy_net::{
    tcp::TcpSocket,
    {dns::DnsQueryType, Config, Stack, StackResources},
};
use packet::v5::reason_codes::ReasonCode;
use rust_mqtt::*;

use log::{debug, info};
use utils::rng_generator::CountingRng;

use crate::limero::{timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::{ActorTrait, PubSubCmd, PubSubEvent};

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct MqttActor {
    command: Sink<PubSubCmd, 3>,
    events: Source<PubSubEvent>,
    state: State,
    ping_timeouts: u32,
    msg_id: u16,
    timers: Timers,
    topic_id_counter: u16,
    stack : &'static Stack<WifiDevice<'static,WifiStaDevice>>,
}
impl MqttActor {
    pub fn new( stack: &'static Stack<WifiDevice<WifiStaDevice>>) -> MqttActor {
        MqttActor {
            command: Sink::new(),
            events: Source::new(),
            state: State::Disconnected,
            ping_timeouts: 0,
            msg_id: 0,
            timers: Timers::new(),
            topic_id_counter: 0,
            stack,
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.command.sink_ref()
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        let mut rx_buffer = [0; 4096];
        let mut tx_buffer = [0; 4096];
        loop {
            embassy_time::Timer::after(Duration::from_millis(1_000)).await;

            let mut socket = TcpSocket::new(self.stack, &mut rx_buffer, &mut tx_buffer);

            socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));
            let address = match self.stack
                .dns_query("broker.hivemq.com", DnsQueryType::A)
                .await
                .map(|a| a[0])
            {
                Ok(address) => address,
                Err(e) => {
                    info!("DNS lookup error: {e:?}");
                    continue;
                }
            };

            let remote_endpoint = (address, 1883);
            info!("connecting...");
            let connection = socket.connect(remote_endpoint).await;
            if let Err(e) = connection {
                info!("connect error: {:?}", e);
                continue;
            }
            info!("connected!");

            let mut config = ClientConfig::new(
                rust_mqtt::client::client_config::MqttVersion::MQTTv5,
                CountingRng(20000),
            );
            config.add_max_subscribe_qos(
                rust_mqtt::packet::v5::publish_packet::QualityOfService::QoS1,
            );
            config.add_client_id("clientId-8rhWgBODCl");
            config.max_packet_size = 100;
            let mut recv_buffer = [0; 80];
            let mut write_buffer = [0; 80];

            let mut client = MqttClient::<_, 5, _>::new(
                socket,
                &mut write_buffer,
                80,
                &mut recv_buffer,
                80,
                config,
            );

            match client.connect_to_broker().await {
                Ok(()) => {}
                Err(mqtt_error) => match mqtt_error {
                    ReasonCode::NetworkError => {
                        info!("MQTT Network Error");
                        continue;
                    }
                    _ => {
                        info!("Other MQTT Error: {:?}", mqtt_error);
                        continue;
                    }
                },
            }
            loop {
                
                match select(self.command.next(), self.timers.alarm(),client.recv_messsage()).await {
                    First(msg) => match msg {
                        Some(cmd) => {
                            match cmd {
                                PubSubCmd::Publish(topic, payload) => {
                                    let res = client.send_message(topic, payload).await;
                                    info!("Publish sent {:?}", res);
                                }
                                PubSubCmd::Subscribe(topic) => {
                                    let res = client.subscribe_to_topics(vec![topic]).await;
                                    info!("
                                    Subscribe sent {:?}", res);
                                }
                                PubSubCmd::Unsubscribe(topic) => {
                                    let res = client.unsubscribe_from_topic(topic).await;
                                    info!("Unsubscribe sent {:?}", res);
                                }
                                _ => {}
                            }
                        }
                        None => {
                            info!("Unexpected {:?}", msg);
                        }
                    },
                    Second(idx) => {
                        self.on_timeout(idx).await;
                        let res = client.send_ping().await;
                        info!("Ping sent {:?}", res);
                    },
                    Third(msg) => {
                        match msg {
                            Ok((topic,payload))) => {
                                self.events.emit(PubSubEvent::Publish(topic,payload));
                            }
                            Err(e) => {
                                info!("MQTT Error: {:?}", e);
                                break;
                            }
                        }
                    }
                }
            }
            let res = client.disconnect().await;
            info!("Disconnect sent {:?}", res);
        }
    }

    fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.command.sink_ref()
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
