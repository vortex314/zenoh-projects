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
use esp_hal::rng::Rng;
use esp_wifi::wifi::{WifiDevice, WifiStaDevice};
use packet::v5::publish_packet::QualityOfService;
use packet::v5::reason_codes::ReasonCode;
use rust_mqtt::*;

use log::{debug, error, info};
use utils::rng_generator::CountingRng;

use crate::limero::{timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::{ActorTrait, PubSubCmd, PubSubEvent};

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

struct Network {
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
    rx_buffer: [u8; 4096],
    tx_buffer: [u8; 4096],
    recv_buffer: [u8; 256],
    write_buffer: [u8; 256],

    client: Option<MqttClient<'static, TcpSocket<'static>, 5, CountingRng>>,
}

impl Network {
    pub fn new(stack: &'static Stack<WifiDevice<WifiStaDevice>>) -> Network {
        Network {
            stack,
            rx_buffer: [0u8; 4096],
            tx_buffer: [0u8; 4096],
            recv_buffer: [0u8; 256],
            write_buffer: [0u8; 256],
            client: None,
        }
    }
    pub async fn connect(&'static mut self) -> Result<(), MqttError> {
        let mut socket = TcpSocket::new(self.stack, &mut self.rx_buffer, &mut self.tx_buffer);
        socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

        let mut config: ClientConfig<5, CountingRng> = ClientConfig::new(
            rust_mqtt::client::client_config::MqttVersion::MQTTv5,
            CountingRng(20000),
        );
        config.add_max_subscribe_qos(rust_mqtt::packet::v5::publish_packet::QualityOfService::QoS1);
        config.add_client_id("esp32");
        config.max_packet_size = 256;

        info!("DNS Query ...");
        let server_address = self
            .stack
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
        let mut client = MqttClient::new(
            socket,
            &mut self.write_buffer,
            256,
            &mut self.recv_buffer,
            256,
            config,
        );
        client
            .connect_to_broker()
            .await
            .map_err(|err| MqttError::Network(err))?;
        self.client = Some(client);
        Ok(())
    }
    pub async fn disconnect(&mut self) {
        let _ = self.client.as_mut().expect("").disconnect().await;
    }

    pub async fn send_message(
        &mut self,
        topic: &str,
        payload: &[u8],
        qos: QualityOfService,
        retain: bool,
    ) {
        if self.client.is_none() {
            error!("No client for sending messages");
            return;
        }
        let _ = self
            .client
            .as_mut()
            .expect("")
            .send_message(topic, payload, qos, retain)
            .await;
    }

    pub async fn subscribe_to_topic(&mut self, topic: &str) {
        if self.client.is_none() {
            error!("No client for subscription");
            return;
        }
        let _ = self
            .client
            .as_mut()
            .expect("")
            .subscribe_to_topic(topic)
            .await;
    }

    pub async fn unsubscribe_from_topic(&mut self, topic: &str) {
        if self.client.is_none() {
            error!("No client for unsubscription");
            return;
        }
        let _ = self
            .client
            .as_mut()
            .expect("")
            .unsubscribe_from_topic(topic)
            .await;
    }

    pub async fn receive_message(&mut self) -> Result<(String, Vec<u8>), ReasonCode> {
        if self.client.is_none() {
            error!("No client for receiving messages");
            return Err(ReasonCode::UnspecifiedError);
        }
        self.client
            .as_mut()
            .expect("no client ")
            .receive_message()
            .await
            .map(|(topic, payload)| (topic.to_string(), payload.to_vec()))
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
    network: &'static mut Network,
}
impl MqttActor {
    pub fn new(stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>) -> MqttActor {
        MqttActor {
            cmds: Sink::new(),
            events: Source::new(),
            ping_timeouts: 0,
            timers: Timers::new(),
            network: Box::leak(Box::new(Network::new(stack))),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub async fn handle_cmd(&mut self, cmd: PubSubCmd) {
        match cmd {
            PubSubCmd::Connect { client_id: _ } => {}
            PubSubCmd::Disconnect => {
                let _ = (*self.network).borrow_mut().disconnect().await;
            }
            PubSubCmd::Publish { topic, payload } => {
                let _ = (*self.network)
                    .borrow_mut()
                    .send_message(topic.as_str(), &payload, QualityOfService::QoS1, false)
                    .await;
            }
            PubSubCmd::Subscribe { topic } => {
                let _ = (*self.network)
                    .borrow_mut()
                    .subscribe_to_topic(topic.as_str())
                    .await;
            }
            PubSubCmd::Unsubscribe { topic } => {
                let _ = (*self.network)
                    .borrow_mut()
                    .unsubscribe_from_topic(topic.as_str())
                    .await;
            }
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        info!("MqttActor::run");
        loop {
            info!("MqttActor::loop");
            embassy_time::Timer::after(Duration::from_millis(1000)).await;
            match select3(
                self.timers.alarm(),
                self.network.receive_message(),
                self.cmds.next(),
            )
            .await
            {
                First(_idx) => {
                    info!("Alarm -- connecting ");
                   // let _r = self.network.connect().await;
                    //                   let res = self.network.send_ping().await;
                    //                  info!("Ping sent {:?}", res);
                }
                Second(msg) => match msg {
                    Ok((topic, payload)) => {
                        self.events.emit(PubSubEvent::Publish { topic, payload });
                    }
                    Err(_) => {
                        info!("MQTT Error");
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
