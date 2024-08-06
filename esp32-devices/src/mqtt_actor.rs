use core::result;

use alloc::boxed::Box;
use alloc::format;
use alloc::rc::Rc;
use alloc::string::ToString;
use alloc::vec::Vec;
use client::client::MqttClient;
use client::client_config::ClientConfig;
use embassy_executor::Spawner;

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
use esp_hal::rng::Rng;
use esp_wifi::wifi::{WifiDevice, WifiStaDevice};
use packet::v5::publish_packet::QualityOfService;
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

struct Network<'a> {
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
    rx_buffer: [u8; 4096],
    tx_buffer: [u8; 4096],
    recv_buffer: [u8; 256],
    write_buffer: [u8; 256],

    client: Option<MqttClient<'a, TcpSocket<'a>, 5, CountingRng>>,
}

impl<'a> Network<'a> {
    pub fn new(stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>) -> Network {
        Network {
            stack,
            rx_buffer: [0u8; 4096],
            tx_buffer: [0u8; 4096],
            recv_buffer: [0u8; 256],
            write_buffer: [0u8; 256],
            client: None,
        }
    }
    pub async fn connect(&mut self) -> Result<(), ReasonCode> {
        let mut socket = TcpSocket::new(self.stack, &mut self.rx_buffer, &mut self.tx_buffer);
        socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

        let mut config:ClientConfig<5, CountingRng> = ClientConfig::new(
            rust_mqtt::client::client_config::MqttVersion::MQTTv5,
            CountingRng(20000),
        );
        config.add_max_subscribe_qos(rust_mqtt::packet::v5::publish_packet::QualityOfService::QoS1);
        config.add_client_id("esp32");
        config.max_packet_size = 256;

        info!("connecting...");
        let server_address = match self
            .stack
            .dns_query("test.mosquitto.org", DnsQueryType::A)
            .await
            .map(|a| a[0])
        {
            // 0 is the first address
            Ok(address) => Ok(address),
            Err(e) => {
                info!("DNS lookup error: {e:?}");
                Err(ReasonCode::UnspecifiedError)
            }
        }?;
        let server_endpoint = (server_address, 1883);
        socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));
        socket.connect(server_endpoint).await.map_err(|e| {
            info!("connect error: {:?}", e);
            ReasonCode::UnspecifiedError
        })?;

        info!("connected!");
        let mut client = MqttClient::new(
            socket,
            &mut self.write_buffer,
            256,
            &mut self.recv_buffer,
            256,
            config,
        );
        client.connect_to_broker().await?;
 //       self.client = Some(client);
        Ok(())
    }
    pub fn disconnect(&mut self) {
        let _ = self.client.as_mut().expect("").disconnect().await;
    }

    pub async fn send_message(
        &mut self,
        topic: &str,
        payload: &[u8],
        qos: QualityOfService,
        retain: bool,
    ) {
        let _ = self.client
            .as_mut()
            .expect("")
            .send_message(topic, payload, qos, retain).await;
    }

    pub async fn subscribe_to_topic(&mut self, topic: &str) {
        let _ = self.client.as_mut().expect("").subscribe_to_topic(topic).await;
    }

    pub async fn unsubscribe_from_topic(&mut self, topic: &str) {
        let _ = self.client
            .as_mut()
            .expect("")
            .unsubscribe_from_topic(topic).await;
    }

    pub async fn receive_message(&mut self) -> Result<(&str, &[u8]), ReasonCode> {
        self.client.as_mut().expect("").receive_message().await
    }

    pub async fn get_dns_address_server(&self) -> Result<IpAddress, ReasonCode> {
        match self
            .stack
            .dns_query("test.mosquitto.org", DnsQueryType::A)
            .await
            .map(|a| a[0])
        {
            // 0 is the first address
            Ok(address) => Ok(address),
            Err(e) => {
                info!("DNS lookup error: {e:?}");
                Err(ReasonCode::UnspecifiedError)
            }
        }
    }

    pub async fn send_ping(&mut self) -> Result<(), ReasonCode> {
        self.client.as_mut().expect("").send_ping().await
    }
}

pub struct MqttActor {
    cmds: Sink<PubSubCmd, 3>,
    events: Source<PubSubEvent>,
    ping_timeouts: u32,
    timers: Timers,
    network: Network<'static>,
}
impl MqttActor {
    pub fn new(stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>) -> MqttActor {
        MqttActor {
            cmds: Sink::new(),
            events: Source::new(),
            ping_timeouts: 0,
            timers: Timers::new(),
            network: Network::new(stack),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub async fn handle_cmd(&mut self, cmd: PubSubCmd) {
        match cmd {
            PubSubCmd::Connect { client_id: _ } => {}
            PubSubCmd::Disconnect => {
                self.network.disconnect();
            }
            PubSubCmd::Publish { topic, payload } => {
                let _ = self.network
                    .send_message(topic.as_str(), &payload, QualityOfService::QoS1, false).await;
            }
            PubSubCmd::Subscribe { topic } => {
                let _ = self.network.subscribe_to_topic(topic.as_str()).await;
            }
            PubSubCmd::Unsubscribe { topic } => {
                let _ = self.network.unsubscribe_from_topic(topic.as_str()).await;
            }
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(& mut self) {
       // 
        loop {
            info!("MqttActor::run");
            match select3(
                self.timers.alarm(),
                self.network.receive_message(),
                self.cmds.next(),
            )
            .await
            {
                First(_idx) => {
                    self.network.connect().await.unwrap();
                    let res = self.network.send_ping().await;
                    info!("Ping sent {:?}", res);
                }
                Second(msg) => match msg {
                    Ok((topic, payload)) => {
                        self.events.emit(PubSubEvent::Publish {
                            topic: topic.to_string(),
                            payload: payload.to_vec(),
                        });
                    }
                    Err(e) => {
                        info!("MQTT Error: {:?}", e);
                        break;
                    }
                },
                Third(cmd) => {
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
