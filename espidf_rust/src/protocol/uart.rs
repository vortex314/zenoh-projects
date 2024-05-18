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
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, channel::{DynamicReceiver, DynamicSender}};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

pub const UART_BUFSIZE: usize = 127;

pub struct UartActor {
    tx: UartTx<'static, UART0>,
    rx: UartRx<'static, UART0>,
    txd_msg : DynamicReceiver<'static,ProxyMessage>,
    rxd_msg : DynamicSender<'static,ProxyMessage>,
}

impl UartActor {
    pub fn new(
        uart0: UART0,
        txd_msg: DynamicReceiver<'static,ProxyMessage>,
        rxd_msg: DynamicSender<'static,ProxyMessage>,
    ) -> Self {
        uart0
            .set_rx_fifo_full_threshold(UART_BUFSIZE as u16)
            .unwrap();
        // Split UART0 to create seperate Tx and Rx handles
        let (tx, rx) = uart0.split();
        Self { tx, rx,txd_msg,rxd_msg }
    }
    pub async fn run(&mut self) {
        info!("UART0 initialized");
        // Spawn Tx and Rx tasks
        self.uart_reader(self.rx);
        self.uart_writer(self.tx);
    }

    async fn uart_reader(&self,mut rx: UartRx<'static, UART0>) {
        // Declare read buffer to store Rx characters
        let mut rbuf = [0u8; UART_BUFSIZE];

        let mut message_decoder = MessageDecoder::new();
        loop {
            info!("Waiting for message");
            let r = embedded_io_async::Read::read(&mut rx, &mut rbuf[0..]).await;
            match r {
                Ok(_cnt) => {
                    let v = message_decoder.decode(&mut rbuf);
                    // Read characters from UART into read buffer until EOT
                    for msg in v {
                        self.rxd_msg.send(msg).await;
                    }
                }
                Err(_) => {
                    continue;
                }
            }
        }
    }

    async fn uart_writer(&self,mut tx: UartTx<'static, UART0>) {
        loop {
            let msg = self.txd_msg.receive().await;
            info!("Sending message {:?}", msg);
            let bytes = encode_frame(msg).unwrap();
            let _ = embedded_io_async::Write::write(&mut tx, bytes.as_slice()).await;
            let _ = embedded_io_async::Write::flush(&mut tx).await;
        }
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
