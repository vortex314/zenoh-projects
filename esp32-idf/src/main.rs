#![allow(unused_imports)]
#![allow(dead_code)]

use anyhow;

use embedded_svc::mqtt::client::EventPayload;
use embedded_svc::mqtt::client::QoS;
use embedded_svc::wifi;
use embedded_svc::wifi::{AuthMethod, ClientConfiguration, Configuration};
use esp_idf_hal::gpio::AnyOutputPin;
use esp_idf_hal::gpio::PinDriver;
use esp_idf_hal::peripherals::Peripherals;
// use esp_idf_hal::task::block_on;
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::log::EspLogger;
use esp_idf_svc::mqtt::client::EspMqttEvent;
use esp_idf_svc::mqtt::client::{EspMqttClient, MqttClientConfiguration};
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::wifi::WifiEvent;
use esp_idf_svc::wifi::{BlockingWifi, EspWifi};
use esp_idf_sys::select;
use futures::executor::block_on;
use futures::executor::LocalPool;
use futures::executor::ThreadPoolBuilder;
use futures::task::FutureObj;
use futures::task::LocalSpawn;
use futures::task::Spawn;
use futures::FutureExt;
// use futures::FutureExt;
use led_actor::LedCmd;
use log::{error, info};
use std::env;
use std::{thread::sleep, time::Duration};
use wifi_actor::WifiActorEvent;

mod limero;
use limero::*;

mod pubsub;
use pubsub::*;

mod wifi_actor;
use wifi_actor::WifiActor;

mod led_actor;
use led_actor::LedActor;

mod mqtt_actor;
use mqtt_actor::MqttActor;

static WIFI_SSID: &str = env!("WIFI_SSID");
static WIFI_PASS: &str = env!("WIFI_PASS");
static MQTT_BROKER: &str = "test.mosquitto.org";
static MQTT_CLIENT_ID: &str = "esp32";

fn main() {
    let r = run();
    info!("main done {:?}", r);
}
fn run() -> anyhow::Result<()> {
    esp_idf_sys::link_patches();
    limero::logger::semi_logger_init()?;

    let peripherals = Peripherals::take()?;
    let sysloop = EspSystemEventLoop::take()?;

    let led_on_board = peripherals.pins.gpio2;

    // Create actors
    let mut wifi_actor = WifiActor::new(sysloop.clone(), peripherals.modem, WIFI_SSID, WIFI_PASS)?;
    let mut mqtt_actor = MqttActor::new(MQTT_CLIENT_ID, MQTT_BROKER);
    let led_actor = LedActor::new(led_on_board.into());
    // Wire actors together
    mqtt_actor.map_to(connected_to_blink, led_actor.handler());
    wifi_actor.map_to(connected_to_mqtt, mqtt_actor.handler());
    mqtt_actor.for_each(print_pubsub_event);

    let wifi_actor = Box::leak(Box::new(wifi_actor));
    let mqtt_actor = Box::leak(Box::new(mqtt_actor));
    let led_actor = Box::leak(Box::new(led_actor));
    // Start actors
    let pool = ThreadPoolBuilder::new()
        .pool_size(2)
        .stack_size(5000)
        .after_start(|x| -> () {
            info!("started thread {x:}");
        })
        .create()?;
    pool.spawn_ok(wifi_actor.run());
    pool.spawn_ok(mqtt_actor.run());
    pool.spawn_ok(led_actor.run());
    Ok(())
}

fn connected_to_blink(event: &PubSubEvent) -> Option<LedCmd> {
    match event {
        PubSubEvent::Connected => Some(LedCmd::Blink { duration: 500 }),
        PubSubEvent::Disconnected => Some(LedCmd::Blink { duration: 50 }),
        _ => None,
    }
}

fn connected_to_mqtt(event: &WifiActorEvent) -> Option<PubSubCmd> {
    match event {
        WifiActorEvent::Connected => Some(PubSubCmd::Connect {
            client_id: MQTT_CLIENT_ID.to_string(),
        }),
        WifiActorEvent::Disconnected => Some(PubSubCmd::Disconnect),
    }
}

fn print_pubsub_event(event: &PubSubEvent) {
    match event {
        PubSubEvent::Connected => info!("Connected"),
        PubSubEvent::Disconnected => info!("Disconnected"),
        PubSubEvent::Publish { topic, payload } => {
            info!("Publish: {} {:?}", topic, payload_display_json(&payload));
            info!("Publish: {} {:?}", topic, payload_as_f64_json(&payload));
        }
    }
}
