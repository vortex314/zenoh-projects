//! Embassy ESP-NOW Example
//!
//! Broadcasts, receives and sends messages via esp-now in an async way
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: async embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils esp-wifi/esp-now
//% CHIPS: esp32 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]
#![allow(unused_imports)]
#![feature(impl_trait_in_assoc_type)]
#![feature(type_alias_impl_trait)]


use core::mem::MaybeUninit;


use edge_executor::LocalExecutor;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl, macros::main, peripherals::Peripherals, rng::Rng, system::SystemControl, timer::{
        timg::{Timer, TimerGroup},
        ErasedTimer, OneShotTimer, PeriodicTimer,
    }
};
use esp_println::println;
use esp_wifi::{
    esp_now::{PeerInfo, BROADCAST_ADDRESS},
    initialize, EspWifiInitFor,
};

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 32 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

macro_rules! mk_static {
    ($t:ty,$val:expr) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        #[deny(unused_attributes)]
        let x = STATIC_CELL.uninit().write(($val));
        x
    }};
}

fn main() {
    esp_println::logger::init_logger_from_env();

    let peripherals = Peripherals::take();

    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();

    let timg0 = TimerGroup::new(peripherals.TIMG0, &clocks, None);
    let timer0: ErasedTimer = timg0.timer0.into();
    let timer0 = PeriodicTimer::new(timer0);
 //   let timer0 = mk_static!(PeriodicTimer<ErasedTimer>, timer0);
    let init = esp_wifi::initialize(
        EspWifiInitFor::Wifi,
        timer0,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        &clocks,
    )
    .unwrap();

    let wifi = peripherals.WIFI;
    let  esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    println!("esp-now version {}", esp_now.get_version().unwrap());

    let (mgr, sender, receiver) = esp_now.split();

    let timg1 = TimerGroup::new(peripherals.TIMG1, &clocks, None);
    let timer1: ErasedTimer = timg1.timer1.into();
    let timer1 = [OneShotTimer::new(timer1)];
    let timer1 = mk_static!([OneShotTimer<ErasedTimer>; 1], timer1);

 //   esp_hal_embassy::init(&clocks, timer1);
 //   let executor = mk_static!(esp_hal_embassy::Executor, esp_hal_embassy::Executor::new());
 let executore : LocalExecutor = Default::default();
    let mut executor = executore;
    executor.run(|spawner| async move {
        let mut sender = sender.init(mgr).unwrap();
        let mut receiver = receiver.init(mgr).unwrap();

        spawner.spawn(async {
            send_hello(&mut sender).await;
        });

        spawner.spawn(async {
            receive_hello(&mut receiver).await;
        });
    });


}

async fn send_hello(sender: &mut esp_wifi::esp_now::EspNowSender<'static>) {
    loop {
        let status = sender.send_async(&BROADCAST_ADDRESS, b"Hello").await;
        println!("Send hello status: {:?}", status);
    }
}

async fn receive_hello(receiver: &mut esp_wifi::esp_now::EspNowReceiver<'static>) {
    loop {
        let recv_data = receiver.receive_async().await;
        println!(
            "Received from {:?}: {:?}",
            recv_data.info.src_address,
            core::str::from_utf8(&recv_data.data).unwrap()
        );
    }
}
