#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![allow(unused_imports)]
#![allow(dead_code)]

mod limero;
mod pubsub;
use alloc::string::ToString;
use embassy_futures::select::select4;
use pubsub::PubSubCmd;
use pubsub::PubSubEvent;

use alloc::boxed::Box;
use embassy_executor::Spawner;
use embassy_net::{tcp::TcpSocket, Config, Ipv4Address, Stack, StackResources};
use embassy_time::{Duration, Timer};

use esp_wifi::{
    current_millis,
    wifi::{
        utils::create_network_interface, AccessPointInfo, ClientConfiguration, Configuration,
        WifiController, WifiDevice, WifiError, WifiEvent, WifiStaDevice, WifiState,
    },
    EspWifiInitFor,
};
use limero::ActorTrait;
use limero::SourceTrait;
use limero::*;

use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    delay::Delay,
    peripherals::Peripherals,
    prelude::*,
    rng::Rng,
    system::SystemControl,
    timer::{timg::TimerGroup, ErasedTimer, OneShotTimer, PeriodicTimer},
};
use log::info;
use smoltcp::iface::SocketStorage;

mod wifi_actor;
use wifi_actor::*;

mod mqtt_actor;
use mqtt_actor::*;

extern crate alloc;
use core::mem::MaybeUninit;

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 64 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

#[main]
async fn main(spawner: Spawner) {
    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = ClockControl::max(system.clock_control).freeze();
    //    let _delay = Delay::new(&clocks);
    init_heap();

    //   esp_info::logger::init_logger_from_env();
    let _ = limero::init_logger();

    let timg0 = TimerGroup::new(peripherals.TIMG0, &clocks, None);
    let timer0: ErasedTimer = timg0.timer0.into();
    let embassy_timer = [OneShotTimer::new(timer0)];
    let embassy_timer = Box::leak(Box::new(embassy_timer)); // leak the timers to make them 'static
    esp_hal_embassy::init(&clocks, embassy_timer);

    let timer1: ErasedTimer = timg0.timer1.into();
    let wifi_timer = PeriodicTimer::new(timer1);

    /*let led_actor = LedActor::new(peripherals.LED, Duration::from_millis(500));
    spawner.spawn(led_actor.run()).ok();
    let button_actor = ButtonActor::new(peripherals.BUTTON, Duration::from_millis(500));
    spawner.spawn(button_actor.run()).ok();*/

    let mut wifi_actor = WifiActor::new(
        peripherals.WIFI,
        wifi_timer,
        &clocks,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
        spawner,
    );
    let mut mqtt_actor = MqttActor::new(wifi_actor.stack());

    wifi_actor.map_to(connect_on_wifi_ready, mqtt_actor.sink_ref());

    loop {
        select4(
            wifi_actor.run(),
            mqtt_actor.run(),
            embassy_time::Timer::after(Duration::from_millis(1_000_000)),
            embassy_time::Timer::after(Duration::from_millis(1_000_000)),
        )
        .await;
    }
}

fn connect_on_wifi_ready(event: WifiActorEvent) -> Option<PubSubCmd> {
    match event {
        WifiActorEvent::Connected => {
            info!("Wifi Connected");
            Some(PubSubCmd::Connect {
                client_id: "my_esp32".to_string(),
            })
        }
        WifiActorEvent::Disconnected => {
            info!("Wifi Disconnected");
            Some(PubSubCmd::Disconnect)
        }
    }
}
