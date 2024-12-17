/*

mcpwm and capture actor

- drives a motor DC via PWM at 10kHz
- measures the motor speed via a hall sensor or tacho-meter
- PID control of the motor speed

MotorActor
- recv Msg => SetReq(src,dst), InfoReq(dst) ,PublishBroadcast(src)
- send Msg => SetResp(src,dst), InfoResp(src), PublishBroadcast(src)

pub msg_in : Handler<Msg>
pub msg_out : EventHandlers<Msg>
pub cmd_in : Handler<MotorCmd>

based on
- https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_bldc_hall_control/main/mcpwm_bldc_hall_control_example_main.c
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html
*/

use anyhow::Result;
use async_channel::Sender;
use embassy_futures::select::{select3, Either3};
use embassy_time::{Duration, Instant};
use esp_idf_svc::sys::{
    esp, esp_get_free_heap_size, mcpwm_cap_channel_handle_t, mcpwm_cap_channel_t, mcpwm_cap_timer_handle_t, mcpwm_capture_channel_config_t, mcpwm_capture_channel_config_t__bindgen_ty_1, mcpwm_capture_channel_enable, mcpwm_capture_channel_register_event_callbacks, mcpwm_capture_edge_t_MCPWM_CAP_EDGE_NEG, mcpwm_capture_edge_t_MCPWM_CAP_EDGE_POS, mcpwm_capture_event_callbacks_t, mcpwm_capture_event_cb_t, mcpwm_capture_event_data_t, mcpwm_capture_timer_config_t, mcpwm_capture_timer_enable, mcpwm_capture_timer_start, mcpwm_cmpr_handle_t, mcpwm_comparator_config_t, mcpwm_comparator_config_t__bindgen_ty_1, mcpwm_comparator_set_compare_value, mcpwm_gen_compare_event_action_t, mcpwm_gen_handle_t, mcpwm_gen_timer_event_action_t, mcpwm_generator_action_t_MCPWM_GEN_ACTION_HIGH, mcpwm_generator_action_t_MCPWM_GEN_ACTION_LOW, mcpwm_generator_config_t, mcpwm_generator_config_t__bindgen_ty_1, mcpwm_generator_set_action_on_compare_event, mcpwm_generator_set_action_on_timer_event, mcpwm_new_capture_channel, mcpwm_new_capture_timer, mcpwm_new_comparator, mcpwm_new_generator, mcpwm_new_operator, mcpwm_new_timer, mcpwm_oper_handle_t, mcpwm_operator_config_t, mcpwm_operator_config_t__bindgen_ty_1, mcpwm_operator_connect_timer, mcpwm_timer_config_t, mcpwm_timer_config_t__bindgen_ty_1, mcpwm_timer_count_mode_t_MCPWM_TIMER_COUNT_MODE_UP, mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP, mcpwm_timer_enable, mcpwm_timer_event_t_MCPWM_TIMER_EVENT_EMPTY, mcpwm_timer_handle_t, mcpwm_timer_start_stop, mcpwm_timer_start_stop_cmd_t_MCPWM_TIMER_START_NO_STOP, soc_module_clk_t_SOC_MOD_CLK_APB, soc_module_clk_t_SOC_MOD_CLK_PLL_F160M
};
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use log::{debug, error, info};
use minicbor::{Decode, Encode};
use msg::{fnv, InfoMap, Msg};
use msg::{PropMode, PropType};
use std::ffi::c_void;
use std::time::SystemTime;

const MCPWM_TIMER_CLK_SRC_DEFAULT: u32 = soc_module_clk_t_SOC_MOD_CLK_PLL_F160M;
const BLDC_MCPWM_TIMER_RESOLUTION_HZ: u32 = 10000000; // 10MHz, 1 tick = 0.1us
const TICKS_PER_PERIOD: u32 = 500; // 50us, 20KHz
const GPIO_CAPTURE: i32 = 14; // gpio12
const GPIO_PWM: i32 = 13; // gpio13
pub const MAC_BROADCAST: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

#[derive(Encode, Decode, Default, Clone, Debug)]
#[cbor(map)]
struct MotorMsg {
    #[n(0)]
    target_rpm: Option<u32>,
    #[n(1)]
    measured_rpm: Option<u32>,
    #[n(2)]
    target_current: Option<u32>,
    #[n(3)]
    measured_current: Option<u32>,
    #[n(4)]
    rpm_kp: Option<f32>,
    #[n(5)]
    rpm_ki: Option<f32>,
    #[n(6)]
    rpm_kd: Option<f32>,
}

