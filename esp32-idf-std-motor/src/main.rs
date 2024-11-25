//! MQTT asynchronous client example which subscribes to an internet MQTT server and then sends
//! and receives events in its own topic.

use core::time::Duration;

use anyhow::Ok;
use anyhow::Result;

use embassy_futures::select::{select, Either};

use esp_idf_svc::espnow::EspNow;
use esp_idf_svc::espnow::PeerInfo;
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::hal::gpio::OutputPin;
use esp_idf_svc::hal::gpio::PinDriver;
use esp_idf_svc::hal::peripherals::Peripherals;
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::timer::EspTimerService;
use esp_idf_svc::wifi::*;

mod limero_logger;
use limero::Actor;
use limero_logger::*;

mod led_actor;
use led_actor::LedActor;
use led_actor::LedCmd;

mod esp_now_actor;
use esp_now_actor::EspNowEvent;
use esp_now_actor::EspNowCmd;
use esp_now_actor::EspNowActor;
use log::*;

const MAC_BROADCAST: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

fn main() {
    esp_idf_svc::sys::link_patches();
    //   esp_idf_svc::log::EspLogger::initialize_default();
    limero_logger_init().unwrap();
    main_task()
        .map_err(|err| error!("Error: {:?}", err))
        .unwrap();
}

fn main_task() -> anyhow::Result<()> {
    let timer_service = EspTimerService::new()?;
    let led_gpio = PinDriver::output(peripherals.pins.gpio2.downgrade_output())?;

    let mut led_actor = LedActor::new(led_gpio);
    let mut led_handler = led_actor.handler();

    let esp_now_actor = EspNowActor::new(peripherals.modem)?;
    let esp_now_handler = esp_now_actor.handler();

    esp_now_actor.map_to(event_to_blink, &led_handler);


    let mut counter = 0;
    let mut received_counter = 0;

    let mut timer = timer_service.timer_async().unwrap();

    esp_idf_svc::hal::task::block_on(async {
        loop {
            match select(led_actor.run(), timer.after(Duration::from_millis(1000))).await {
                Either::First(_) => {}
                Either::Second(_) => {
                    let data = minicbor::to_vec(counter).unwrap();
                    esp_now_handler.handle( EspNowCmd { MAC_BROADCAST, &data })
                    counter += 1;
                }
            }
            
        }
    });
    Ok(())
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            channel: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
        EspNowEvent::Broadcast {
            peer: _,
            channel: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
    }
}
