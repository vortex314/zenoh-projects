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

// static uart_channel_in:Rc<RefCell<Option<Channel<NoopRawMutex,Message,10>>>>=Rc::new(RefCell::new(None));

#[embassy_executor::task]
async fn uart_task( mut uart:UartActor) {
    uart.run().await;
}

#[embassy_executor::task]
async fn client_task( mut client:ClientSession) {
    client.run().await;
}


#[main]
async fn main(spawner: Spawner) {
    init_heap();
    semi_logger_init().unwrap();
    let peripherals = Peripherals::take();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();

    // Initialize Embassy with needed timers
    let timer_group0 = esp_hal::timer::TimerGroup::new(peripherals.TIMG0, &clocks);
    embassy::init(&clocks, timer_group0);

    log::info!("Hello, log!");

    // Initialize and configure UART0
    let  uart0 = Uart::new(peripherals.UART0, &clocks);

    // uart0.set_at_cmd(AtCmdConfig::new(None, None, None, AT_CMD, None));

    let  client_session = ClientSession::new();
    let mut uart_actor = UartActor::new(uart0);

    uart_actor.actor.add_sink(Box::new(|msg:MqttSnMessage| {
        info!("UartActor received message: {:?}", msg);
    }));

    
    // Spawn uart and client tasks
    spawner.spawn(uart_task(uart_actor)).ok();
    spawner.spawn(client_task(client_session)).ok();
    loop {
        Timer::after(Duration::from_millis(5_000)).await;
        info!("main thread running");
    }
}
