use core::fmt::Debug;

use alloc::boxed::Box;
use alloc::format;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use esp_hal::{Async, Blocking};
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, UartRx, UartTx,Uart},
};

use alloc::string::String;
use alloc::vec::Vec;
use log::{debug, info};
use minicbor::decode::info;

use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex,
    channel::{DynamicReceiver, DynamicSender},
};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use crate::limero::{Flow, Sink, SinkRef, SinkTrait, Source, SourceTrait};
use crate::protocol::msg::ProxyMessage;
use crate::protocol::MessageDecoder;
use crate::protocol::{decode_frame, encode_frame};

pub const UART_BUFSIZE: usize = 127;

#[derive(Clone)]
pub enum TransportCmd {
    SendMessage { message: ProxyMessage },
}

#[derive(Clone)]
pub enum TransportEvent {
    RecvMessage { message: ProxyMessage },
}

pub struct UartActor {
    command: Sink<ProxyMessage, 5>,
    events: Source<ProxyMessage>,
    tx: UartTx<'static, UART0,Async>,
    rx: UartRx<'static, UART0,Async>,
    message_decoder: MessageDecoder,
}

impl UartActor {
    pub fn new(mut uart0: Uart<'static, UART0,Async>) -> Self {
        // Split UART0 to create seperate Tx and Rx handles
        let (tx, rx) = uart0.split();
        Self {
            command: Sink::new(),
            events: Source::new(),
            tx,
            rx,
            message_decoder: MessageDecoder::new(),
        }
    }

    pub fn sink_ref(&self) -> SinkRef<ProxyMessage> {
        self.command.sink_ref()
    }

    pub async fn run(&mut self) {
        let mut small_buf = [0u8; 100];
        info!("UART0 running");
        // Spawn Tx and Rx tasks
        loop {
            match select(
                self.rx.read_async(&mut small_buf),
                self.command.next(),
            )
            .await
            {
                First(r) => {
                    match r {
                        Ok(r) => {
                            let line:String  = small_buf[0..r].iter().map(|b| format!("{:02X} ", b)).collect();
                            debug!("Rx bytes : {:?}",line);
                            self.on_bytes_rxd( &mut small_buf[0..r]);
                        }
                        Err(e) => {
                            info!("Rx error {:?}", e);
                        }
                    }
                }
                Second(msg) => {
                    self.on_cmd_msg(msg.unwrap()).await;
                }
            }
        }
    }

    async fn on_cmd_msg(&mut self, msg: ProxyMessage) {
        debug!("TXD: {:?}", msg);
        let bytes = encode_frame(msg).unwrap();
        let line:String  = bytes.iter().map(|b| format!("{:02X} ", b)).collect();
        debug!(" TXD {}",line);
        let _ = embedded_io_async::Write::write(&mut self.tx, bytes.as_slice()).await;
        let _ = embedded_io_async::Write::flush(&mut self.tx).await;
    }
    fn on_bytes_rxd(&mut self,  small_buf: &mut [u8]) {
        let v = self.message_decoder.decode(small_buf);
        // Read characters from UART into read buffer until EOT
        for msg in v {
            debug!("RXD: {:?}", msg);
            self.events.emit(msg);
        }
    }
    
}

impl SourceTrait<ProxyMessage> for UartActor {
    fn add_listener(&mut self, sink: SinkRef<ProxyMessage>) {
        self.events.add_listener(sink);
    }
}
