// use embassy_futures::select::select;
use embassy_futures::select::select3;
use embassy_time::{Duration, Timer};
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::hal::gpio::AnyOutputPin;
use esp_idf_svc::hal::prelude::Peripherals;
use esp_idf_svc::hal::sys::esp_get_free_heap_size;
use esp_idf_svc::hal::task::block_on;
use esp_idf_svc::nvs::EspDefaultNvsPartition;

// use esp_idf_svc::sys::esp_task_wdt_reset;
// use esp_idf_svc::wifi::BlockingWifi;
use esp_idf_svc::wifi::EspWifi;
use log::info;
mod esp_now_actor;
mod led_actor;
// use anyhow::Error;
use anyhow::Result;
use embedded_svc::wifi::AccessPointConfiguration;
use embedded_svc::wifi::Configuration;
// use embedded_svc::wifi::Protocol;
use embedded_svc::wifi::ClientConfiguration;
use esp_idf_svc::timer::EspTaskTimerService;
use esp_idf_svc::wifi::AsyncWifi;
use esp_idf_svc::hal::gpio::Output;
// use esp_idf_svc::wifi::AuthMethod;
// use esp_idf_svc::hal::gpio::Output;
// use esp_idf_svc::wifi::ClientConfiguration;
// use esp_idf_svc::wifi::Protocol::P802D11BGN;
// use esp_idf_svc::hal::gpio::Level;
use esp_idf_svc::hal::gpio::PinDriver;
use esp_idf_svc::hal::gpio::OutputPin;
use enumset::enum_set;
use esp_now_actor::EspNowActor;
// use esp_now_actor::EspNowCmd;
use esp_now_actor::EspNowEvent;

use led_actor::LedActor;
use led_actor::LedCmd;

use limero::Actor;

const SSID: &str ;
const PASSWORD: &str ";

mod logger;

fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_svc::sys::link_patches();

    // Bind the log crate to the ESP Logging facilities
    //   esp_idf_svc::log::EspLogger::initialize_default();
    logger::semi_logger_init().unwrap();

    log::info!("Hello, world!");

    match block_on(async_main()) {
        Ok(_) => log::info!("Main exited successfully"),
        Err(e) => log::error!("Main exited with error: {:?}", e),
    }
}

async fn async_main() -> Result<()> {
    let peripherals = Peripherals::take()?;
    let sys_loop = EspSystemEventLoop::take()?;
    let nvs = EspDefaultNvsPartition::take()?;
    let timer_service = EspTaskTimerService::new()?;

    let pins = peripherals.pins;

    let mut led_pin:PinDriver<'static, AnyOutputPin,Output> = PinDriver::output(pins.gpio2.downgrade_output())?;
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    let mut wifi = AsyncWifi::wrap(
        EspWifi::new(peripherals.modem, sys_loop.clone(), Some(nvs))?,
        sys_loop,
        timer_service,
    )?;

    let _ap = AccessPointConfiguration {
        ssid: "Motor".try_into().unwrap(),
        password: "Motor".try_into().unwrap(),
        channel: 1u8,
        secondary_channel: None,
        protocols: enum_set! ( embedded_svc::wifi::Protocol::P802D11BGN ),
        ..Default::default()
    };

    let _wifi_configuration: Configuration = Configuration::Client(ClientConfiguration {
        ssid: SSID.try_into().unwrap(),
        bssid: None,
        auth_method: embedded_svc::wifi::AuthMethod::WPA2Personal,
        password: PASSWORD.try_into().unwrap(),
        channel: Some(1),
        ..Default::default()
    });

    let _wifi_conf = esp_idf_svc::wifi::Configuration::AccessPoint(_ap);

    wifi.set_configuration(&_wifi_configuration)?;

    wifi.start().await?;
    info!("Wifi started");

    wifi.connect().await?;
    info!("Wifi connected");

    wifi.wait_netif_up().await?;
    info!("Wifi netif up");

    let mut actor = EspNowActor::new()
        .map_err(|e| {
            log::error!("Failed to create EspNowActor: {:?}", e);
        })
        .unwrap();
    actor.map_to( event_to_blink,led_actor.handler());
    loop {
        // task().await;
        let _res = select3(actor.run(), task(),led_actor.run()).await;
        info!("Restarting main loop ");
    }
}

async fn task() {
    loop {
        let heap_size = unsafe { esp_get_free_heap_size() };
        info!("Heap size is: {}", heap_size);
        Timer::after(Duration::from_secs(1)).await;
        //   unsafe { esp_task_wdt_reset() };
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    info!("Event to blink {:?}", ev);
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            channel: _,
            rssi: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
        EspNowEvent::Broadcast {
            peer: _,
            channel: _,
            rssi: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
    }
}
