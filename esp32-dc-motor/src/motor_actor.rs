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

const DC_MOTOR_ID = fnv("lm1/cutter")

#[derive(Encode,Decode,Default,Debug,Clone)]
#[cbor(map)]
pub struct DcMotorMsg {
    #[n(0)] pub target_rpm: Option<i16>,
    #[n(1)] pub measured_rpm: Option<i16>,
    #[n(2)] pub current_left: Option<f32>,
    #[n(3)] pub current_right: Option<f32>,
}

#[derive(Clone)]
pub enum DcMotorCmd {
    Rpm { rpm: i16 },
    Stop {},
    Request { msg_header:MsgHeader , msg : DcMotorMsg },
}

pub enum DcMotorEvent {}

pub struct DcMotorActor {
    cmds: CmdQueue<DcMotorCmd>,
    event_handlers: EventHandlers<DcMotorEvent>,
    timers: Timers,
    mcpwm: MCPWM,
    id : u32,
    str_id : String,
}

impl DcMotorActor {
    pub fn new(clocks: &Clocks<'_>,peripherals:&Peripherals,str_id : &str) -> DcMotorActor {
        let clock_cfg = PeripheralClockConfig::with_frequency(clocks, 40u32.MHz()).unwrap();
        let mut mcpwm = MCPWM::new(peripherals.PWM0, clock_cfg);
        mcpwm.set_period(0, 1000u32.Hz());
        DcMotorActor {
            cmds: CmdQueue::new(3),
            event_handlers: EventHandlers::new(),
            timers: Timers::new(),
            mcpwm: mcpwm,
            id : fnv(str_id),
            str_id,
        }
    }
    fn on_cmd(&mut self, cmd: DcMotorCmd) {
        let deactivation_timer = self.timers.timer(1, 1000u32.ms());
        match cmd {
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
                                match dc_motor_cmd {
                                    DcMotorMap::Rpm { rpm } => {
                                        self.mcpwm.set_duty(0, 0, 1000u32.Hz(), 1000u32.Hz() / 2);
                                        self.timers.cancel(deactivation_timer);
                                    }
                                    DcMotorMap::Stop {} => {
                                        self.mcpwm.set_duty(0, 0, 1000u32.Hz(), 0);
                                    }
                                }
                            });
                        }
                    });
                });
            }
        }
    }
    fn on_timer(idx : u8) {
        match idx {
            1 => {
                self.mcpwm.set_duty(0, 0, 1000u32.Hz(), 0);
            }
            _ => {}
        }
    }
    fn wants(&self, msg_header : MsgHeader ) -> bool {
        if msg_header.dst.is_some() && msg_header.dst.unwrap()==DC_MOTOR_ID {true }
        else false
    }
}

impl Actor<DcMotorCmd, DcMotorEvent> for DcMotorActor {
    async fn run(&mut self) {
        loop {
            let res = select(self.cmds.next(), self.timers.alarm()).await;
            match res {
                Either::First(cmd) => self.on_cmd(cmd),
                Either::Second(ti) => self.on_timer(ti),
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
