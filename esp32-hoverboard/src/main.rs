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
use embassy_futures::select::select;

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
use log::warn;
use log::info;
use msg::{framer::cobs_crc_deframe, FrameExtractor};
use serde::{Deserialize, Serialize};

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
    let (tx_pin, rx_pin) = (io.pins.gpio17, io.pins.gpio16);

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, wifi).unwrap();
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin
    let mut motor_deframer = FrameExtractor::new();
    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    #[derive(Debug, Serialize, Deserialize)]
    struct UartMsg([u8; 6], Vec<u8>);

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;

    let uart = Uart::new_async_with_config(
        peripherals.UART2,
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
        rx_pin, // RXD
        tx_pin, // TXD
    )
    .unwrap();

    let mut uart_actor = UartActor::<UART2>::new(uart);

    let uart_handler = mk_static!(Endpoint<UartCmd>, uart_actor.handler());
    let mut controller = Controller::new();

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

        let r = event_to_motor(&mut controller, &data).map(|_| {
            let motor_cmd = controller.motor_cmd();
            info!("MotorCmd: {:?}", motor_cmd);
            uart_handler.handle(&UartCmd::Txd(motor_cmd.encode()));
        }).map_err(|e| {
            warn!("Error: {:?}", e);
        });
    });

    let espnow_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());

    uart_actor.for_all(move |msg: &UartEvent| match msg {
        UartEvent::Rxd(data) => {
            let v = motor_deframer.decode(data);
            for data in v {
                let _ = cobs_crc_deframe(&data).map(|data| {
                    espnow_handler.handle(&EspNowCmd::Broadcast { data });
                });
            }
        }
    });

    loop {
        select(
            uart_actor.run(),
            select(esp_now_actor.run(), led_actor.run()),
        )
        .await;
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

fn event_to_motor(controller: &mut Controller, data: &Vec<u8>) -> Result<()> {
    let mut msg_decoder = msg::MsgDecoder::new(&data);
    let msg_header = msg_decoder.decode_header()?;
    if msg_header.src.is_some()
        && msg_header.src.unwrap() == msg::ps4::PS4_ID
        && msg_header.msg_type as u8 == msg::MsgType::Pub as u8
    {
        let _ = msg_decoder
            .find_in_map(msg::ps4::Ps4::StickLeftX as i8)
            .map(|_| {
                if let Ok(left_axis_x) = msg_decoder.decode::<i16>() {
                    controller.on_x_axis(left_axis_x);
                }
            });
        let _ = msg_decoder
            .find_in_map(msg::ps4::Ps4::StickRightY as i8)
            .map(|_| {
                if let Ok(right_axis_y) = msg_decoder.decode::<i16>() {
                    controller.on_y_axis(right_axis_y);
                }
            });

        Ok(())
    } else {
        Err(anyhow::Error::msg("Invalid message"))
    }
}

struct Controller {
    motor_speed: i16,
    motor_steer: i16,
    changed: bool,
}

impl Controller {
    fn new() -> Controller {
        Controller {
            motor_speed: 0,
            motor_steer: 0,
            changed: false,
        }
    }
    fn on_x_axis(&mut self, left_axis_x: i16) {
        if left_axis_x > 100 {
            self.motor_steer += 5;
            self.changed = true;
        } else if left_axis_x < -100 {
            self.motor_steer -= 5;
            self.changed = true;
        }
    }
    fn on_y_axis(&mut self, right_axis_y: i16) {
        if right_axis_y > 100 {
            self.motor_speed -= 5;
            self.changed = true;
        } else if right_axis_y < -100 {
            self.motor_speed += 5;
            self.changed = true;
        }
    }
    fn motor_cmd(&self) -> MotorCmd {
        MotorCmd {
            speed: self.motor_speed,
            steer: self.motor_steer,
        }
    }

    fn reset(&mut self) {
        self.motor_speed = 0;
        self.motor_steer = 0;
        self.changed = false;
    }
}
