#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![allow(unused_imports)]
#![allow(dead_code)]
use core::ops::Shr;
use core::{cell::RefCell, mem::MaybeUninit};

use alloc::rc::Rc;
use alloc::string::ToString;
use embassy_executor::Spawner;
use embassy_futures::join::{join3, join4};
use embassy_futures::select::{self, select3,Either3};
use embassy_futures::select::Either3::{First, Second, Third};
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::DynamicSender;
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};
use embassy_time::{with_timeout, Duration, Timer};
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    embassy,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
    Uart,
};
use esp_println::println;
use log::info;

use minicbor::decode::info;

mod logger;
use logger::semi_logger_init;

mod protocol;
use protocol::msg::MqttSnMessage;

mod client;
use client::ClientSession;

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


fn map_connected_to_blink_fast(event : SessionEvent) -> Option<LedCmd> {
    match event {
        SessionEvent::Connected => Some(LedCmd::Blink { duration: 100 }),
        SessionEvent::Disconnected => Some(LedCmd::Blink { duration: 1000 }),
        _ => None
    }
}

#[main]
async fn main(_spawner: Spawner) {
    init_heap();
    semi_logger_init().unwrap();
    log::info!("Logger initialized.");

    let peripherals = Peripherals::take();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();

    // Initialize Embassy with needed timers
    let timer_group0 = esp_hal::timer::TimerGroup::new(peripherals.TIMG0, &clocks);
    embassy::init(&clocks, timer_group0);

    // Initialize and configure UART0
    let uart0 = Uart::new(peripherals.UART0, &clocks);
    let led = Led::new(peripherals.GPIO.split().into_output_pin(2));

    // uart0.set_at_cmd(AtCmdConfig::new(None, None, None, AT_CMD, None));

    let mut uart_actor = UartActor::new(uart0);
    let mut client_session = ClientSession::new(uart_actor.sink_ref() );
    connect(&client_session, map_connected_to_blink_fast, led.sink_ref());

    loop {
        join3(
            uart_actor.run(),
            client_session.run(),
            led.run(),
        ).await;
    }
}
