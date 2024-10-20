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

use core::mem::MaybeUninit;
use embassy_executor::Spawner;
use embassy_futures::select::select3;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    gpio::{AnyOutput, Io, Level},
    peripherals::Peripherals,
    prelude::*,
    rng::Rng,
    system::SystemControl,
    timer::{ErasedTimer, OneShotTimer, PeriodicTimer},
};

use esp_wifi::{initialize, EspWifiInitFor};
use limero::*;
use log::warn;
use minicbor::Decoder;
use msg::dc_motor::DC_MOTOR_ID;
use msg::{FrameExtractor, MsgHeader};

extern crate alloc;

use actors::esp_now_actor::*;
use actors::led_actor::*;

mod motor_actor;
use motor_actor::*;

#[global_allocator]
pub static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 32 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

// When you are okay with using a nightly compiler it's better to use https://docs.rs/static_cell/2.1.0/static_cell/macro.make_static.html
macro_rules! mk_static {
    ($t:ty,$val:expr) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        #[deny(unused_attributes)]
        let x = STATIC_CELL.uninit().write(($val));
        x
    }};
}

/*fn mk_static<T>(val: T) -> &'static T {
    Box::leak(Box::new(val))
}*/

const HB_ID: u32 = msg::fnv("lm1/hb");

#[main]
async fn main(_spawner: Spawner) -> ! {
    //esp_info::logger::init_logger_from_env();
    init_heap();
    let _ = limero::init_logger();
    log::info!("ESP32-DC-MOTOR bridge starting...");

    let peripherals = Peripherals::take();

    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();

    let mut motor_actor = DcMotorActor::new(&clocks, &peripherals);

    let timer = PeriodicTimer::new(
        esp_hal::timer::timg::TimerGroup::new(peripherals.TIMG0, &clocks, None)
            .timer0
            .into(),
    );

    let init = initialize(
        EspWifiInitFor::Wifi,
        timer,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        &clocks,
    )
    .unwrap();

    let timg1 = esp_hal::timer::timg::TimerGroup::new(peripherals.TIMG1, &clocks, None);
    esp_hal_embassy::init(
        &clocks,
        mk_static!(
            [OneShotTimer<ErasedTimer>; 1],
            [OneShotTimer::new(timg1.timer0.into())]
        ),
    );

    let wifi = peripherals.WIFI;
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);

    let led_pin = io.pins.gpio2;
    let led_pin: AnyOutput = AnyOutput::new(led_pin, Level::Low);
    let (tx_pin, rx_pin) = (io.pins.gpio16, io.pins.gpio17); // was 17,16

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin
    let mut frame_extractor = FrameExtractor::new();

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;
    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    // esp_now_acor >> motor_cmd >> uart_actor
    esp_now_actor.for_all(move |ev| {
        let data = match ev {
            EspNowEvent::Rxd {
                peer: _,
                rssi: _,
                channel: _,
                data,
            } => data,
            EspNowEvent::Broadcast {
                peer: _,
                rssi: _,
                channel: _,
                data,
            } => data,
        };
        let mut decoder = Decoder::new(data);
        decoder.decode::<MsgHeader>().map(|msg_header| {
            msg_header.dst.map(|dst| {
                if dst == DC_MOTOR_ID {
                    motor_actor.handle(&DcMotorCmd::Request {
                        msg_header,
                        data,
                    });
                }
            });
        });
    });

    let espnow_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());

    loop {
        select3( motor_actor.run(),esp_now_actor.run(), led_actor.run()).await;
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
