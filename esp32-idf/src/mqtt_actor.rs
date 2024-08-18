use core::borrow::BorrowMut;
use core::cell::RefCell;
use core::result;
use std::sync::{Arc, RwLock};
use std::time::{Duration, Instant};

use embedded_svc::http::{client, Query};
use esp_idf_svc::mdns::{EspMdns, QueryResult};
use esp_idf_svc::mqtt::client::{
    EspAsyncMqttClient, EspAsyncMqttConnection, EspMqttClient, EspMqttEvent, EventPayload, LwtConfiguration, MqttClientConfiguration, QoS
};

use futures::FutureExt;
use log::{debug, error, info};

use crate::limero::{
    async_wait_millis, timer::Timer, timer::Timers, Sink, SinkRef, SinkTrait, Source, SourceTrait,
};
use crate::{timer, ActorTrait, PubSubCmd, PubSubEvent};
use anyhow::Result;

#[derive(PartialEq)]

enum State {
    Disconnected,
    Connected,
}

#[derive(Clone, Debug)]
pub enum MqttEvent {
    Connected,
    Disconnected,
    Publish { topic: String, payload: Vec<u8> },
}

enum TimerId {
    PingTimer = 1,
    ConnectTimer = 2,
    ConnectionTimer = 3,
}

pub struct MqttActor {
    cmds: Sink<PubSubCmd>,
    events: Arc<RwLock<Source<PubSubEvent>>>,
    timers: Timers,
    mqtt_host : String,
    mqtt_client: Option<EspAsyncMqttClient>,
    mqtt_connection : Option<EspAsyncMqttConnection>,
    client_id: String,
    uptime_topic: String,
    subscribe_topic: String,
    lwt_topic: String,
    lwt_payload_online: Vec<u8>,
    lwt_payload_offline: Vec<u8>,
    esp_mdns: EspMdns,
}
impl MqttActor {
    pub fn new(client_id: &str, mqtt_host: &str) -> MqttActor {
        // Set up handle for MQTT Config
        let events = Arc::new(RwLock::new(Source::new()));
        let cmds = Sink::new();
        let lwt_topic = format!("src/{}/sys/lwt", client_id);
        let lwt_payload_offline = "offline".as_bytes().to_vec();
        let lwt_payload_online = "online".as_bytes().to_vec();
        let lwt_topic = format!("src/{}/sys/lwt", client_id);
        let subscribe_topic = format!("dst/{}/#", client_id);
        let uptime_topic = format!("src/{}/sys/uptime", client_id);


        MqttActor {
            cmds,
            events,
            timers: Timers::new(),
            mqtt_host: mqtt_host.to_string(),
            mqtt_client:None,
            mqtt_connection:None,
            client_id: client_id.to_string(),
            uptime_topic,
            subscribe_topic,
            lwt_topic,
            lwt_payload_online:lwt_payload_online.clone(),
            lwt_payload_offline:lwt_payload_offline.clone(),
            esp_mdns: EspMdns::take().unwrap(),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub async fn create_client(&mut self) -> Result<(EspAsyncMqttClient,EspAsyncMqttConnection)> {
        let mut mqtt_config = MqttClientConfiguration::default();
        let lwt_topic = format!("src/{}/sys/lwt", self.client_id);
        let lwt_config = LwtConfiguration {
            topic: &lwt_topic   ,
            payload: &self.lwt_payload_offline.clone(),
            qos: QoS::AtLeastOnce,
            retain: false,
        };
        mqtt_config.lwt = Some(lwt_config);
        mqtt_config.keep_alive_interval = Some(Duration::from_secs(3));
        mqtt_config.reconnect_timeout = Some(Duration::from_secs(1));
        let mqtt_broker = format!("mqtt://{}", self.mqtt_host).as_str();
        let (client,conn) = EspAsyncMqttClient::new(self.mqtt_host.as_str(), &mqtt_config)?;
        Ok((client,conn))
    }

    pub async fn handle_cmd(&mut self, cmd: PubSubCmd) -> Result<()> {
        match cmd {
            PubSubCmd::Publish { topic, payload } => {
                self.mqtt_client.as_mut().map( |cl| cl.publish(topic.as_str(), QoS::AtLeastOnce, false, &payload));
                  //  .publish(topic.as_str(), QoS::AtLeastOnce, false, &payload)?;
            }
            PubSubCmd::Subscribe { topic } => {
                self.mqtt_client.as_mut().map( |cl| cl.subscribe(topic.as_str(), QoS::AtLeastOnce));
            }
            PubSubCmd::Unsubscribe { topic } => {
                self.mqtt_client.as_mut().map( |cl| cl.unsubscribe(topic.as_str()));
            }
            PubSubCmd::Connect { client_id: _ } => {
                let ( mqtt_client,mqtt_connection) = self.create_client().await?;
                self.mqtt_client = Some(mqtt_client);
                self.mqtt_connection = Some(mqtt_connection);

                info!("Subcribe to {}", self.subscribe_topic);
                self.mqtt_client.as_mut().map( |cl| cl.subscribe(&self.subscribe_topic, QoS::AtLeastOnce));
                self.mqtt_client.as_mut().map( |cl| cl.publish(&self.lwt_topic, QoS::AtLeastOnce, false, &self.lwt_payload_online));
            }
            cmd => {
                return Err(anyhow::anyhow!("Command not implemented {:?}", cmd));
            }
        }
        Ok(())
    }
    async fn handle_event(&mut self, event: EspMqttEvent<'_>) {
        {
            match event.payload() {
                EventPayload::BeforeConnect => {
                    info!("Before Connect");
                    self.events.write().unwrap().emit(PubSubEvent::Disconnected);
                }
                EventPayload::Connected(b) => {
                    info!("Connect {}", b);
                    self.events.write().unwrap().emit(PubSubEvent::Connected);
                    self.cmds.sink_ref().send(PubSubCmd::Connect {
                        client_id: self.client_id.clone(),
                    });
                }
                EventPayload::Disconnected => {
                    info!("Disconnected");
                    self.events.write().unwrap().emit(PubSubEvent::Disconnected);
                }
                EventPayload::Received {
                    id,
                    topic,
                    data,
                    details,
                } => {
                    info!("Received {:?} {:?} {:?}", id, topic, details);
                    self.events.write().unwrap().emit(PubSubEvent::Publish {
                        topic: topic.unwrap().to_string(),
                        payload: data.to_vec(),
                    });
                }
                EventPayload::Published(_id) => {}
                _r => {
                    info!("Other Event {:?}", _r);
                }
            }
        }
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        info!("MqttActor::run");
        let res = self
            .esp_mdns
            .query_a("pcthink.local", Duration::from_secs(3));
        match res {
            Ok(ip) => {
                info!("Query Result {:?}", ip);
            }
            Err(e) => {
                info!("Query Result Error {:?}", e);
            }
        }

        self.timers.add_timer(timer::Timer::new_repeater(
            TimerId::PingTimer as u32,
            Duration::from_secs(1),
        ));
        loop {
            futures::select! {
                cmd = self.cmds.next().fuse() => {
                    if let Some(cmd) = cmd {
                        let r  = self.handle_cmd(cmd).await;
                        if r.is_err() {
                            error!("MqttActor::handle_cmd {:?}",r);
                        }
                    }
                }
                _ = self.timers.alarm().fuse() => {
                    let line = format!("{}",std::time::UNIX_EPOCH.elapsed().unwrap().as_millis());
                    self.cmds.sink_ref().send(PubSubCmd::Publish { topic:self.uptime_topic.clone(), payload:line.as_bytes().to_vec() });
                }

            }
        }
    }

    fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    fn add_listener(&mut self, sink_ref: SinkRef<PubSubEvent>) {
        self.events
            .borrow_mut()
            .write()
            .unwrap()
            .add_listener(sink_ref);
    }
}

impl SourceTrait<PubSubEvent> for MqttActor {
    fn add_listener(&mut self, sink: SinkRef<PubSubEvent>) {
        self.events.borrow_mut().write().unwrap().add_listener(sink);
    }
}
