use alloc::format;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};


use alloc::vec::Vec;
use log:: info;

use anyhow::Error;
use anyhow::Result;
use limero::{Actor, CmdQueue, Endpoint, EventHandlers, Handler};

pub enum FramerOption {
    Cobs,
    Crc
}

#[derive(Clone)]
pub enum FramerCmd {
    Frame(Vec<u8>),
    Deframe(Vec<u8>),
}

#[derive(Clone, Debug)]
pub enum FramerEvent {
    Framed(Vec<u8>),
    Deframed(Vec<u8>),
}

pub struct FramerActor
{
    cmds: CmdQueue<FramerCmd>,
    event_handlers: EventHandlers<FramerEvent>,
frame_extractor : FrameExtractor,
}

impl FramerActor
where
    UART: _esp_hal_uart_Instance,
{
    pub fn new(_framer_options: Vec<FramerOption>) -> Self {
        Self {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            frame_extractor: FrameExtractor::new(),
        }
    }
}

impl Actor<FramerCmd, FramerEvent> for FramerActor
{
    async fn run(&mut self) {
        info!("Framer running");
        // Spawn Tx and Rx tasks
        loop {
            let cmd = self.cmds.next()).await ;
            match cmd {
                FramerCmd::Frame(data) => {
                    let framed = msg::framer::cobs_crc_frame(data);
                    self.event_handlers.handle(FramerEvent::Framed(framed));
                }
                FramerCmd::Deframe(data) => {
                    let deframed = self.frame_extractor.decode(&data);
                    deframed.map(|deframed|

                    self.event_handlers.handle(FramerEvent::Deframed(deframed));
                }
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Endpoint<FramerEvent>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Endpoint<FramerCmd> {
        self.cmds.handler()
    }
}
