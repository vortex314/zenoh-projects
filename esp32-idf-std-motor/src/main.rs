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
    let sys_loop = EspSystemEventLoop::take()?;
    let timer_service = EspTimerService::new()?;
    let nvs = EspDefaultNvsPartition::take()?;
    let peripherals = Peripherals::take()?;
    info!("Peripherals taken");

    let led_gpio = PinDriver::output(peripherals.pins.gpio2.downgrade_output())?;

    let mut led_actor = LedActor::new(led_gpio);
    let mut led_handler = led_actor.handler();

    let mut wifi_driver =
        esp_idf_svc::wifi::WifiDriver::new(peripherals.modem, sys_loop.clone(), Some(nvs))?;
    info!("Wifi driver created");

    wifi_driver.start()?;
    info!("Wifi driver started");

    let _sub = {
        sys_loop
            .subscribe::<WifiEvent, _>(move |event| {
                info!("Wifi event ===> {:?}", event);
            })
            .unwrap()
    };

    let esp_now = esp_idf_svc::espnow::EspNow::take()?;
    let mut received_counter = 0;

    esp_now.register_recv_cb(|mac, data| {
        info!("Received {:?}: {}", mac, minicbor::display(data));
        let r_c: i32 = minicbor::decode(data).unwrap();
        if (r_c - received_counter) != 1 {
            error!("Lost packets: {} -> {}", received_counter, r_c);
        }
        led_handler.handle(&LedCmd::Pulse { duration: 10 });
        received_counter = r_c;
    })?;
    esp_now.register_send_cb(|_data, status| {
        debug!("Send status: {:?}", status);
    })?;
    esp_now.add_peer(PeerInfo {
        peer_addr: MAC_BROADCAST,
        lmk: [0; 16],
        channel: 1,
        ifidx: 0,
        encrypt: false,
        priv_: std::ptr::null_mut(),
    })?;

    let mut counter = 0;
    let mut timer = timer_service.timer_async().unwrap();

    esp_idf_svc::hal::task::block_on(async {
        loop {
            match select(led_actor.run(), timer.after(Duration::from_millis(1000))).await {
                Either::First(_) => {}
                Either::Second(_) => {
                    let data = minicbor::to_vec(counter).unwrap();
                    let res = esp_now.send(MAC_BROADCAST, &data);
                    if res.is_err() {
                        error!("Error: {:?}", res.err().unwrap());
                    }
                    counter += 1;
                }
            }
            
        }
    });
    Ok(())
}

/*fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            data: _,
            rssi: _,
            channel: _,
        } => Some(LedCmd::Pulse { duration: 50 }),
        EspNowEvent::Broadcast {
            peer: _,
            data: _,
            rssi: _,
            channel: _,
        } => Some(LedCmd::Pulse { duration: 50 }),
    }
}*/
