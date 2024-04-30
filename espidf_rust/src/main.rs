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

static TXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
static RXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();
const UART_BUFSIZE: usize = 127;

/*
wait for message to send, encode and send to UART
*/
#[embassy_executor::task]
async fn uart_writer(mut tx: UartTx<'static, UART0>) {
    loop {
        let msg = TXD_MSG.receive().await;
        info!("Sending message {:?}", msg);
        let bytes = protocol::encode_frame(msg).unwrap();
        let _ = embedded_io_async::Write::write(&mut tx, bytes.as_slice()).await;
        let _ = embedded_io_async::Write::flush(&mut tx).await;
    }
}
/*
handle UART input, convert to message and send to channel
*/
#[embassy_executor::task]
async fn uart_reader(mut rx: UartRx<'static, UART0>) {
    // Declare read buffer to store Rx characters
    let mut rbuf = [0u8; UART_BUFSIZE];

    let mut message_decoder = protocol::MessageDecoder::new();
    loop {
        info!("Waiting for message");
        let r = embedded_io_async::Read::read(&mut rx, &mut rbuf[0..]).await;
        match r {
            Ok(_cnt) => {
                let v = message_decoder.decode(&mut rbuf);
                // Read characters from UART into read buffer until EOT
                for msg in v {
                    RXD_MSG.send(msg).await;
                }
            }
            Err(_) => {
                continue;
            }
        }
    }
}

#[embassy_executor::task]
async fn fsm_connection() {
    #[derive(PartialEq)]
    enum State {
        Idle,
        Connected,
    }
    let mut state = State::Idle;
    loop {
        loop {
            // wait for connection
            info!("Waiting for connection");
            TXD_MSG
                .send(ProxyMessage::Connect {
                    protocol_id: 1,
                    duration: 100,
                    client_id: "myClient".to_string(),
                })
                .await;
//            RXD_MSG.send(ProxyMessage::ConnAck { return_code:0 }).await;
            let result = with_timeout(Duration::from_secs(5), RXD_MSG.receive()).await;
            match result {
                Ok(msg) => match msg {
                    ProxyMessage::ConnAck { return_code } => {
                        info!("Connected code  {}", return_code);
                        state = State::Connected;
                    }
                    _ => {
                        info!("Unexpected message {:?}", msg);
                    }
                },
                Err(_) => {
                    info!("Connection timeout");
                }
            }
            if state == State::Connected {
                break;
            }
        }
        loop {
            let mut timeouts = 0;
            // wait for message
            info!("Waiting for message");
            let result = with_timeout(Duration::from_secs(5), RXD_MSG.receive()).await;
            match result {
                Ok(msg) => match msg {
                    ProxyMessage::Publish { topic_id, message } => {
                        info!("Received message on topic {} : {}", topic_id, message);
                    }
                    _ => {
                        info!("Unexpected message {:?}", msg);
                    }
                },
                Err(_) => {
                    timeouts+=1;
                    TXD_MSG
                        .send(ProxyMessage::PingReq {
                        })
                        .await;
                }
            }
        }
    }
}

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