struct InfoStruct {
    id: u32,
    name: &'static str,
    desc: &'static str,
    prop_type: PropType,
    prop_mode: PropMode,
}

impl InfoStruct {
    fn new(
        id: u32,
        name: &'static str,
        desc: &'static str,
        prop_type: PropType,
        prop_mode: PropMode,
    ) -> Self {
        InfoStruct {
            id,
            name,
            desc,
            prop_type,
            prop_mode,
        }
    }
}

static MOTOR_INTERFACE: &[InfoStruct] = &[
    InfoStruct {
        id: 0,
        name: "target_rpm",
        desc: "target desired RPM ",
        prop_type: PropType::UINT,
        prop_mode: PropMode::ReadWrite,
    },
    InfoStruct {
        id: 1,
        name: "measured_rpm",
        desc: "measured RPM ",
        prop_type: PropType::UINT,
        prop_mode: PropMode::Read,
    },
    InfoStruct {
        id: 2,
        name: "target_current",
        desc: "target current mA",
        prop_type: PropType::UINT,
        prop_mode: PropMode::ReadWrite,
    },
    InfoStruct {
        id: 3,
        name: "measured_current",
        desc: "measured current mA",
        prop_type: PropType::UINT,
        prop_mode: PropMode::Read,
    },
    InfoStruct {
        id: 4,
        name: "rpm_kp",
        desc: "RPM PID Kp ",
        prop_type: PropType::FLOAT,
        prop_mode: PropMode::ReadWrite,
    },
    InfoStruct {
        id: 5,
        name: "rpm_ki",
        desc: "RPM PID Ki ",
        prop_type: PropType::FLOAT,
        prop_mode: PropMode::ReadWrite,
    },
    InfoStruct {
        id: 6,
        name: "rpm_kd",
        desc: "RPM PID Kd ",
        prop_type: PropType::FLOAT,
        prop_mode: PropMode::ReadWrite,
    },
];
#[derive(Clone, Debug)]
pub enum MotorEvent {
    Msg { msg: Msg },
    Isr { sum: u32, count: u32 },
}
#[derive(Clone)]
pub enum MotorCmd {
    Msg { msg: Msg },
}
enum TimerId {
    WatchdogTimer = 1,
    ReportingTimer = 2,
}
pub struct MotorActor {
    // actor info
    cmds: CmdQueue<MotorCmd>,
    eventhandlers: EventHandlers<MotorEvent>,
    timers: Timers,
    sender: async_channel::Sender<MotorEvent>,
    receiver: async_channel::Receiver<MotorEvent>,
    // props
    topic_name: String,
    topic_id: u32,
    prop_counter: u32,
    target_rpm: u32,
    measured_rpm: u32,
    target_current: u32,
    measured_current: u32,
    // PID info
    rpm_kp: f32,
    rpm_ki: f32,
    rpm_kd: f32,
    // Device driver info
    pwm_value: u32, // from 0 to TICKS_PER_PERIOD
    timer_handle: mcpwm_timer_handle_t,
    operator_handle: mcpwm_oper_handle_t,
    comparator_handle: mcpwm_cmpr_handle_t,
    generator_handle: mcpwm_gen_handle_t,
    cap_timer_handle: mcpwm_cap_timer_handle_t,
    cap_channel_handle: mcpwm_cap_channel_handle_t,
}

// static bounded channel for isr
static mut ISR_DATA : Option<IsrData> = None;

struct IsrData {
    sender : Sender<MotorEvent>,
    sum : u32,
    count: u32,
    last_capture : u32,
}

impl IsrData {
    fn new(sender:Sender<MotorEvent>) -> Self {
        IsrData { sender, sum:0, count: 0 ,last_capture:0}
    }
}

    unsafe extern "C" fn capture_callback(
        _cap_channel: *mut mcpwm_cap_channel_t,
        capture_event: *const mcpwm_capture_event_data_t,
        _user_data: *mut c_void,
    ) -> bool {
        let cap_value = (*capture_event).cap_value;
  //      let pos_edge = (*capture_event).cap_edge == mcpwm_capture_edge_t_MCPWM_CAP_EDGE_POS;
        
        
        let _ = ISR_DATA.as_mut().map( | isr_data| 
            {
            let ticks_per_period = if cap_value > isr_data.last_capture { cap_value - isr_data.last_capture } else { (0xFFFFFFFF - isr_data.last_capture) + cap_value };
            isr_data.sum += ticks_per_period;
            isr_data.count += 1;

            if isr_data.sum > 8_000_000 { // 100 msec at 80 MHz
                let _ = isr_data.sender.try_send(MotorEvent::Isr {
                    sum : isr_data.sum,
                    count:isr_data.count,
                });
                isr_data.sum = 0;
                isr_data.count = 0;
            };
            
            isr_data.last_capture = cap_value;
            });
        true
    }



