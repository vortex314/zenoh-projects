// use alloc::string::String;//
use embassy_time::Duration;
use esp_hal::mcpwm::McPwm;
use esp_hal::mcpwm::PeripheralClockConfig;
use esp_hal::mcpwm::PwmPeripheral;
// use esp_hal::peripherals::MCPWM0;
// use esp_hal::gpio::Level;
use esp_hal::gpio::GpioPin;
use esp_hal::gpio::Output;
use esp_hal::gpio::Input;
// use esp_hal::gpio::Pin;
// use esp_hal::gpio::any_pin::AnyPin;
use esp_hal::mcpwm::operator::PwmPin;
use esp_hal::mcpwm::operator::PwmPinConfig;
use esp_hal::mcpwm::timer::PwmWorkingMode;
use esp_hal::pcnt::Pcnt;
// use esp_hal::prelude::*; // trait ExtU32
use log::info;

use limero::Actor;
use limero::CmdQueue;
use limero::EventHandlers;
use limero::Timer;
use limero::Timers;

use embassy_futures::select::select;
use embassy_futures::select::Either;

use msg::MsgHeader;

use minicbor::Decode;
use minicbor::Encode;

use esp_hal::prelude::*; // trait ExtU32

use core::cmp::min;
use esp_hal::interrupt::Priority;
use esp_hal::pcnt::channel;
use portable_atomic::AtomicI32;
use portable_atomic::Ordering;

use embassy_sync::blocking_mutex::Mutex;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use core::cell::RefCell;
use esp_hal::pcnt::unit;

static UNIT0: Mutex<NoopRawMutex,RefCell<Option<unit::Unit<'static, 1>>>> = Mutex::new(RefCell::new(None));
static VALUE: AtomicI32 = AtomicI32::new(0);



#[derive(Encode, Decode, Default, Debug, Clone)]
#[cbor(map)]
pub struct DcMotorMsg {
    #[n(0)]
    pub target_rpm: Option<i16>,
    #[n(1)]
    pub measured_rpm: Option<i16>,
    #[n(2)]
    pub current_left: Option<f32>,
    #[n(3)]
    pub current_right: Option<f32>,
    #[n(4)]
    pub pwm_percent: Option<i16>,
}

#[derive(Clone)]
pub enum DcMotorCmd {
    Rpm {
        rpm: i16,
    },
    Request {
        msg_header: MsgHeader,
        msg: DcMotorMsg,
    },
}

pub enum DcMotorEvent {}

const DEACTIVATION_TIMER: u32 = 1;

pub struct DcMotorActor<PWM>
where
    PWM: PwmPeripheral + 'static,
{
    cmds: CmdQueue<DcMotorCmd>,
    clock_cfg: PeripheralClockConfig,
    event_handlers: EventHandlers<DcMotorEvent>,
    timers: Timers,
    //   mcpwm: McPwm<'static, PWM>,
    pwm_a: PwmPin<'static, GpioPin<19>, PWM, 0, true>,
    //   pwm_pin_right: McPwm<'static, MCPWM0>,
    left_enable_pin: Output<'static>,
    right_enable_pin: Output<'static>,


}

impl<PWM> DcMotorActor<PWM>
where
    PWM: PwmPeripheral + 'static,
{
    pub fn new(
        mut mcpwm: McPwm<'static, PWM>,
        clock_cfg: PeripheralClockConfig,
        left_enable_pin: Output<'static>,
        right_enable_pin: Output<'static>,
        pwm_pin_left: GpioPin<19>,
    ) -> DcMotorActor<PWM> {
        /*  let left_current_sense_pin = io.pins.gpio36;
        let right_current_sense_pin = io.pins.gpio34;*/

    //    let heap_size = unsafe { esp_get_free_heap_size() };

         mcpwm.operator0.set_timer(&mcpwm.timer0);
        let mut pwm_a = mcpwm
            .operator0
            .with_pin_a(pwm_pin_left, PwmPinConfig::UP_ACTIVE_HIGH);

        /*mcpwm.operator1.set_timer(&mcpwm.timer0);
        let mut pwm_pin_right = mcpwm
            .operator1
            .with_pin_a(right_pwm_pin, PwmPinConfig::UP_ACTIVE_HIGH);*/

        let timer_clock_cfg = clock_cfg
            .timer_clock_with_frequency(99, PwmWorkingMode::Increase, 20u32.kHz())
            .unwrap();

        mcpwm.timer0.start(timer_clock_cfg);
        pwm_a.set_timestamp(50);
        //     pwm_pin_right.set_timestamp(50);

        pcnt.set_interrupt_handler(interrupt_handler);
        let u0 = pcnt.unit1;
        u0.set_low_limit(Some(-100)).unwrap();
        u0.set_high_limit(Some(100)).unwrap();
        u0.set_filter(Some(min(10u16 * 80, 1023u16))).unwrap();
        u0.clear();

        info!("enabling interrupts");
        u0.listen();
        info!("resume pulse counter unit 0");
        u0.resume();

        let counter = u0.counter.clone();

        critical_section::with(|cs| UNIT0.borrow_ref_mut(cs).replace(u0));


        DcMotorActor {
            cmds: CmdQueue::new(3),
            event_handlers: EventHandlers::new(),
            timers: Timers::new(),
            clock_cfg,
            pwm_a,
            //         right_pwm_pin,
            left_enable_pin,
            right_enable_pin,

        }
    }


    fn on_cmd(&mut self, cmd: DcMotorCmd) {
        self.timers.add_timer(Timer::new_repeater(
            DEACTIVATION_TIMER,
            Duration::from_millis(1000),
        ));
        match cmd {
            DcMotorCmd::Rpm { rpm: _ } => {
                self.pwm_a.set_timestamp(50);
            }

            DcMotorCmd::Request { msg_header: _, msg } => {
                msg.target_rpm.map(|_target_rpm| {
                    self.pwm_a.set_timestamp(50);
                    self.timers.start(DEACTIVATION_TIMER);
                });
                msg.pwm_percent.map(|pwm_percent| {
                    info!("pwm_percent: {}", pwm_percent);
                    self.pwm_a.set_timestamp((pwm_percent % 100i16) as u16);
                    self.timers.start(DEACTIVATION_TIMER);
                });
            }
        }
    }
    fn on_timer(&mut self, idx: u32) {
        match idx {
            DEACTIVATION_TIMER => {
                info!("Deactivation timer pwm=0%");
                self.pwm_a.set_timestamp(0);
            }
            _ => {}
        }
    }
}

impl<PWM> Actor<DcMotorCmd, DcMotorEvent> for DcMotorActor<PWM>
where
    PWM: PwmPeripheral,
{
    async fn run(&mut self) {
        self.left_enable_pin.set_high();
        self.right_enable_pin.set_high();
        loop {
            let res = select(self.cmds.next(), self.timers.alarm()).await;
            match res {
                Either::First(cmd) => self.on_cmd(cmd.unwrap()),
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

#[handler(priority = Priority::Priority2)]
fn interrupt_handler() {
    critical_section::with(|_cs| {
       let mut u0 = UNIT0.borrow_ref_mut(_cs);
        let u0 = u0.as_mut().unwrap();
        if u0.interrupt_is_set() {
            let events = u0.get_events();
            if events.high_limit {
                VALUE.fetch_add(100, Ordering::SeqCst);
            } else if events.low_limit {
                VALUE.fetch_add(-100, Ordering::SeqCst);
            }
            u0.reset_interrupt();
        }
    });
}
