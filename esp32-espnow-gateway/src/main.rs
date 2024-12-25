/*

Serves as a gateway between ESP-NOW WiFi and UART

Any data received from ESP-NOW is sent to UART
Any data received from UART is sent to ESP-NOW

To avoid data corruption, the data is framed using COBS and CRC8

*/

#![no_std]
#![no_main]
#![allow(unused_imports)]
#[allow(dead_code)]

use core::{cell::RefCell, mem::MaybeUninit};
use embassy_executor::Spawner;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex};
use embassy_time::{Duration, Ticker};

use esp_backtrace as _;
use esp_hal::timer::timg::TimerGroup;
use esp_hal::uart;
use esp_hal::{
    gpio::{GpioPin, Io, Level, Output},
    peripherals::Peripherals,
    prelude::*,
    rng::Rng,
    timer::{OneShotTimer, PeriodicTimer},
    uart::{
        config::{AtCmdConfig, Config, DataBits, Parity, StopBits},
        ClockSource, Uart, UartRx, UartTx,
    },
};
use esp_wifi::esp_now;
use esp_wifi::{
    esp_now::{EspNowManager, EspNowReceiver, EspNowSender, PeerInfo, BROADCAST_ADDRESS},
    EspWifiInitFor,
};
use limero::*;
use log::{info, warn};

extern crate alloc;
use crate::alloc::string::ToString;
use alloc::boxed::Box;
use alloc::format;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;

use msg::PubSubCmd;

use actors::esp_now_actor::*;
use actors::framer_actor::*;
use actors::led_actor::*;
use actors::pubsub_actor::*;
use actors::sys_actor::*;
use actors::uart_actor::*;
use msg::framer::cobs_crc_frame;

mod translator;
use translator::*;

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

    info!("Starting ESP-NOW actor");
    let esp_now = esp_wifi::esp_now::EspNow::new(&init, peripherals.WIFI).unwrap();
    info!("esp-now version {}", esp_now.get_version().unwrap());
    let mut esp_now_actor = EspNowActor::new(esp_now);

    info!("Starting LedActor");
    let led_pin = io.pins.gpio2;
    let led_pin: Output = Output::new(led_pin, Level::Low);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    info!("Starting UART0");
    /*
        let uart0 = Uart::new_async_with_config(
            peripherals.UART0,
            Config {
                baudrate: 115200,
                data_bits: DataBits::DataBits8,
                parity: Parity::ParityNone,
                stop_bits: StopBits::STOP1,
                clock_source: ClockSource::Apb,
                rx_fifo_full_threshold: 64,
                rx_timeout: Some(10),
            },
            io.pins.gpio1,
            io.pins.gpio3,
        )
        .unwrap();

        let mut uart_actor = UartActor::new(uart0, 0x00); // COBS separator == 0x00
                                                          //  let uart_handler = mk_static!(Endpoint<UartCmd>, uart_actor.handler());
    */
    let mut framer_actor = FramerActor::new(vec![]);
    // let framer_handler = mk_static!(Endpoint<FramerCmd>, framer_actor.handler());

    esp_now_actor.map_to(event_to_blink, led_actor.handler()); // ESP-NOW -> LED

    //    esp_now_actor.map_to(esp_now_to_uart, uart_actor.handler()); // ESP-NOW -> UART ( stateless )

    //  uart_actor.map_to(rxd_to_framer, framer_actor.handler()); // UART -> deframer -> ESP-NOW
    framer_actor.map_to(framer_to_esp_now, esp_now_actor.handler()); // deframer -> ESP-NOW

    loop {
        info!("loop");
        //     select(
        //         uart_actor.run(),
        select(led_actor.run(), esp_now_actor.run()).await;
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    let data = match ev {
        EspNowEvent::Rxd {
            peer: _,
            data,
            rssi: _,
            channel: _,
        } => data,
        EspNowEvent::Broadcast {
            peer: _,
            data,
            rssi: _,
            channel: _,
        } => data,
    };

    let _ = minicbor::decode::<msg::Msg>(data.as_slice())
        .map(|msg| info!("event {}", msg))
        .map_err(|e| {
            warn!("Failed to decode {:?}", e);
        });
    Some(LedCmd::Pulse { duration: 100 })
}

fn esp_now_to_uart(ev: &EspNowEvent) -> Option<UartCmd> {
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
    let data = cobs_crc_frame(data).unwrap();
    Some(UartCmd::Txd(data))
}

fn rxd_to_framer(ev: &UartEvent) -> Option<FramerCmd> {
    info!("rxd_to_framer {:?}", ev);
    match ev {
        UartEvent::Rxd(data) => Some(FramerCmd::Deframe(data.clone())),
    }
}

fn framer_to_esp_now(ev: &FramerEvent) -> Option<EspNowCmd> {
    match ev {
        FramerEvent::Deframed(data) => Some(EspNowCmd::Broadcast { data: data.clone() }),
        _ => None,
    }
}
