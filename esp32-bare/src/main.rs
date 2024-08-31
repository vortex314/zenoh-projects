//! Embassy ESP-NOW Example (Duplex)
//!
//! Asynchronously broadcasts, receives and sends messages via esp-now in multiple embassy tasks
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: async embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils esp-wifi/esp-now
//% CHIPS: esp32 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]
#![allow(unused_imports)]
use core::{cell::RefCell, mem::MaybeUninit};
use embassy_executor::Spawner;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex};
use embassy_time::{Duration, Ticker};

use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    gpio::{AnyOutput, GpioPin, Io, Level, Output},
    peripherals::Peripherals,
    prelude::*,
    rng::Rng,
    system::SystemControl,
    timer::{ErasedTimer, OneShotTimer, PeriodicTimer},
    uart::{
        config::{AtCmdConfig, Config, DataBits, Parity, StopBits},
        ClockSource, DefaultRxPin, DefaultTxPin, Uart, UartRx, UartTx,
    },
};
use esp_wifi::esp_now;
use esp_wifi::{
    esp_now::{EspNowManager, EspNowReceiver, EspNowSender, PeerInfo, BROADCAST_ADDRESS},
    initialize, EspWifiInitFor,
};
use limero::*;
use log::{info, warn};

extern crate alloc;
use crate::alloc::string::ToString;
use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 16 * 1024;
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
use pubsub::PubSubCmd;

mod pubsub_actor;
use pubsub_actor::PubSubActor;

mod esp_now_actor;
use esp_now_actor::*;

mod proxy_message;
use proxy_message::*;

mod uart_actor;
use uart_actor::UartActor;

mod sys_actor;
use sys_actor::SysActor;

mod led_actor;
use led_actor::*;

use serdes::*;

#[main]
async fn main(_spawner: Spawner) -> ! {
    //esp_info::logger::init_logger_from_env();
    init_heap();
    let _ = limero::init_logger();
    log::info!("Hello, world!");

    let peripherals = Peripherals::take();

    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();

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

    // let esp_now = mk_static!(esp_wifi::esp_now::EspNow<'static>, esp_now);

    #[cfg(feature = "esp32")]
    {
        let timg1 = esp_hal::timer::timg::TimerGroup::new(peripherals.TIMG1, &clocks, None);
        esp_hal_embassy::init(
            &clocks,
            mk_static!(
                [OneShotTimer<ErasedTimer>; 1],
                [OneShotTimer::new(timg1.timer0.into())]
            ),
        );
    }

    #[cfg(not(feature = "esp32"))]
    {
        let systimer = esp_hal::timer::systimer::SystemTimer::new(peripherals.SYSTIMER);
        esp_hal_embassy::init(
            &clocks,
            mk_static!(
                [OneShotTimer<ErasedTimer>; 1],
                [OneShotTimer::new(systimer.alarm0.into())]
            ),
        );
    }

    let wifi = peripherals.WIFI;
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);

    let led_pin = io.pins.gpio2;
    let led_pin: AnyOutput = AnyOutput::new(led_pin, Level::Low);

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    esp_now_actor.for_each(|ev| {
        match ev {
            EspNowEvent::Rxd { peer, data } => {
                info!(
                    "Rxd: {:?} {:?}",
                    mac_to_string(peer),
                    String::from_utf8_lossy(data).to_string()
                );
                // led_actor.handler().handle(&LedCmd::Blink { duration: 100 });
            }
            EspNowEvent::Broadcast { peer, rssi, data } => {
                info!(
                    "Broadcast: {:?} {:?} {:?}",
                    mac_to_string(peer),
                    rssi,
                    String::from_utf8_lossy(data).to_string()
                );
                //  led_actor.handler().handle(&LedCmd::Pulse { duration: 100 });
            }
        }
    });

    esp_now_actor.map_to(
        |ev| match ev {
            EspNowEvent::Rxd { peer, data } => Some(LedCmd::Pulse { duration: 100 }),
            EspNowEvent::Broadcast { peer, rssi, data } => Some(LedCmd::Pulse { duration: 100 }),
            _ => None,
        },
        led_actor.handler(),
    );

    #[cfg(feature = "gateway")]
    {
        let uart0 = Uart::new_async_with_config(
            peripherals.UART0,
            Config {
                baudrate: 115200,
                data_bits: DataBits::DataBits8,
                parity: Parity::ParityNone,
                stop_bits: StopBits::STOP1,
                clock_source: ClockSource::Apb,
                rx_fifo_full_threshold: 127,
                rx_timeout: None,
            },
            &clocks,
            io.pins.gpio1,
            io.pins.gpio3,
        )
        .unwrap();

        let mut uart_actor = UartActor::new(uart0);
        loop {
            select(
                uart_actor.run(),
                select(esp_now_actor.run(), led_actor.run()),
            )
            .await;
        }
    }
    #[cfg(feature = "client")]
    {
        let mut pubsub_actor = PubSubActor::new(transport_actor.handler());
        pubsub_actor.handler().handle(&PubSubCmd::Connect {
            client_id: "esp-now".to_string(),
        });
        loop {
            select(pubsub_actor.run(), esp_now_actor.run()).await;
        }
    }
}
