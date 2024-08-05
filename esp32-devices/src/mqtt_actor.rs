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

pub struct MqttActor<'a> {
    cmds: Sink<PubSubCmd, 3>,
    events: Source<PubSubEvent>,
    ping_timeouts: u32,
    timers: Timers,
    stack: &'a Stack<WifiDevice<'a, WifiStaDevice>>,
    rx_buffer : &'a mut [u8; 4096],
    tx_buffer : &'a mut [u8; 4096],
    recv_buffer : &'a mut [u8; 256],
    write_buffer : &'a mut [u8; 256],
}
impl<'a> MqttActor<'a> {
    pub fn new(stack: &'a Stack<WifiDevice<WifiStaDevice>>) -> MqttActor<'a> {
        MqttActor {
            cmds: Sink::new(),
            events: Source::new(),
            ping_timeouts: 0,
            timers: Timers::new(),
            stack,
            rx_buffer: &mut [0; 4096],
            tx_buffer:  &mut [0; 4096],
            recv_buffer:  &mut [0; 256],
            write_buffer: &mut [0; 256],
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub async fn handle_cmd(
        &mut self,
        cmd: PubSubCmd,
        client: &mut MqttClient<'static, TcpSocket<'static>, 5, CountingRng>,
    ) {
        match cmd {
            PubSubCmd::Connect { client_id: _ } => {}
            PubSubCmd::Disconnect => {
                client.disconnect();
            }
            PubSubCmd::Publish { topic, payload } => {
                client.send_message(topic.as_str(), &payload, QualityOfService::QoS1, false);
            }
            PubSubCmd::Subscribe { topic } => {
                client.subscribe_to_topic(topic.as_str());
            }
            PubSubCmd::Unsubscribe { topic } => {
                client.unsubscribe_from_topic(topic.as_str());
            }
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        loop {
            info!("WifiActor::run");
            let mut client = loop {
                let cl = connect(self.stack,self.rx_buffer,self.tx_buffer,self.recv_buffer,self.write_buffer).await;
                if cl.is_ok() {
                    break cl.unwrap();
                }
            };
            match select3(
                self.timers.alarm(),
                client.receive_message(),
                self.cmds.next(),
            )
            .await
            {
                First(_idx) => {
                    let res = client.send_ping().await;
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
                    self.handle_cmd(cmd.unwrap(), &mut client).await;
                }
            }
        }
    }

    fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        todo!()
    }

    fn add_listener(&mut self, sink_ref: SinkRef<PubSubEvent>) {
        todo!()
    }
}

impl<'a> SourceTrait<PubSubEvent> for MqttActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.add_listener(sink);
    }
}

pub async fn get_dns_address_server(
    stack: &'static Stack<WifiDevice<'static, WifiStaDevice>>,
) -> Result<IpAddress, ReasonCode> {
    match stack
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

pub async fn connect<'a>(
    stack: &'a Stack<WifiDevice<'a, WifiStaDevice>>,
    rx_buffer: &'a mut [u8; 4096],
    tx_buffer: &'a mut [u8; 4096],
    recv_buffer: &'a  mut [u8; 256],
    write_buffer: &'a  mut [u8; 256],
) -> Result<MqttClient<TcpSocket<'a>, 5, CountingRng>, ReasonCode> {
    let mut socket = TcpSocket::new(stack, rx_buffer, tx_buffer);
    socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));

    let mut config = ClientConfig::new(
        rust_mqtt::client::client_config::MqttVersion::MQTTv5,
        CountingRng(20000),
    );
    config.add_max_subscribe_qos(rust_mqtt::packet::v5::publish_packet::QualityOfService::QoS1);
    config.add_client_id("esp32");
    config.max_packet_size = 256;

    info!("connecting...");
    let server_address = get_dns_address_server(stack).await?;
    let server_endpoint = (server_address, 1883);
    socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));
    socket.connect(server_endpoint).await.map_err(|e| {
        info!("connect error: {:?}", e);
        ReasonCode::UnspecifiedError
    })?;

    info!("connected!");
    let mut client = MqttClient::new(socket, write_buffer, 256, recv_buffer, 256, config);
    client.connect_to_broker().await?;
    Ok(client)
}
