//! Embassy ESP-NOW Example (Duplex)
//!
//! Asynchronously broadcasts, receives and sends messages via esp-now in multiple embassy tasks
//!
//! Because of the huge task-arena size configured this won't work on ESP32-S2

//% FEATURES: async embassy embassy-generic-timers esp-wifi esp-wifi/async esp-wifi/embassy-net esp-wifi/wifi-default esp-wifi/wifi esp-wifi/utils esp-wifi/esp-now
//% CHIPS: esp32 esp32s3 esp32c2 esp32c3 esp32c6

#![no_std]
#![no_main]
#![allow(unused_imports)]
use core::{cell::RefCell, mem::MaybeUninit};
use embassy_executor::Spawner;
use embassy_futures::select::select;
use embassy_futures::select::select3;
use embassy_futures::select::Either::{First, Second};
use embassy_futures::select::{self};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, mutex::Mutex};
use embassy_time::{Duration, Ticker};

use esp_backtrace as _;
use esp_hal::peripherals::UART2;
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
use anyhow::Result;

use msg::PubSubCmd;

use actors::esp_now_actor::*;
use actors::framer_actor::*;
use actors::led_actor::*;
use actors::pubsub_actor::*;
use actors::sys_actor::*;
use actors::uart_actor::*;
use msg::framer::cobs_crc_frame;
use msg::hb::MotorCmd;
use msg::FrameExtractor;
use msg::MsgHeader;
use msg::MsgType;

use log::debug;

mod motor;
use motor::*;

const HB_ID: u32 = msg::fnv("lm1/hb");

#[main]
async fn main(_spawner: Spawner) -> ! {
    esp_alloc::heap_allocator!(32 * 1024);
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

    let led_pin = io.pins.gpio2;
    let led_pin: Output = Output::new(led_pin, Level::Low);
    let mut led_actor = LedActor::new(led_pin); // pass as OutputPin

    let uart2 = Uart::new_async_with_config(
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
        io.pins.gpio17,
        io.pins.gpio16,
    )
    .unwrap();

    // let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);

    let esp_now = esp_wifi::esp_now::EspNow::new(&init, peripherals.WIFI).unwrap();
    info!("esp-now version {}", esp_now.get_version().unwrap());
    let mut esp_now_actor = EspNowActor::new(esp_now);
    let mut frame_extractor = FrameExtractor::new();

    // esp_now_actor >> espnow_rxd_to_pulse >> led_actor;
    esp_now_actor.map_to(event_to_blink, led_actor.handler());

    let mut uart_actor = UartActor::<UART2>::new(uart2, 0x00);
    let mut uart_handler = uart_actor.handler();
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

    let mut espnow_handler = esp_now_actor.handler();

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
        ev.stick_right_x.map(|x| controller.steer((x * 2) as i16));
        ev.stick_right_y.map(|y| controller.speed((y * 2) as i16));

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
