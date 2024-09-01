use core::fmt::Debug;

use alloc::boxed::Box;
use alloc::format;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use esp_hal::{
    clock::ClockControl,
    peripherals::{Peripherals, UART0},
    prelude::*,
    uart::{config::AtCmdConfig, Uart, UartRx, UartTx},
};
use esp_hal::{Async, Blocking};

use alloc::string::String;
use alloc::vec::Vec;
use log::{debug, info};
use minicbor::decode::info;

use embassy_sync::{
    blocking_mutex::raw::CriticalSectionRawMutex,
    channel::{DynamicReceiver, DynamicSender},
};
use embassy_sync::{blocking_mutex::raw::NoopRawMutex, channel::Channel};

use anyhow::Result;
use anyhow::Error;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, Endpoint, EventHandlers, Handler};
use serdes::deframe;
use serdes::frame;
use serdes::FrameExtractor;

pub const UART_BUFSIZE: usize = 127;

#[derive(Clone)]
pub enum UartCmd {
    TransmitFrame(Vec<u8>),
}

#[derive(Clone)]
pub enum UartEvent {
    ReceivedFrame(Vec<u8>),
}

pub struct UartActor {
    cmds: CmdQueue<UartCmd>,
    events: EventHandlers<UartEvent>,
    timers: Timers,
    tx: UartTx<'static, UART0, Async>,
    rx: UartRx<'static, UART0, Async>,
    frame_extractor: FrameExtractor,
}

impl UartActor {
    pub fn new(mut uart0: Uart<'static, UART0, Async>) -> Self {
        // Split UART0 to create seperate Tx and Rx handles
        uart0.set_at_cmd(AtCmdConfig {
            // catch sentinel char 0x00
            pre_idle_count: Some(1),
            post_idle_count: Some(1),
            gap_timeout: Some(1),
            cmd_char: 0u8,
            char_num: Some(1),
        });
        let (tx, rx) = uart0.split();
        Self {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            tx,
            rx,
            frame_extractor: FrameExtractor::new(),
        }
    }
}

impl Actor<UartCmd, UartEvent> for UartActor {
    async fn run(&mut self) {
        let mut small_buf = [0u8; 100];
        info!("UART0 running");
        // Spawn Tx and Rx tasks
        loop {
            match select(self.rx.read_async(&mut small_buf), self.cmds.next()).await {
                First(r) => match r {
                    Ok(r) => {
                        let line: String = small_buf[0..r]
                            .iter()
                            .map(|b| format!("{:02X} ", b))
                            .collect();
                        debug!("Rx bytes : {:?}", line);
                        self.on_bytes_rxd(&mut small_buf[0..r]);
                    }
                    Err(e) => {
                        info!("Rx error {:?}", e);
                    }
                },
                Second(msg) => {
                    if let Err(e) = self.on_cmd_msg(msg.unwrap()).await {
                        info!("Cmd error {:?}", e);
                    }
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Endpoint<UartEvent>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Endpoint<UartCmd> {
        self.cmds.handler()
    }
}

impl UartActor {
    async fn on_cmd_msg(&mut self, msg: UartCmd) -> Result<()> {
        match msg {
            UartCmd::TransmitFrame(bytes) => {
                let bytes = frame(&bytes)?;
                let line: String = bytes.iter().map(|b| format!("{:02X} ", b)).collect();
                debug!(" TXD {}", line);
                embedded_io_async::Write::write(&mut self.tx, bytes.as_slice())
                    .await
                    .map_err(|e| Error::msg(format!("{:?}",e)))?;
                embedded_io_async::Write::flush(&mut self.tx)
                    .await
                    .map_err(|e| Error::msg(format!("{:?}",e)))?;
            }
        }
        Ok(())
    }
    fn on_bytes_rxd(&mut self, small_buf: &mut [u8]) {
        let v = self.frame_extractor.decode(small_buf);
        // Read characters from UART into read buffer until EOT
        for msg in v {
            debug!("RXD: {:?}", msg);
            self.events.handle(&UartEvent::ReceivedFrame(msg));
        }
    }
}
