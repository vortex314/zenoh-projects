use core::borrow::BorrowMut;
use core::cell::RefCell;
use core::result;
use std::sync::{Arc, RwLock};
use std::task::Poll;
use std::time::{Duration, Instant};

use embedded_svc::http::{client, Query};
use esp_idf_svc::mdns::{EspMdns, QueryResult};
use esp_idf_svc::mqtt::client::{
    EspAsyncMqttClient, EspAsyncMqttConnection, EspMqttClient, EspMqttEvent, EventPayload,
    LwtConfiguration, MqttClientConfiguration, QoS,
};

use esp_idf_sys::EspError;
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
    mqtt_host: String,
    mqtt_client: Option<EspMqttClient<'static>>,
    client_id: String,
    uptime_topic: String,
    subscribe_topic: String,
    lwt_topic: String,
    lwt_payload_online: Vec<u8>,
    lwt_payload_offline: Vec<u8>,
    esp_mdns: EspMdns,
    mqtt_events: Sink<MqttEvent>,
}
impl MqttActor {
    pub fn new(client_id: &str, mqtt_host: &str) -> MqttActor {
        // Set up handle for MQTT Config
        let events = Arc::new(RwLock::new(Source::new()));
        let cmds = Sink::new();
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
            mqtt_client: None,
            client_id: client_id.to_string(),
            uptime_topic,
            subscribe_topic,
            lwt_topic,
            lwt_payload_online: lwt_payload_online.clone(),
            lwt_payload_offline: lwt_payload_offline.clone(),
            esp_mdns: EspMdns::take().unwrap(),
            mqtt_events: Sink::new(),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<PubSubCmd> {
        self.cmds.sink_ref()
    }

    pub fn create_client(&mut self) -> Result<()> {
        info!("MDNS Query");
        let ipv4 = self.esp_mdns.query_a("pcthink", Duration::from_secs(3))?; // Query for IP Address

        info!("MDNS Query =>  {:?}", ipv4);

        let mut mqtt_config = MqttClientConfiguration::default();
        let lwt_topic = format!("src/{}/sys/lwt", self.client_id);
        let lwt_config = LwtConfiguration {
            topic: &lwt_topic,
            payload: &self.lwt_payload_offline.clone(),
            qos: QoS::AtLeastOnce,
            retain: false,
        };
        mqtt_config.lwt = Some(lwt_config);
        mqtt_config.keep_alive_interval = Some(Duration::from_secs(3));
        mqtt_config.reconnect_timeout = Some(Duration::from_secs(1));

        let sink_ref = self.mqtt_events.sink_ref();

        let mqtt_broker = format!("mqtt://{}", ipv4);
        info!("MQTT connecting {}", mqtt_broker);
        let client =
            EspMqttClient::new_cb(mqtt_broker.as_str(), &mqtt_config, move |event| match event
                .payload()
            {
                EventPayload::BeforeConnect => {
                    info!("Before Connect");
                }
                EventPayload::Connected(b) => {
                    info!("Connect {}", b);
                    sink_ref.send(MqttEvent::Connected);
                }
                EventPayload::Disconnected => {
                    info!("Disconnected");
                    sink_ref.send(MqttEvent::Disconnected);
                }
                EventPayload::Received {
                    id:_,
                    topic,
                    data,
                    details:_,
                } => {
                    sink_ref.send(MqttEvent::Publish {
                        topic: topic.unwrap().to_string(),
                        payload: data.to_vec(),
                    });
                }
                EventPayload::Published(_id) => {}
                _r => {
                    info!("Other Event {:?}", _r);
                }
            })?;

        info!("MQTT connected");
        self.mqtt_client = Some(client);
        Ok(())
    }

    pub fn handle_cmd(&mut self, cmd: PubSubCmd) -> Result<()> {
        match cmd {
            PubSubCmd::Publish { topic, payload } => {
                if let Some(cl) = self.mqtt_client.as_mut() {
                    cl.publish(topic.as_str(), QoS::AtLeastOnce, false, &payload)?;
                } else {
                    return Err(anyhow::anyhow!("Not connected"));
                }
            }
            PubSubCmd::Subscribe { topic } => {
                if let Some(cl) = self.mqtt_client.as_mut() {
                    cl.subscribe(topic.as_str(), QoS::AtLeastOnce)?;
                } else {
                    return Err(anyhow::anyhow!("Not connected"));
                }
            }
            PubSubCmd::Unsubscribe { topic } => {
                if let Some(cl) = self.mqtt_client.as_mut() {
                    cl.unsubscribe(topic.as_str())?;
                } else {
                    return Err(anyhow::anyhow!("Not connected"));
                }
            }
            PubSubCmd::Connect { client_id: _ } => {
                if self.mqtt_client.is_some() {
                    return Err(anyhow::anyhow!("Already connected"));
                }
                self.create_client()?;
                info!("MQTT Subscribe to {}", self.subscribe_topic);
                self.mqtt_client
                    .as_mut()
                    .map(|cl| cl.subscribe(&self.subscribe_topic, QoS::AtLeastOnce)); // Subscribe to topic
                info!("MQTT Publish to {}", self.lwt_topic);
                self.mqtt_client.as_mut().map(|cl| {
                    cl.publish(
                        &self.lwt_topic,
                        QoS::AtLeastOnce,
                        false,
                        &self.lwt_payload_online,
                    )
                });
            }
            cmd => {
                return Err(anyhow::anyhow!("Command not implemented {:?}", cmd));
            }
        }
        Ok(())
    }

    fn emit_event(&self, event: PubSubEvent) -> Result<()> {
        let r = self.events.try_write().map(|w| w.emit(event));
        if r.is_err() {
            return Err(anyhow::anyhow!("Error emitting event"));
        };
        Ok(())
    }

    async fn handle_connection_event(&mut self, event: EspMqttEvent<'_>) -> Result<()> {
        {
            match event.payload() {
                EventPayload::BeforeConnect => {
                    info!("Before Connect");
                    self.emit_event(PubSubEvent::Disconnected)?;
                }
                EventPayload::Connected(b) => {
                    info!("Connect {}", b);
                    self.emit_event(PubSubEvent::Disconnected)?;
                    self.cmds.sink_ref().send(PubSubCmd::Connect {
                        client_id: self.client_id.clone(),
                    });
                }
                EventPayload::Disconnected => {
                    info!("Disconnected");
                    self.emit_event(PubSubEvent::Disconnected)?;
                }
                EventPayload::Received {
                    id,
                    topic,
                    data,
                    details,
                } => {
                    info!("Received {:?} {:?} {:?}", id, topic, details);
                    self.emit_event(PubSubEvent::Publish {
                        topic: topic.unwrap().to_string(),
                        payload: data.to_vec(),
                    })?;
                }
                EventPayload::Published(_id) => {}
                _r => {
                    info!("Other Event {:?}", _r);
                }
            }
        }
        Ok(())
    }
}

impl ActorTrait<PubSubCmd, PubSubEvent> for MqttActor {
    async fn run(&mut self) {
        info!("MqttActor::run");

        self.timers.add_timer(timer::Timer::new_repeater(
            TimerId::PingTimer as u32,
            Duration::from_secs(1),
        ));
        loop {
            futures::select! {
                cmd = self.cmds.next().fuse() => {
                    if let Some(cmd) = cmd {
                        let r  = self.handle_cmd(cmd);
                        if r.is_err() {
                            error!("MqttActor::handle_cmd {:?}",r);
                        }
                    }
                }
                timer_idx = self.timers.alarm().fuse() => {
                    if timer_idx == TimerId::PingTimer as u32 {
                    let line = format!("{}",std::time::UNIX_EPOCH.elapsed().unwrap().as_millis());
                    self.cmds.sink_ref().send(PubSubCmd::Publish { topic:self.uptime_topic.clone(), payload:line.as_bytes().to_vec() });
                    }
                }
                event = self.mqtt_events.next().fuse() => {
                    if let Some(event) = event {
                        match event {
                            MqttEvent::Connected => {
                                self.emit_event(PubSubEvent::Connected).unwrap();
                            }
                            MqttEvent::Disconnected => {
                                self.emit_event(PubSubEvent::Disconnected).unwrap();
                            }
                            MqttEvent::Publish { topic, payload } => {
                                self.emit_event(PubSubEvent::Publish { topic, payload }).unwrap();
                            }
                        }
                    }
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
