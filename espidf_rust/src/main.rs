#![no_std]
#![no_main]
#![feature(alloc)]
#![feature(type_alias_impl_trait)]
use core::{cell::RefCell, mem::MaybeUninit};

extern crate alloc;

#[global_allocator]
static ALLOCATOR: esp_alloc::EspHeap = esp_alloc::EspHeap::empty();

use alloc::rc::Rc;
use embassy_executor::Spawner;
use embassy_sync::{
    blocking_mutex::raw::{ NoopRawMutex},
    channel::{Channel},
};
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    embassy,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
    Uart,
};
use log::info;

mod protocol;

use protocol::Message;

const MTU: usize = 1024;

#[embassy_executor::task]
async fn uart_writer(
    mut tx: UartTx<'static, UART0>,
    receiver: embassy_sync::channel::Receiver<'static, NoopRawMutex, Message, 10>,
) {
    loop {
        let msg = receiver.receive().await;
        let bytes = protocol::make_frame(msg).unwrap();
        embedded_io_async::Write::write(&mut tx, bytes.as_slice()).await;
        embedded_io_async::Write::flush(&mut tx).await;
    }
}

#[embassy_executor::task]
async fn uart_reader(
    mut rx: UartRx<'static, UART0>,
    sender: embassy_sync::channel::Sender<'static, NoopRawMutex, Message, 10>,
) {
    // Declare read buffer to store Rx characters
    const READ_BUF_SIZE: usize = 1024;
    let mut rbuf = [0u8; READ_BUF_SIZE];

    let mut message_decoder = protocol::MessageDecoder::new();
    loop {
        let r = embedded_io_async::Read::read(&mut rx, &mut rbuf[0..]).await;
        match r {
            Ok(cnt) => {
                let v = message_decoder.decode(&mut rbuf);
                // Read characters from UART into read buffer until EOT
                for msg in v {
                    sender.send(msg).await;
                }
            }
            Err(_) => {
                continue;
            }
        }
    }
}

fn init_heap() {
    const HEAP_SIZE: usize = 100 * 1024;
    static mut HEAP: MaybeUninit<[u8; HEAP_SIZE]> = MaybeUninit::uninit();

    unsafe {
        ALLOCATOR.init(HEAP.as_mut_ptr() as *mut u8, HEAP_SIZE);
    }
}

const READ_BUF_SIZE:usize=1024;

// static uart_channel_in:Rc<RefCell<Option<Channel<NoopRawMutex,Message,10>>>>=Rc::new(RefCell::new(None));

#[main]
async fn main(spawner: Spawner) {
    init_heap();
    let peripherals = Peripherals::take();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();

    // Initialize Embassy with needed timers
    let timer_group0 = esp_hal::timer::TimerGroup::new(peripherals.TIMG0, &clocks);
    embassy::init(&clocks, timer_group0);

    // Initialize and configure UART0
    let mut uart0 = Uart::new(peripherals.UART0, &clocks);
//    uart0.set_at_cmd(AtCmdConfig::new(None, None, None, AT_CMD, None));
    uart0
        .set_rx_fifo_full_threshold(READ_BUF_SIZE as u16)
        .unwrap();
    // Split UART0 to create seperate Tx and Rx handles
    let (tx, rx) = uart0.split();
    let uart_channel_out = Channel::<NoopRawMutex,Message,10>::new(); 
    let uart_channel_in = Channel::<NoopRawMutex,Message,10>::new();

    // Spawn Tx and Rx tasks
    spawner.spawn(uart_reader(rx,uart_channel_in.sender())).ok();
    spawner.spawn(uart_writer(tx,uart_channel_out.receiver())).ok();
    loop {};
}
