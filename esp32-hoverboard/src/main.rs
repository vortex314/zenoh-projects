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
    peripherals::Peripherals,
    peripherals::UART2,
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
use log::{info, warn};
use minicbor::Decoder;
use minicbor::Encoder;
use serde::{Deserialize, Serialize};

extern crate alloc;
use alloc::{collections::vec_deque::VecDeque, vec::Vec};

use actors::esp_now_actor::*;
use actors::led_actor::*;
use actors::uart_actor::*;

use serdes::request;
use serdes::EspNowHeader;
use serdes::MotorCmd;
use serdes::MotorEvent;
use serdes::MsgType;
use serdes::Ps4Event;
use serdes::PS4_ID;
use serdes::START_FRAME;

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

struct MotorDeFramer {
    buffer: VecDeque<u8>,
    prev_byte: u8,
}

impl MotorDeFramer {
    fn write_byte(&mut self, byte: u8) -> Option<MotorEvent> {
        if byte == (START_FRAME >> 8) as u8 {
            if self.buffer.len() > 0 && self.prev_byte == (START_FRAME & 0xFF) as u8 {
                self.buffer.clear();
                self.buffer.push_back((START_FRAME & 0xFF) as u8);
                self.buffer.push_back((START_FRAME >> 8) as u8);
            //                info!("Restarting buffer");
            } else {
                self.buffer.push_back(byte);
            }
        } else {
            self.buffer.push_back(byte);
        }
        self.prev_byte = byte;
        //       info!("Received: {:02X?}", self.buffer);

        if self.buffer.len() > 100 {
            self.buffer.clear();
            info!("Buffer too long");
        }
        if self.buffer.len() == 18
            && self.buffer[1] == (START_FRAME >> 8) as u8
            && self.buffer[0] == (START_FRAME & 0xFF) as u8
        {
            //           info!("Received: {:02X?}", self.buffer);
            let mut event = MotorEvent::new();
            event.decode(&mut self.buffer);
            self.buffer.clear();
            Some(event)
        } else {
            None
        }
    }
}

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
    let mut motor_deframer = MotorDeFramer {
        buffer: VecDeque::new(),
        prev_byte: 0,
    };
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
        let (_peer, data) = match ev {
            EspNowEvent::Rxd { peer, data } => (peer, data),
            EspNowEvent::Broadcast {
                peer,
                rssi: _,
                data,
            } => (peer, data),
        };
        let mut decoder = Decoder::new(&data);
        //  info!("Received: {:?}", serdes::Cbor::to_string(&buffer));
        let maybe_header = EspNowHeader::from_decoder(&mut decoder);
        if let Ok(header) = maybe_header {
            if header.src.is_some() && header.src.unwrap() == PS4_ID {
                let ev = Ps4Event::from_decoder(&mut decoder);
                if let Ok(ev) = ev {
                    controller.update(&ev);
                    let motor_cmd = controller.motor_cmd();
                    uart_handler.handle(&UartCmd::Txd(motor_cmd.encode()));
                } 
            }
        } else {
            info!("Parse header fails from {:X?} : {:?} ", _peer, maybe_header);
        }
    });

    let espnow_handler = mk_static!(Endpoint<EspNowCmd>, esp_now_actor.handler());
    let last_motor_event = mk_static!(MotorEvent, MotorEvent::new());

    uart_actor.for_all(move |msg: &UartEvent| {
        match msg {
            UartEvent::Rxd(data) => {
                for b in data {
                    if let Some(ev) = motor_deframer.write_byte(*b) {
                        if *last_motor_event != ev {
                            info!(
                                "MotorEvent left : {} right : {} , temp : {} C , battery : {} V",
                                ev.speed_left, ev.speed_right,ev.board_temperature as f32 /10.0,ev.battery_voltage as f32 /100.0
                            );
                            *last_motor_event = ev.clone();
                            let header = EspNowHeader {
                                dst: None,
                                src: Some(serdes::fnv("lm/motor")),
                                msg_type: request(MsgType::PubReq),
                                msg_id: None,
                            };
                            let mut encoder = Encoder::new(Vec::new());
                            let _ = header.encode(&mut encoder);
                            let _ = ev.encode(&mut encoder);
                            let _ = encoder.end();
                            espnow_handler.handle(&EspNowCmd::Broadcast {
                                data: encoder.into_writer(),
                            });
                            

                        }
                    }
                }
                // info!("UART RXD {:?}", data.len());
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
        EspNowEvent::Rxd { peer: _, data: _ } => Some(LedCmd::Pulse { duration: 10 }),
        EspNowEvent::Broadcast {
            peer: _,
            rssi: _,
            data: _,
        } => Some(LedCmd::Pulse { duration: 10 }),
    }
}

struct Controller {
    motor_speed: i16,
    motor_steer: i16,
}

impl Controller {
    fn new() -> Controller {
        Controller {
            motor_speed: 0,
            motor_steer: 0,
        }
    }
    fn update(&mut self, ev: &Ps4Event) {
        let mut changed = false;
        if ev.right_axis_y > 100 {
            self.motor_speed -= 5;
            changed = true;
        } else if ev.right_axis_y < -100 {
            self.motor_speed += 5;
            changed = true;
        }
        if ev.left_axis_x > 100 {
            self.motor_steer += 5;
            changed = true;
        } else if ev.left_axis_x < -100 {
            self.motor_steer -= 5;
            changed = true;
        }
        if changed {
            info!(" speed {} steer {}", self.motor_speed, self.motor_steer);
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
    }
}