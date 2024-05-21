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

use super::decode_frame;
use super::encode_frame;
use super::msg::ProxyMessage;
use super::Handler;
use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex,
    channel::{DynamicReceiver, DynamicSender},
};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

pub const UART_BUFSIZE: usize = 127;
static TXD_MSG: Channel<CriticalSectionRawMutex, ProxyMessage, 5> = Channel::new();

pub struct UartActor {
    tx: UartTx<'static, UART0>,
    rx: UartRx<'static, UART0>,
    rxd_msg: Handler<'static, ProxyMessage>,
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
            rxd_msg: Handler::new(),
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
                    match r {
                        Ok(_cnt) => {
                            let v = message_decoder.decode(&mut rbuf);
                            // Read characters from UART into read buffer until EOT
                            for msg in v {
                                self.rxd_msg.emit(msg);
                            }
                        }
                        Err(_) => {
                            continue;
                        }
                    }
                }
                Second(msg) => {
                    let bytes = encode_frame(msg).unwrap();
                    let _ = embedded_io_async::Write::write(&mut self.tx, bytes.as_slice()).await;
                    let _ = embedded_io_async::Write::flush(&mut self.tx).await;
                }
            }
        }
    }

    pub fn txd_sink(&self) -> DynamicSender<'_, ProxyMessage> {
        TXD_MSG.dyn_sender()
    }

    pub fn rxd_source(&self) -> &mut Handler<ProxyMessage> {
        &mut self.rxd_msg 
    }
}



pub struct MessageDecoder {
    buffer: Vec<u8>,
}

impl MessageDecoder {
    pub fn new() -> Self {
        Self { buffer: Vec::new() }
    }

    pub fn decode(&mut self, data: &[u8]) -> Vec<ProxyMessage> {
        let mut messages_found = Vec::new();
        for byte in data {
            self.buffer.push(*byte);
            if *byte == 0 {
                // decode cobs from frame
                let msg = decode_frame(&self.buffer);
                msg.into_iter().for_each(|m| {
                    messages_found.push(m);
                });
                self.buffer.clear();
            }
        }
        messages_found
    }

    pub fn to_str(&self) -> String {
        match String::from_utf8(self.buffer.clone()) {
            Ok(s) => s,
            Err(_) => String::from(""),
        }
    }
}
