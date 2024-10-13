/*

Serves as a gateway between ESP-NOW WiFi and UART

Any data received from ESP-NOW is sent to UART
Any data received from UART is sent to ESP-NOW

To avoid data corruption, the data is framed using COBS and CRC8

*/

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
use esp_hal::uart;
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
use alloc::boxed::Box;
use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;
use alloc::vec;

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
use msg::PubSubCmd;

use actors::esp_now_actor::*;
use actors::led_actor::*;
use actors::pubsub_actor::*;
use actors::sys_actor::*;
use actors::uart_actor::*;
use actors::framer_actor::*;
use msg::framer::cobs_crc_frame;

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

  //  let esp_now_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

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

    let mut uart_actor = UartActor::new(uart0,0x00); // COBS separator == 0x00
  //  let uart_handler = mk_static!(Endpoint<UartCmd>, uart_actor.handler());

    let mut framer_actor = FramerActor::new(vec![]);
   // let framer_handler = mk_static!(Endpoint<FramerCmd>, framer_actor.handler());

    esp_now_actor.map_to(event_to_blink, led_actor.handler()); // ESP-NOW -> LED

    esp_now_actor.map_to(esp_now_to_uart, uart_actor.handler()); // ESP-NOW -> UART ( stateless ) 

    uart_actor.map_to(rxd_to_framer,framer_actor.handler()); // UART -> deframer -> ESP-NOW
    framer_actor.map_to(framer_to_esp_now,esp_now_actor.handler()); // deframer -> ESP-NOW  

    loop {
        select(
            uart_actor.run(),
            select(led_actor.run(), esp_now_actor.run()),
        )
        .await;
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
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
}

fn esp_now_to_uart(ev: &EspNowEvent)-> Option<UartCmd> {
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
        Some(UartCmd::Txd(cobs_crc_frame(data).unwrap()))
}

fn rxd_to_framer(ev: &UartEvent) -> Option<FramerCmd> {
    match ev {
        UartEvent::Rxd(data) => Some(FramerCmd::Deframe(data.clone())),
    }
}

fn framer_to_esp_now(ev: &FramerEvent) -> Option<EspNowCmd> {
    match ev {
        FramerEvent::Deframed(data) => Some(EspNowCmd::Broadcast { data:data.clone()}),
        _ => None,
    }
}


