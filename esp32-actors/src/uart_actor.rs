use alloc::format;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};
use esp_hal::prelude::_esp_hal_uart_Instance;
use esp_hal::uart::Instance;
use esp_hal::Async;
use esp_hal::{
    peripherals::UART0,
    peripherals::UART1,
    peripherals::UART2,
    uart::{config::AtCmdConfig, Uart, UartRx, UartTx},
};

use alloc::string::String;
use alloc::vec::Vec;
use log::{debug, info};

use anyhow::Error;
use anyhow::Result;
use limero::timer::Timers;
use limero::{Actor, CmdQueue, Endpoint, EventHandlers, Handler};
use serdes::cobs_crc_frame;
use serdes::FrameExtractor;

pub const UART_BUFSIZE: usize = 127;

#[derive(Clone)]
pub enum UartCmd {
    Txd(Vec<u8>),
}

#[derive(Clone, Debug)]
pub enum UartEvent {
    Rxd(Vec<u8>),
}

pub struct UartActor<UART>
where
    UART: 'static,
{
    cmds: CmdQueue<UartCmd>,
    events: EventHandlers<UartEvent>,
    timers: Timers,
    tx: UartTx<'static, UART, Async>,
    rx: UartRx<'static, UART, Async>,
}

impl<UART> UartActor<UART>
where
    UART: _esp_hal_uart_Instance,
{
    pub fn new(mut uart: Uart<'static, UART, Async>) -> Self {
        // Split UART to create seperate Tx and Rx handles
        uart.set_at_cmd(AtCmdConfig {
            // catch sentinel char 0x00
            pre_idle_count: Some(1),
            post_idle_count: Some(1),
            gap_timeout: Some(1),
            cmd_char: 0xABu8,
            char_num: Some(1),
        });
        let (tx, rx) = uart.split();
        Self {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            tx,
            rx,
        }
    }
}

impl<UART> Actor<UartCmd, UartEvent> for UartActor<UART>
where
    UART: _esp_hal_uart_Instance,
{
    async fn run(&mut self) {
        let mut small_buf = [0u8; 100];
        info!("UART running");
        // Spawn Tx and Rx tasks
        loop {
            match select(self.rx.read_async(&mut small_buf), self.cmds.next()).await {
                First(r) => match r {
                    Ok(r) => {
                      //  info!("Rx {:?}", r);
                        self.events
                            .handle(&UartEvent::Rxd(small_buf[0..r].to_vec()));
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

impl<UART> UartActor<UART>
where
    UART: _esp_hal_uart_Instance,
{
    async fn on_cmd_msg(&mut self, msg: UartCmd) -> Result<()> {
        match msg {
            UartCmd::Txd(bytes) => {
//                info!("Tx {:X?}", bytes    );
                embedded_io_async::Write::write(&mut self.tx, bytes.as_slice())
                    .await
                    .map_err(|e| Error::msg(format!("{:?}", e)))?;
                embedded_io_async::Write::flush(&mut self.tx)
                    .await
                    .map_err(|e| Error::msg(format!("{:?}", e)))?;
            }
        }
        Ok(())
    }
}
