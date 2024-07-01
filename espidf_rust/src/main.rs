#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![allow(unused_imports)]
#![allow(dead_code)]
use core::ops::Shr;
use core::{cell::RefCell, mem::MaybeUninit};

use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::string::ToString;

use embassy_executor::raw::Executor;
use embassy_executor::Spawner;
use embassy_futures::join::{join3, join4};
use embassy_futures::select::Either3::{First, Second, Third};
use embassy_futures::select::{self, select3, Either3};
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::DynamicSender;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};
use embassy_time::{with_timeout, Duration, Timer};

use embedded_hal::digital::v2::OutputPin;
use esp_backtrace as _;
use esp_hal::gpio::any_pin::AnyPin;
use esp_hal::gpio::{AnyOutput, GpioPin, Io, Level, Output};
use esp_hal::peripheral::Peripheral;
use esp_hal::system::SystemControl;
use esp_hal::timer::timg::TimerGroup;
use esp_hal::uart::config::{Config, DataBits, Parity, StopBits};
use esp_hal::uart::{ClockSource, TxRxPins};
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, Uart, UartRx, UartTx},
};
use esp_hal_embassy::*;
use esp_println::println;
use limero::{connect, SourceTrait};
use log::info;

use minicbor::decode::info;

mod logger;
use logger::semi_logger_init;

mod protocol;
use protocol::msg::MqttSnMessage;

mod client;
use client::{ClientSession, SessionEvent};

mod uart;
use uart::UartActor;

mod led;
use led::{Led, LedCmd};

mod limero;

extern crate alloc;

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 50 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

fn map_connected_to_blink_fast(event: SessionEvent) -> Option<LedCmd> {
    info!("Event: {:?}", event);
    match event {
        SessionEvent::Connected => Some(LedCmd::Blink { duration: 100 }),
        SessionEvent::Disconnected => Some(LedCmd::Blink { duration: 1000 }),
        _ => None,
    }
}

#[main]
async fn main(_spawner: Spawner) {
    init_heap();
    semi_logger_init().unwrap();
    log::info!("Logger initialized.");

    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();

    // Initialize Embassy with needed timers
    let timg0 = TimerGroup::new_async(peripherals.TIMG0, &clocks);
    esp_hal_embassy::init(&clocks, timg0);

    // Initialize and configure UART0

    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);

    let led_pin = io.pins.gpio2;
    let led_pin: AnyOutput = AnyOutput::new(led_pin, Level::Low);
    //  let  led_ouput_pin : Box<dyn esp_hal::gpio::OutputPin> = Box::new(led_pin) ;

    let mut led_actor = Led::new(led_pin); // pass as OutputPin

    let mut uart0 = Uart::new_async(peripherals.UART0, &clocks);
    uart0.change_baud(921600,ClockSource::Apb,&clocks);

    if uart0.set_rx_fifo_full_threshold(127).is_err() {
        info!("Error setting RX FIFO full threshold");
    }
    uart0.set_at_cmd(AtCmdConfig {
        // catch sentinel char 0x00
        pre_idle_count: Some(1),
        post_idle_count: Some(1),
        gap_timeout: Some(1),
        cmd_char: 0u8,
        char_num: Some(1),
    });
    let mut uart_actor = UartActor::new(uart0);

    let mut client_session = ClientSession::new(uart_actor.sink_ref());
    uart_actor.subscribe(Box::new(client_session.transport_sink_ref()));
    connect(
        &mut client_session,
        map_connected_to_blink_fast,
        led_actor.sink_ref(),
    );

    loop {
        select3(uart_actor.run(), client_session.run(), led_actor.run()).await;
    }
}
