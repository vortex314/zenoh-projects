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
#![warn(unused_extern_crates)]

use core::mem::MaybeUninit;
use embassy_executor::Spawner;
use embassy_futures::select::select;

use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    gpio::{AnyOutput, Io, Level},
    peripherals::Peripherals,
    prelude::*,
    rng::Rng,
    system::SystemControl,
    timer::{ErasedTimer, OneShotTimer, PeriodicTimer},
    uart::{
        config::{Config, DataBits, Parity, StopBits},
        ClockSource, Uart,
    },
};
use esp_wifi::{initialize, EspWifiInitFor};
use limero::*;
use log::{info, warn};
use minicbor::Encode;
use serde::{Deserialize, Serialize};
use serdes::cobs_crc_frame;
use serdes::{Cbor, PayloadCodec};

extern crate alloc;
use crate::alloc::string::ToString;
use alloc::{boxed::Box, string::String, vec::Vec};

mod proxy_msg;
use proxy_msg::*;

#[global_allocator]
pub static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

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

fn mk_static<T>(val: T) -> &'static T {
    Box::leak(Box::new(val))
}

use actors::esp_now_actor::*;
use actors::led_actor::*;
use actors::uart_actor::*;

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

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    #[derive(Debug, Serialize, Deserialize)]
    struct UartMsg([u8; 6], Vec<u8>);

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;

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

    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    let uart_handler = mk_static!(Endpoint<UartCmd>, uart_actor.handler());

    // esp_now_actor >> esp_now_to_uart >> uart_actor;
    // uart_actor >> uart_to_esp_now >> esp_now_actor;
    /*
    let emitter = Emitter(func);
    emitter.add_listener(esp_now_actor.handler());
    uart_handler.add_listener(emitter);

     */

    esp_now_actor.for_all(|ev| {
        match ev {
            EspNowEvent::Rxd {
                peer,
                rssi: _,
                channel: _,
                data,
            } => {
                match event_to_proxy_msg(ev) {
                    Some(msg) => {
                        let buffer = Vec::new();
                        let mut encoder = minicbor::Encoder::new(buffer);
                        let v = msg.encode(&mut encoder, &mut ());
                        let frame = encoder.into_writer();
                        // info!("{}",minicbor::display(frame.as_slice()));
                        uart_handler.handle(&UartCmd::Txd(cobs_crc_frame(&frame).unwrap()));
                    },
                    None => {}
                }
            }
            EspNowEvent::Broadcast {
                peer,
                rssi: _,
                channel: _,
                data,
            } => {
                match event_to_proxy_msg(ev) {
                    Some(msg) => {
                        let buffer = Vec::new();
                        let mut encoder = minicbor::Encoder::new(buffer);
                        let v = msg.encode(&mut encoder, &mut ());
                        let frame = encoder.into_writer();
                        // info!("{}",minicbor::display(frame.as_slice()));
                        uart_handler.handle(&UartCmd::Txd(cobs_crc_frame(&frame).unwrap()));
                    },
                    None => {}
                }
            }
        };
        //        uart_handler.handle(&UartCmd::Txd(Cbor::encode(&UartMsg(*peer, data.to_vec()))));
    });

    loop {
        select(
            uart_actor.run(),
            select(esp_now_actor.run(), led_actor.run()),
        )
        .await;
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            rssi: _,
            channel: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 50 }),
        EspNowEvent::Broadcast {
            peer: _,
            rssi: _,
            channel: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 50 }),
    }
}

fn event_to_proxy_msg(ev: &EspNowEvent) -> Option<ProxyMessage> {
    match ev {
        EspNowEvent::Rxd {
            peer,
            rssi,
            channel,
            data,
        } => Some(ProxyMessage::RecvEvent(RecvEvent {
            header: EspNowHeader {
                dst: None,
                src: Some(peer.clone()),
                channel: *channel,
                rssi: *rssi,
            },
            payload: data.to_vec(),
        })),
        EspNowEvent::Broadcast {
            peer,
            rssi,
            channel,
            data,
        } => Some(ProxyMessage::RecvEvent(RecvEvent {
            header: EspNowHeader {
                dst: None,
                src: Some(peer.clone()),
                channel: *channel,
                rssi: *rssi,
            },
            payload: data.to_vec(),
        })),
    }
}
