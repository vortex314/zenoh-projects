use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use esp_hal::{
    clock::ClockControl,
    embassy,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx},
    Uart,
};

use alloc::string::String;
use alloc::vec::Vec;
use log::info;
use minicbor::decode::info;

use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex,
    channel::{DynamicReceiver, DynamicSender},
};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};


use crate::stream::{Source, SourceTrait};
use crate::protocol::msg::ProxyMessage;
use crate::protocol::{decode_frame, encode_frame,};
use crate::protocol::MessageDecoder;

pub const UART_BUFSIZE: usize = 127;
static TXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();

pub struct UartActor {
    tx: UartTx<'static, UART0>,
    rx: UartRx<'static, UART0>,
    rxd_msg: Source<'static, ProxyMessage>,
    txd_msg: DynamicReceiver<'static, ProxyMessage>,
}

impl UartActor {
    pub fn new(mut uart0: Uart<'static, UART0>) -> Self {
        uart0
            .set_rx_fifo_full_threshold(UART_BUFSIZE as u16)
            .unwrap();
        // Split UART0 to create seperate Tx and Rx handles
        let (tx, rx) = uart0.split();
        Self {
            tx,
            rx,
            rxd_msg: Source::new(),
            txd_msg: TXD_MSG.dyn_receiver(),
        }
    }

    pub async fn run(&mut self) {
        let mut rbuf = [0u8; UART_BUFSIZE];
        let mut message_decoder = MessageDecoder::new();
        info!("UART0 running");
        // Spawn Tx and Rx tasks
        loop {
            let _res = select(
                embedded_io_async::Read::read(&mut self.rx, &mut rbuf[0..]),
                self.txd_msg.receive(),
            )
            .await;
            match _res {
                First(r) => {
                    info!("Received {} bytes", r.ok().unwrap());
                    match r {
                        Ok(_cnt) => {
                            let v = message_decoder.decode(&mut rbuf);
                            // Read characters from UART into read buffer until EOT
                            for msg in v {
                                info!("Received message: {:?}", msg);
                                self.rxd_msg.emit(&msg);
                            }
                        }
                        Err(_) => {
                            continue;
                        }
                    }
                }
                Second(msg) => {
                    info!("Send message: {:?}", msg);
                    let bytes = encode_frame(msg).unwrap();
                    let _ = embedded_io_async::Write::write(&mut self.tx, bytes.as_slice()).await;
                    let _ = embedded_io_async::Write::flush(&mut self.tx).await;
                }
            }
        }
    }

    pub fn txd_sink(&self) -> DynamicSender<'static, ProxyMessage> {
        TXD_MSG.dyn_sender()
    }

    pub fn add_rxd_sink(&mut self, sender: DynamicSender<'static, ProxyMessage>) {
        self.rxd_msg.add_sender(sender);
    }
}