impl MotorActor {
    fn get_prop_values(&self) -> MotorMsg {
        MotorMsg {
            target_rpm: Some(self.target_rpm),
            measured_rpm: Some(self.measured_rpm),
            target_current: Some(self.target_current),
            measured_current: Some(self.measured_current),
            rpm_kp: Some(self.rpm_kp),
            rpm_ki: Some(self.rpm_ki),
            rpm_kd: Some(self.rpm_kd),
        }
    }
    fn set_prop_values(&mut self, msg: &MotorMsg) {
        msg.target_rpm.map(|rpm| self.target_rpm = rpm);
        msg.target_current
            .map(|current| self.target_current = current);
        msg.rpm_kp.map(|kp| self.rpm_kp = kp);
        msg.rpm_ki.map(|ki| self.rpm_ki = ki);
        msg.rpm_kd.map(|kd| self.rpm_kd = kd);
    }
    pub fn new(topic_name: String) -> Self {
        let (sender, receiver) = async_channel::bounded(4);
        unsafe { ISR_DATA = Some(IsrData::new(sender.clone())); };
        MotorActor {
            cmds: CmdQueue::new(4),
            eventhandlers: EventHandlers::new(),
            timers: Timers::new(),
            sender,
            receiver,
            topic_name: topic_name.clone(),
            topic_id: fnv(topic_name.as_str()),
            prop_counter: 0,
            target_rpm: 0,
            measured_rpm: 0,
            target_current: 0,
            measured_current: 0,
            rpm_kp: 1.0001,
            rpm_ki: 0.0001,
            rpm_kd: -0.0001,
            pwm_value: 400, // 50% duty cycle to test
            timer_handle: std::ptr::null_mut(),
            operator_handle: std::ptr::null_mut(),
            comparator_handle: std::ptr::null_mut(),
            generator_handle: std::ptr::null_mut(),
            cap_timer_handle: std::ptr::null_mut(),
            cap_channel_handle: std::ptr::null_mut(),
        }
    }
    fn get_dev_info(&self) -> Result<InfoMap> {
        Ok(InfoMap {
            id: -1,
            name: Some(self.topic_name.clone()),
            desc: Some("MotorActor as Actor".to_string()),
            prop_type: Some(PropType::OBJECT),
            prop_mode: Some(PropMode::Read),
        })
    }
    fn get_prop_info(&self, prop: u32) -> Result<InfoMap> {
        let str = &MOTOR_INTERFACE[prop as usize];
        Ok(InfoMap {
            id: prop as i8,
            name: Some(str.name.to_string()),
            desc: Some(str.desc.to_string()),
            prop_type: Some(str.prop_type),
            prop_mode: Some(str.prop_mode),
        })
    }
    pub fn init(&mut self) -> Result<()> {
        info!("MotorActor init");
        self.init_capture()?;
        info!("MotorActor capture init done");
        self.init_pwm()?;
        info!("MotorActor pwm init done");
        Ok(())
    }
    /*

    | clock |--->| timer |---->| operator |---->| comparator |---->| generator |---->| GPIO |
    - timer counts to 500 , at 10 Mhz, resets at peak
    - comparator generates event when compare value with timer is reached

     */

