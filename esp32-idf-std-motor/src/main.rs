//! MQTT asynchronous client example which subscribes to an internet MQTT server and then sends
//! and receives events in its own topic
#[allow(dead_code)]
use std::panic;

use anyhow::Ok;

use embassy_futures::select::*;

use esp_idf_svc::hal::gpio::OutputPin;
use esp_idf_svc::hal::gpio::PinDriver;
use esp_idf_svc::hal::peripherals::Peripherals;

mod limero_logger;
use esp_now_actor::mac_to_string;
use esp_now_actor::MAC_BROADCAST;
use limero::Actor;
use limero_logger::*;

mod led_actor;
use led_actor::LedActor;
use led_actor::LedCmd;

mod esp_now_actor;
use esp_now_actor::EspNowActor;
use esp_now_actor::EspNowCmd;
use esp_now_actor::EspNowEvent;

mod motor_actor;
use motor_actor::MotorActor;
use motor_actor::MotorCmd;

use log::*;
use motor_actor::MotorEvent;
use msg::Msg;

fn main() {
    esp_idf_svc::sys::link_patches();
    panic::set_hook(Box::new(|panic_info| {
        // Custom panic logging/handling
        println!("Custom panic: {:?}", panic_info);
    }));
    //   esp_idf_svc::log::EspLogger::initialize_default();
    let _ = limero_logger_init();
    let result = main_task();
    match result {
        Result::Ok(_) => info!("Main task finished"),
        Err(err) => error!("Main task failed: {:?}", err),
    }
    loop {}
}

fn main_task() -> anyhow::Result<()> {
    let peripherals = Peripherals::take()?;
    let led_gpio = PinDriver::output(peripherals.pins.gpio2.downgrade_output())?;

    let mut led_actor = LedActor::new(led_gpio);
    let led_handler = led_actor.handler();

    let mut esp_now_actor = EspNowActor::new(peripherals.modem)?;
    let espnow_handler = esp_now_actor.handler();

    let mut motor_actor = MotorActor::new("lm1/motor".to_string());
    let motor_handler = motor_actor.handler();

    esp_now_actor.map_to(event_to_blink, led_handler);
    esp_now_actor.map_to(event_to_motor, motor_handler);
    motor_actor.map_to(motor_to_espnow, espnow_handler);
    esp_now_actor.for_each(event_display);

    motor_actor.init()?;

    esp_idf_svc::hal::task::block_on(async {
        loop {
            info!("Main loop");
            match select3(led_actor.run(), motor_actor.run(), esp_now_actor.run()).await {
                Either3::First(_) => {}
                Either3::Second(_) => {}
                Either3::Third(_) => {}
            }
            info!("Main loop ");
        }
    });
    Ok(())
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd { peer: _, data: _ } => Some(LedCmd::Pulse { duration: 100 }),
    }
}

fn event_to_motor(ev: &EspNowEvent) -> Option<MotorCmd> {
    match ev {
        EspNowEvent::Rxd { peer: _, data } => minicbor::decode::<Msg>(data)
            .ok()
            .map(|msg| MotorCmd::Msg { msg: msg }),
    }
}

fn motor_to_espnow(cmd: &MotorEvent) -> Option<EspNowCmd> {
    match cmd {
        MotorEvent::Msg { msg } => minicbor::to_vec(msg)
            .map(|data| EspNowCmd::Txd {
                peer: MAC_BROADCAST,
                data,
            })
            .ok(),
        _ => None,
    }
}

fn event_display(ev: &EspNowEvent) {
    match ev {
        EspNowEvent::Rxd { peer, data } => {
            if let Result::Ok(msg) = minicbor::decode::<Msg>(data) {
                info!(
                    "{:?} => {}",
                    mac_to_string(peer),
                    msg,
                );
            } else {
                info!(
                    "Invalid EspNow Msg {:?} from {} ",
                    data,
                    mac_to_string(peer)
                );
            }
        }
    }
}

