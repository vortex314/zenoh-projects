#![allow(unused_imports)]
#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(unused_mut)]

use esp_idf_svc::hal::peripherals::Peripherals;
use core::fmt::Write; // allows use to use the WriteLn! macro for easy printing
use debouncr::{debounce_16, Edge};
use esp32_hal::{
    clock::ClockControl, peripherals::Peripherals, prelude::*, timer::TimerGroup, uart::Uart, Rtc,
    IO,
};
use esp_backtrace as _;
use log::info;

mod protocol;

use protocol::Message;



fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71


    esp_idf_svc::sys::link_patches();

    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    let peripherals = Peripherals::take();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();
    
    // Instantiate and Create Handles for the RTC and TIMG watchdog timers
    let mut rtc = Rtc::new(peripherals.RTC_CNTL);
    let timer_group0 = TimerGroup::new(peripherals.TIMG0, &clocks);
    let mut wdt0 = timer_group0.wdt;
    let timer_group1 = TimerGroup::new(peripherals.TIMG1, &clocks);
    let mut wdt1 = timer_group1.wdt;


    let log_msg = Message::new_log("Hello world!");
    let bytes = protocol::make_frame(log_msg).unwrap();
    let decoded = protocol::decode_frame(bytes).unwrap();
    info!("Decoded : {:?}", decoded );


    info!("Hello, world!");
}
