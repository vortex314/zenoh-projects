use alloc::vec::Vec;

use esp_hal::peripherals::Peripherals;
use esp_hal::clock::Clocks;
use esp_hal::mcpwm:: PeripheralClockConfig;
use  esp_hal::McPwm::MCPWM;

use limero::Actor;
use limero::CmdQueue;
use limero::EventHandlers;
use limero::Timers;



use embassy_futures::select::select;
use embassy_futures::select::Either;
use msg::dc_motor::*;
use msg::dc_motor::DC_MOTOR_ID;
use msg::MsgHeader;
use log::info;

use minicbor::Decoder;

#[derive(Clone)]
pub enum DcMotorCmd {
    Rpm { rpm: i16 },
    Stop {},
    Request { msg_header:MsgHeader ,data : Vec<u8> },
}

pub enum DcMotorEvent {}

pub struct DcMotorActor {
    cmds: CmdQueue<DcMotorCmd>,
    event_handlers: EventHandlers<DcMotorEvent>,
    timers: Timers,
    mcpwm: MCPWM,
}

impl DcMotorActor {
    pub fn new(clocks: &Clocks<'_>,peripherals:&Peripherals) -> DcMotorActor {
        let clock_cfg = PeripheralClockConfig::with_frequency(clocks, 40u32.MHz()).unwrap();
        let mut mcpwm = MCPWM::new(peripherals.PWM0, clock_cfg);
        mcpwm.set_period(0, 1000u32.Hz());
        DcMotorActor {
            cmds: CmdQueue::new(3),
            event_handlers: EventHandlers::new(),
            timers: Timers::new(),
            mcpwm: mcpwm,
        }
    }
}

impl Actor<DcMotorCmd, DcMotorEvent> for DcMotorActor {
    async fn run(&mut self) {
        loop {
            let res = select(self.cmds.next(), self.timers.alarm()).await;
            match res {
                Either::First(cmd) => match cmd.unwrap() {
                    DcMotorCmd::Rpm { rpm } => {
                        self.mcpwm.set_duty(0, 0, 1000u32.Hz(), 1000u32.Hz() / 2);
                    }
                    DcMotorCmd::Stop {} => {
                        self.mcpwm.set_duty(0, 0, 1000u32.Hz(), 0);
                    }
                    DcMotorCmd::Request { msg_header, data  } => {
                        let mut decoder = Decoder::new(&data);
                        decoder.decode::<MsgHeader>().map(|msg_header| {
                            msg_header.dst.map(|dst| {
                                if dst == DC_MOTOR_ID {
                                    decoder.decode::<DcMotorMap>().map(|dc_motor_cmd| {
                                        info!("DcMotorCmd {:?}", dc_motor_cmd);
                                    });
                                }
                            });
                        });
                    }
                },
                Either::Second(_) => {}
            }
        }
    }
    fn add_listener(&mut self, handler: alloc::boxed::Box<dyn limero::Handler<DcMotorEvent>>) {
        self.event_handlers.add_listener(handler);
    }
    fn handler(&self) -> alloc::boxed::Box<dyn limero::Handler<DcMotorCmd>> {
        self.cmds.handler()
    }
}
