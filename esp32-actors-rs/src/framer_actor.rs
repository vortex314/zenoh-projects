
use alloc::vec::Vec;
use log::info;


use limero::{Actor, CmdQueue, Endpoint, EventHandlers, Handler};

pub enum FramerOption {
    Cobs,
    Crc,
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

pub struct FramerActor {
    cmds: CmdQueue<FramerCmd>,
    event_handlers: EventHandlers<FramerEvent>,
    frame_extractor: msg::framer::FrameExtractor,
}

impl FramerActor {
    pub fn new(_framer_options: Vec<FramerOption>) -> Self {
        Self {
            cmds: CmdQueue::new(5),
            event_handlers: EventHandlers::new(),
            frame_extractor: msg::framer::FrameExtractor::new(),
        }
    }
}

impl Actor<FramerCmd, FramerEvent> for FramerActor {
    async fn run(&mut self) {
        info!("Framer running");
        // Spawn Tx and Rx tasks
        loop {
            let cmd = self.cmds.next().await;
            if let Some(cmd) = cmd {
                match cmd {
                    FramerCmd::Frame(data) => {
                        let _ = msg::framer::cobs_crc_frame(&data).inspect(|framed| {
                            self.event_handlers.handle(&FramerEvent::Framed(framed.clone()));
                        });
                    }
                    FramerCmd::Deframe(data) => {
                        let frames =  self.frame_extractor.decode(&data);
                        frames.iter().for_each(|deframed| {
                            self.event_handlers.handle(&FramerEvent::Deframed(deframed.clone()));
                        });
                    }
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Endpoint<FramerEvent>) {
        self.event_handlers.add_listener(listener);
    }

    fn handler(&self) -> Endpoint<FramerCmd> {
        self.cmds.handler()
    }
}
