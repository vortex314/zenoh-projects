#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![allow(unused_imports)]
#![allow(dead_code)]
use core::{cell::RefCell, mem::MaybeUninit};

extern crate alloc;

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

use alloc::rc::Rc;
use alloc::string::ToString;
use embassy_executor::Spawner;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
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

mod protocol;

use minicbor::decode::info;
use protocol::ProxyMessage;

mod logger;
use logger::semi_logger_init;

mod client;
use client::fsm_connection;
use client::uart_reader;
use client::uart_writer;
use client::UART_BUFSIZE;




fn init_heap() {
    const HEAP_SIZE: usize = 50 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

// static uart_channel_in:Rc<RefCell<Option<Channel<NoopRawMutex,Message,10>>>>=Rc::new(RefCell::new(None));

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
    let mut uart0 = Uart::new(peripherals.UART0, &clocks);
    //    uart0.set_at_cmd(AtCmdConfig::new(None, None, None, AT_CMD, None));
    uart0
        .set_rx_fifo_full_threshold(UART_BUFSIZE as u16)
        .unwrap();
    // Split UART0 to create seperate Tx and Rx handles
    let (tx, rx) = uart0.split();
    info!("UART0 initialized");
    // Spawn Tx and Rx tasks
    spawner.spawn(uart_reader(rx)).ok();
    spawner.spawn(uart_writer(tx)).ok();
    spawner.spawn(fsm_connection()).ok();
    loop {
        Timer::after(Duration::from_millis(5_000)).await;
    }
}
