//! MQTT asynchronous client example which subscribes to an internet MQTT server and then sends
//! and receives events in its own topic.

use core::time::Duration;

use anyhow::Ok;

use embassy_futures::select::*;

use esp_idf_svc::hal::gpio::OutputPin;
use esp_idf_svc::hal::gpio::PinDriver;
use esp_idf_svc::hal::peripherals::Peripherals;
use esp_idf_svc::timer::EspTimerService;

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
use msg::Msg;

fn main() {
    esp_idf_svc::sys::link_patches();
    //   esp_idf_svc::log::EspLogger::initialize_default();
    limero_logger_init().unwrap();
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
    let esp_now_handler = esp_now_actor.handler();

    let mut motor_actor = MotorActor::new("lm1/motor");
    let motor_handler = motor_actor.handler();

    esp_now_actor.map_to(event_to_blink, led_handler);
    esp_now_actor.map_to(event_to_motor, motor_handler);
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
        }
    });
    Ok(())
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd { peer: _, data: _ } => Some(LedCmd::Pulse { duration: 10 }),
    }
}

fn event_to_motor(data: &Vec<u8>) -> Option<MotorCmd> {
    minicbor::decode::<Msg>(data).ok().map(|msg| { Some(MotorCmd::Rxd {msg: msg,})})
}

fn event_display(ev: &EspNowEvent) {
    match ev {
        EspNowEvent::Rxd { peer, data } => {
            if let Result::Ok(mut msg) = minicbor::decode::<Msg>(data) {
                msg.pub_req.map(|bytes| {
                    info!(
                        "Pub_req from {:x?} : {}",
                        msg.src.get_or_insert(0),
                        minicbor::display(&bytes)
                    );
                });
                msg.info_reply.map(|info_map| {
                    info!(
                        "info_reply from {:x?} : {:?}",
                        msg.src.get_or_insert(0),
                        info_map
                    );
                });
            } else {
                info!(
                    "Invalid EspNow Msg {:?} from {} ",
                    data,
                    mac_to_string(peer)
                );
            }
        }
        _ => {}
    }
}
