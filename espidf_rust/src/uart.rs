use alloc::boxed::Box;
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

use crate::limero::{Flow, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::protocol::msg::MqttSnMessage;
use crate::protocol::MessageDecoder;
use crate::protocol::{decode_frame, encode_frame};

pub const UART_BUFSIZE: usize = 127;

pub struct UartActor {
    command: Sink<MqttSnMessage, 4>,
    events: Source<MqttSnMessage>,
    tx: UartTx<'static, UART0>,
    rx: UartRx<'static, UART0>,
    rxd_msg: Source<MqttSnMessage>,
    rxd_buffer: Vec<u8>,
}

impl UartActor {
    pub fn new(mut uart0: Uart<'static, UART0>) -> Self {
        uart0
            .set_rx_fifo_full_threshold(UART_BUFSIZE as u16)
            .unwrap();
        // Split UART0 to create seperate Tx and Rx handles
        let (tx, rx) = uart0.split();
        Self {
            command: Sink::new(),
            events: Source::new(),
            tx,
            rx,
            rxd_msg: Source::new(),
            rxd_buffer: Vec::new(),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<MqttSnMessage,4> {
        self.command.sink_ref()
    }

    pub async fn run(&mut self) {
        let mut _rbuf = [0u8; UART_BUFSIZE];
        let mut small_buf = [0u8; 1];
        let mut message_decoder = MessageDecoder::new();
        info!("UART0 running");
        // Spawn Tx and Rx tasks
        loop {
            let _res = select(
                //              self.rx.read(&mut rbuf[0..]),
                embedded_io_async::Read::read(&mut self.rx, &mut small_buf),
                self.command.read(),
            )
            .await;
            match _res {
                First(r) => {
                    info!("Received {} bytes", r.ok().unwrap());
                    match r {
                        Ok(_cnt) => {
                            let v = message_decoder.decode(&mut small_buf);
                            // Read characters from UART into read buffer until EOT
                            for msg in v {
                                info!("Received message: {:?}", msg);
                                self.rxd_msg.emit(msg);
                            }
                        }
                        Err(_) => {
                            continue;
                        }
                    }
                }
                Second(msg) => {
                    info!("Send message: {:?}", msg);
                    let bytes = encode_frame(msg.unwrap()).unwrap();
                    let _ = embedded_io_async::Write::write(&mut self.tx, bytes.as_slice()).await;
                    let _ = embedded_io_async::Write::flush(&mut self.tx).await;
                }
            }
        }
    }
}
