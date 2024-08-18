#![allow(unused_imports)]
#![allow(dead_code)]

use anyhow;
use edge_executor::Executor;
use edge_executor::LocalExecutor;
use embedded_svc::mqtt::client::EventPayload;
use embedded_svc::mqtt::client::QoS;
use embedded_svc::wifi::{AuthMethod, ClientConfiguration, Configuration};
use esp_idf_hal::gpio::AnyOutputPin;
use esp_idf_hal::gpio::PinDriver;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_hal::task::block_on;
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::log::EspLogger;
use esp_idf_svc::mqtt::client::EspMqttEvent;
use esp_idf_svc::mqtt::client::{EspMqttClient, MqttClientConfiguration};
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::wifi::WifiEvent;
use esp_idf_svc::wifi::{BlockingWifi, EspWifi};
use futures::FutureExt;
use led_actor::LedCmd;
use log::{error, info};
use wifi_actor::WifiActorEvent;
use std::env;
use std::{thread::sleep, time::Duration};

mod limero;
use limero::ActorTrait;
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
fn main() -> anyhow::Result<()> {
    esp_idf_sys::link_patches();
    limero::logger::semi_logger_init()?;
    error!("test error log");
    info!("test info log");

 //   let ex: LocalExecutor = Default::default();
    block_on(main_handler())?;
    sleep(Duration::from_millis(10000000));
    Ok(())
}

async fn main_handler() -> anyhow::Result<()> {
    esp_idf_sys::link_patches();

    let peripherals = Peripherals::take().unwrap();
    let sysloop = EspSystemEventLoop::take()?;
    let led_on_board  = peripherals.pins.gpio2;
   // let driver = PinDriver::output(led_on_board);

    // Create Wifi Instance
    let mut wifi_actor = WifiActor::new(sysloop.clone(), peripherals.modem, WIFI_SSID, WIFI_PASS)?;
    let mut mqtt_actor = MqttActor::new(MQTT_CLIENT_ID, MQTT_BROKER);
    let mut led_actor = LedActor::new(led_on_board.into() );
    mqtt_actor.map_to(connected_to_blink, led_actor.sink_ref());
    wifi_actor.map_to(connected_to_mqtt, mqtt_actor.sink_ref());

    //   local_ex.spawn(wifi_actor.run()).detach();
 //   local_ex.spawn(mqtt_actor.run()).detach();
 //   let mut fuse1 = Box::pin(wifi_actor.run().fuse());
  //  let mut fuse2 = Box::pin(mqtt_actor.run().fuse());
    futures::select!{
        _ = wifi_actor.run().fuse() => {},
        _ = mqtt_actor.run().fuse() => {},
        _ = led_actor.run().fuse() => {},
    };

    sleep(Duration::from_millis(10000000));
    Ok(())
}


fn connected_to_blink ( event : PubSubEvent) -> Option<LedCmd> {
    info!(" ====> event {:?}",event);
    match event {
        PubSubEvent::Connected => Some(LedCmd::Blink { duration: 500 }),
        PubSubEvent::Disconnected => Some(LedCmd::Blink { duration: 50 }),
        _ => None
    }
}

fn connected_to_mqtt ( event : WifiActorEvent) -> Option<PubSubCmd> {
    info!(" ====> event {:?}",event);
    match event {
        WifiActorEvent::Connected => Some(PubSubCmd::Connect {
            client_id: MQTT_CLIENT_ID.to_string(),
        }),
        WifiActorEvent::Disconnected => Some(PubSubCmd::Disconnect),
        _ => None
    }
}