    fn init_pwm(&mut self) -> Result<()> {
        unsafe {
            // create timer from group 0 at 50us period
            let mut flags = mcpwm_timer_config_t__bindgen_ty_1::default();
            flags.set_update_period_on_empty(1); // permits to update the period of the timer on zero
            flags.set_update_period_on_sync(1); // permits to update the period on sync

            let timer_config: mcpwm_timer_config_t = mcpwm_timer_config_t {
                group_id: 0,
                clk_src: MCPWM_TIMER_CLK_SRC_DEFAULT, // 160MHz
                resolution_hz: BLDC_MCPWM_TIMER_RESOLUTION_HZ, // 10MHz, 1 tick = 0.1us
                count_mode: mcpwm_timer_count_mode_t_MCPWM_TIMER_COUNT_MODE_UP,
                period_ticks: TICKS_PER_PERIOD, // 50us, 20KHz
                flags,                          // when to change the value
                intr_priority: 1,
            };
            esp!(mcpwm_new_timer(&timer_config, &mut self.timer_handle))?;
            esp!(mcpwm_timer_enable(self.timer_handle))?;

            // define operator
            let flags = mcpwm_operator_config_t__bindgen_ty_1::default();
            let operator_config = mcpwm_operator_config_t {
                group_id: 0,
                intr_priority: 1,
                flags,
            };
            esp!(mcpwm_new_operator(
                &operator_config,
                &mut self.operator_handle
            ))?;
            esp!(mcpwm_operator_connect_timer(
                self.operator_handle,
                self.timer_handle
            ))?;

            let mut flags = mcpwm_comparator_config_t__bindgen_ty_1::default();
            flags.set_update_cmp_on_tez(1); // update compare value on timer event zero
            let comparator_config = mcpwm_comparator_config_t {
                flags,
                intr_priority: 1,
            };
            esp!(mcpwm_new_comparator(
                self.operator_handle,
                &comparator_config,
                &mut self.comparator_handle
            ))?;

            let flags = mcpwm_generator_config_t__bindgen_ty_1::default();
            flags.pull_up(); // pull up the GPIO
            let generator_config = mcpwm_generator_config_t {
                gen_gpio_num: GPIO_PWM,
                flags,
            };
            esp!(mcpwm_new_generator(
                self.operator_handle,
                &generator_config,
                &mut self.generator_handle
            ))?;

            esp!(mcpwm_comparator_set_compare_value(
                self.comparator_handle,
                self.pwm_value
            ))?;
            // at start set GPIO high
            esp!(mcpwm_generator_set_action_on_timer_event(
                self.generator_handle,
                mcpwm_gen_timer_event_action_t {
                    direction: mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP,
                    event: mcpwm_timer_event_t_MCPWM_TIMER_EVENT_EMPTY,
                    action: mcpwm_generator_action_t_MCPWM_GEN_ACTION_HIGH
                }
            ))?;
            // go low on GPIO on compare threshold
            esp!(mcpwm_generator_set_action_on_compare_event(
                self.generator_handle,
                mcpwm_gen_compare_event_action_t {
                    direction: mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP,
                    comparator: self.comparator_handle,
                    action: mcpwm_generator_action_t_MCPWM_GEN_ACTION_LOW
                }
            ))?;

            //   esp!(mcpwm_timer_enable(self.timer_handle))?;
            esp!(mcpwm_timer_start_stop(
                self.timer_handle,
                mcpwm_timer_start_stop_cmd_t_MCPWM_TIMER_START_NO_STOP
            ))?;
        }
        Ok(())
    }
    fn pid_update(&mut self,delta_t :f32) -> Result<f32> {
        let error = (self.target_rpm - self.measured_rpm) as f32;
        let p = self.rpm_kp * error;
        let i = self.rpm_ki * error * delta_t;
        let d = self.rpm_kd * error / delta_t;
        Ok(p + i + d)
    }
    fn pwm_stop(&mut self) -> Result<()> {
        self.target_rpm = 0;
        self.pwm_value = 0;
        Ok(())
    }
    fn init_capture(&mut self) -> Result<()> {
        unsafe {
            // initialize timer
            let mut cap_timer_config = mcpwm_capture_timer_config_t::default();
            cap_timer_config.group_id = 0;
            cap_timer_config.clk_src = soc_module_clk_t_SOC_MOD_CLK_APB;
            esp!(mcpwm_new_capture_timer(
                &cap_timer_config,
                &mut self.cap_timer_handle
            ))?;

            // initialize capture channel
            let mut flags = mcpwm_capture_channel_config_t__bindgen_ty_1::default();
            flags.set_pos_edge(1); // capture on positive edge
 //           flags.set_neg_edge(1); // capture on negative edge
            flags.set_pull_down(1); // pull up the GPIO
            let cap_channel_config = mcpwm_capture_channel_config_t {
                prescale: 1,
                flags,
                gpio_num: GPIO_CAPTURE,
                intr_priority: 1,
            };

            esp!(mcpwm_new_capture_channel(
                self.cap_timer_handle,
                &cap_channel_config,
                &mut self.cap_channel_handle
            ))?;
            // set capture callback
            let user_data = Box::into_raw(Box::new(self.sender.clone())) as *mut c_void;

            // TaskHandle_t task_to_notify = xTaskGetCurrentTaskHandle();
            let cbs = mcpwm_capture_event_callbacks_t {
                on_cap: Some(capture_callback),
            };

            esp!(mcpwm_capture_channel_register_event_callbacks(
                self.cap_channel_handle,
                &cbs,
                user_data as *mut c_void
            ))?;
            // activate capture channel
            esp!(mcpwm_capture_channel_enable(self.cap_channel_handle))?;
            esp!(mcpwm_capture_timer_enable(self.cap_timer_handle))?;
            esp!(mcpwm_capture_timer_start(self.cap_timer_handle))?;
        }
        Ok(())
    }
    fn on_cmd(&mut self, cmd: MotorCmd) {
        match cmd {
            MotorCmd::Msg { msg } => {
                if msg.dst.is_some_and(|dst| dst == self.topic_id) {
                    msg.pub_req.map(|pub_vec| {
                        minicbor::decode::<MotorMsg>(&pub_vec)
                            .ok()
                            .map(|motor_msg| self.set_prop_values(&motor_msg))
                    });
                };
            }
        }
    }
    fn on_timer(&mut self, id: u32) -> Result<()> {
        let timer = unsafe { ::std::mem::transmute::<u8, TimerId>(u8::try_from(id).unwrap() ) };
        match timer {
            TimerId::WatchdogTimer  => {
                let data = minicbor::to_vec(&self.get_prop_values())?;
                let pub_msg = Msg {
                    src: Some(self.topic_id),
                    dst: None,
                    pub_req: Some(data),
                    ..Default::default()
                };
                self.eventhandlers.handle(&MotorEvent::Msg { msg: pub_msg });

                if self.prop_counter >= MOTOR_INTERFACE.len() as u32 {
                    self.prop_counter = 0;
                    let dev_info_msg = Msg {
                        src: Some(self.topic_id),
                        dst: None,
                        info_reply: Some(self.get_dev_info()?),
                        ..Default::default()
                    };
                    self.eventhandlers
                        .handle(&MotorEvent::Msg { msg: dev_info_msg });
                } else {
                    let prop_info_msg = Msg {
                        src: Some(self.topic_id),
                        dst: None,
                        info_reply: Some(self.get_prop_info(self.prop_counter)?),
                        ..Default::default()
                    };
                    self.prop_counter += 1u32;
                    self.eventhandlers
                        .handle(&MotorEvent::Msg { msg: prop_info_msg });
                }
                Ok(())
            }
            TimerId::ReportingTimer  => {
                info!("Reporting timer {} heap free ",unsafe { esp_get_free_heap_size()});
                Ok(())
            }
            _ => Err(anyhow::anyhow!("Unknown timer id: {}", id)),
        }
    }
    fn on_event(&mut self, event: MotorEvent) {
        match event {
            MotorEvent::Isr { sum , count } => {
                let avg = sum / count;
                let hz = 80_000_000 / avg;
                let rpm = (hz * 60) / 4;
                info!("Isr sum: {} count: {} freq : {} Hz. RPM = {} ", sum , count, hz, rpm);   
                let period = sum / count;
                let delta_t :f32 = period as f32 / 1000.0;
                let rpm = 60000000.0 / (period as f32 * delta_t);
                self.measured_rpm = rpm as u32;
                let pid = self.pid_update(sum as f32 ).unwrap();
                self.pwm_value = (self.pwm_value as f32 + pid) as u32;
                if self.pwm_value > TICKS_PER_PERIOD {
                    self.pwm_value = TICKS_PER_PERIOD;
                }
                /*unsafe { esp!(mcpwm_comparator_set_compare_value(
                    self.comparator_handle,
                    self.pwm_value
                ))
                .unwrap();};*/

            }
            _ => {
                error!("Unknown event: {:?}", event);
            }
        }
    }
}

impl Actor<MotorCmd, MotorEvent> for MotorActor {
    async fn run(&mut self) {
        self.timers.add_timer(Timer::new_repeater(
            TimerId::WatchdogTimer as u32,
            Duration::from_millis(1000),
        ));
        self.timers.add_timer(Timer::new_repeater(
            TimerId::ReportingTimer as u32,
            Duration::from_millis(3000),
        ));
        loop {
            match select3(self.cmds.next(), self.timers.alarm(), self.receiver.recv()).await {
                Either3::First(cmd) => {
                    cmd.map(|cmd| self.on_cmd(cmd));
                }
                Either3::Second(id) => {
                    let _ = self.on_timer(id).map_err(|err| error!("Error: {:?}", err));
                }
                Either3::Third(event) => {
                    let _ = event
                        .map(|event| self.on_event(event))
                        .map_err(|err| error!("Error: {:?}", err));
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Box<dyn Handler<MotorEvent>>) {
        self.eventhandlers.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<MotorCmd>> {
        self.cmds.handler()
    }
}
