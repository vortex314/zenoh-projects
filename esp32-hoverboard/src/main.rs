//! Embassy ESP-NOW Example (Duplex)
//!
//! Asynchronously broadcasts, receives and sends messages via esp-now in multiple embassy tasks
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: async embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils esp-wifi/esp-now
//% CHIPS: esp32 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]
// #![allow(unused_imports)]
#![warn(unused_extern_crates)]

use core::mem::MaybeUninit;
use embassy_executor::Spawner;
use embassy_futures::select::select3;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    gpio::{AnyOutput, Io, Level},
    peripherals::{Peripherals, UART2},
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
use log::debug;
use log::info;
use log::warn;
use msg::MsgType;
use msg::{FrameExtractor, MsgHeader};

extern crate alloc;
use alloc::vec::Vec;
use anyhow::Result;

use actors::esp_now_actor::*;
use actors::led_actor::*;
use actors::uart_actor::*;

mod motor;
use motor::*;

#[global_allocator]
pub static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

fn init_heap() {
    const HEAP_SIZE: usize = 32 * 1024;
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

/*fn mk_static<T>(val: T) -> &'static T {
    Box::leak(Box::new(val))
}*/

const HB_ID: u32 = msg::fnv("lm1/hb");

#[main]
async fn main(_spawner: Spawner) -> ! {
    //esp_info::logger::init_logger_from_env();
    init_heap();
    let _ = limero::init_logger();
    log::info!("ESP32-HOVERBOARD bridge starting...");

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
    let (tx_pin, rx_pin) = (io.pins.gpio16, io.pins.gpio17); // was 17,16

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin
    let mut frame_extractor = FrameExtractor::new();

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;
    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    let uart = Uart::new_async_with_config(
        peripherals.UART2,
        Config {
            baudrate: 115200,
            data_bits: DataBits::DataBits8,
            parity: Parity::ParityNone,
            stop_bits: StopBits::STOP1,
            clock_source: ClockSource::Apb,
            rx_fifo_full_threshold: 64,
            rx_timeout: Some(10),
        },
        &clocks,
        rx_pin, // RXD
        tx_pin, // TXD
    )
    .unwrap();

    let mut uart_actor = UartActor::<UART2>::new(uart, 0x00);
    let uart_handler = mk_static!(Endpoint<UartCmd>, uart_actor.handler());

    // controller that translates PS4 messages to motor commands
    let mut controller = HbController::new();
    // esp_now_acor >> motor_cmd >> uart_actor
    esp_now_actor.for_all(move |ev| {
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

        let _ = esp_now_to_controller(&mut controller, &data)
            .map(|motor_cmd| {
                info!("MotorCmd: {:?}", motor_cmd);
                uart_handler.handle(&UartCmd::Txd(motor_cmd.encode()));
            })
            .map_err(|e| {
                warn!("Error: {:?}", e);
            });
    });

    let espnow_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());

    // uart_actor >> motor_deframer >> espnow_actor; send uart data to espnow raw data
    uart_actor.for_all(move |msg: &UartEvent| match msg {
        UartEvent::Rxd(raw) => {
            debug!("Raw data received: {}", raw.len());
            let vecs = frame_extractor.decode(raw);
            for data in vecs {
                debug!("UartEvent: {:?}", data);
                espnow_handler.handle(&EspNowCmd::Broadcast { data });
            }
        }
    });

    loop {
        select3(uart_actor.run(), esp_now_actor.run(), led_actor.run()).await;
    }
}

fn event_to_blink(ev: &EspNowEvent) -> Option<LedCmd> {
    match ev {
        EspNowEvent::Rxd {
            peer: _,
            channel: _,
            rssi: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
        EspNowEvent::Broadcast {
            peer: _,
            channel: _,
            rssi: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
    }
}

fn esp_now_to_controller(controller: &mut HbController, data: &Vec<u8>) -> Result<MotorCmd> {
    let mut decoder = minicbor::Decoder::new(&data);
    let msg_header = decoder.decode::<MsgHeader>()?;
    if msg_header.is_msg(MsgType::Pub, None, Some(msg::ps4::PS4_ID))
    // PUB from PS4
    {
        let ev = decoder.decode::<msg::ps4::Ps4Map>()?;
/* 
        ev.stick_left_x
            .filter(|x| *x > 100)
            .map(|_| controller.change_steer(-5));
        ev.stick_left_x
            .filter(|x| *x < -100)
            .map(|_| controller.change_steer(5));
        ev.stick_left_y
            .filter(|x| *x > 50 || *x < -50)
            .map(|_| controller.straight());

        ev.stick_right_y
            .filter(|x| *x > 100)
            .map(|_| controller.change_speed(-5));
        ev.stick_right_y
            .filter(|x| *x < -100)
            .map(|_| controller.change_speed(5));
        ev.stick_right_x
            .filter(|x| *x > 50 || *x < -50)
            .map(|_| controller.stop());
        */
        ev.stick_right_x
            .map(|x| controller.steer((x*2) as i16));
        ev.stick_right_y
            .map(|y| controller.speed((y*2) as i16));

        Ok(controller.motor_cmd())
    } else if msg_header.is_msg(MsgType::Pub, Some(HB_ID), None) {
        // PUB to HB
        let ev = decoder.decode::<msg::hb::HbMap>()?;

        ev.speed.map(|speed| controller.speed(speed));
        ev.steer.map(|steer| controller.steer(steer));

        Ok(controller.motor_cmd())
    } else {
        Err(anyhow::Error::msg("Invalid message"))
    }
}

struct HbController {
    motor_speed: i16,
    motor_steer: i16,
    changed: bool,
}

impl HbController {
    fn new() -> HbController {
        HbController {
            motor_speed: 0,
            motor_steer: 0,
            changed: false,
        }
    }

    fn speed(&mut self, speed: i16) {
        self.motor_speed = speed;
    }

    fn steer(&mut self, steer: i16) {
        self.motor_steer = steer;
    }

    fn change_speed(&mut self, delta: i16) {
        self.motor_speed += delta;
        self.changed = true;
    }

    fn change_steer(&mut self, delta: i16) {
        self.motor_steer += delta;
        self.changed = true;
    }

    fn motor_cmd(&self) -> MotorCmd {
        MotorCmd {
            speed: self.motor_speed,
            steer: self.motor_steer,
        }
    }

    fn straight(&mut self) {
        self.motor_steer = 0;
        self.changed = true;
    }

    fn stop(&mut self) {
        self.motor_speed = 0;
        self.motor_steer = 0;
        self.changed = true;
    }
}
