//! Embassy ESP-NOW Example (Duplex)
//!
//! Asynchronously broadcasts, receives and sends messages via esp-now in multiple embassy tasks
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: async embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils esp-wifi/esp-now
//% CHIPS: esp32 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]
// #![allow(unused_imports)]
#![warn(unused_extern_crates)]

use embassy_executor::Spawner;
use embassy_futures::select::select3;
use esp_backtrace as _;
use esp_hal::{
    gpio::{Output,  Io, Level},
    prelude::*,
    rng::Rng,
};
use esp_hal::mcpwm::PeripheralClockConfig;
use esp_hal::mcpwm::McPwm;
use esp_hal::timer::timg::TimerGroup;
// use esp_hal::gpio::any_pin::AnyPin;

use esp_wifi:: EspWifiInitFor;
use limero::*;
use log::warn;
use minicbor::Decoder;
// use minicbor_ser::de;
use msg:: MsgHeader;
use log::info;

extern crate alloc;

use alloc::vec::Vec;
use alloc::boxed::Box;

use actors::esp_now_actor::*;
use actors::led_actor::*;

mod motor_actor;
use motor_actor::*;

fn mk_static<T>(val: T) -> &'static T {
    Box::leak(Box::new(val))
}

const HB_ID: u32 = msg::fnv("lm1/hb");
const DC_MOTOR: &str = "lm1/cutter";
const DC_MOTOR_ID: u32 = msg::fnv(DC_MOTOR);

#[esp_hal_embassy::main]
async fn main(_spawner: Spawner) -> ! {
    esp_alloc::heap_allocator!(72 * 1024);
    let _ = limero::init_logger();
    log::info!("esp32-espnow-gateway started !");

    let peripherals = esp_hal::init(esp_hal::Config::default());
    let timg1 = TimerGroup::new(peripherals.TIMG1);
    let timg0 = TimerGroup::new(peripherals.TIMG0);

    let init = esp_wifi::init(
        EspWifiInitFor::Wifi,
        timg0.timer0,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
    )
    .unwrap();

    esp_hal_embassy::init(timg1.timer0);
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);

    let  esp_now = esp_wifi::esp_now::EspNow::new(&init, peripherals.WIFI).unwrap();
    info!("esp-now version {}", esp_now.get_version().unwrap());
    let mut esp_now_actor = EspNowActor::new(esp_now);

    let led_pin = io.pins.gpio2;
    let led_pin: Output = Output::new(led_pin, Level::Low);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin
    let clock_cfg = PeripheralClockConfig::with_frequency(40u32.MHz()).unwrap();

    let  mcpwm = McPwm::new(peripherals.MCPWM0, clock_cfg);

   // let left_pwm_pin = GpioPin::new(io.pins.gpio19);
  //  let right_pwm_pin = AnyPin::new(io.pins.gpio17);
    let  left_enable_pin = Output::new(io.pins.gpio23, Level::Low);
    let  right_enable_pin = Output::new(io.pins.gpio32, Level::Low);

    let mut motor_actor = DcMotorActor::new(
        mcpwm,
        clock_cfg,
        DC_MOTOR,
        left_enable_pin,
        right_enable_pin,
        io.pins.gpio19,
    );
    // let (tx_pin, rx_pin) = (io.pins.gpio16, io.pins.gpio17); // was 17,16
    let motor_handler = motor_actor.handler();


    let esp_now = esp_wifi::esp_now::EspNow::new(&init, peripherals.WIFI).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;
    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    // esp_now_acor >> motor_cmd >> uart_actor
    esp_now_actor.for_all(move |ev| {
        let data = data_from_event(&ev);
        let mut decoder = Decoder::new(&data);
        // let msg_header = decoder.decode::<MsgHeader>().unwrap();
        decoder.decode::<MsgHeader>().map(|msg_header| {
            msg_header.dst.filter(|dst| *dst == DC_MOTOR_ID).map(|_| {
                decoder.decode::<DcMotorMsg>().map(|msg| {
                    motor_handler.handle(&DcMotorCmd::Request { msg_header, msg });
                });
            });
        });
    });

 //   let espnow_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());

    loop {
        select3(motor_actor.run(), esp_now_actor.run(), led_actor.run()).await;
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
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

fn data_from_event(ev: &EspNowEvent) -> &Vec<u8> {
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            channel: _,
            rssi: _,
            data,
        } => data,
        EspNowEvent::Broadcast {
            peer: _,
            channel: _,
            rssi: _,
            data,
        } => data,
    }
}
