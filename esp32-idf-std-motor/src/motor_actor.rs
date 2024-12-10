/*

mcpwm and capture actor
based on
- https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_bldc_hall_control/main/mcpwm_bldc_hall_control_example_main.c
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html
*/

use std::ffi::c_void;

use async_channel::Sender;
use embassy_futures::select::{select3, Either3};
use embassy_time::Duration;
use log::{debug, error, info};

use anyhow::Result;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use minicbor::{Decode, Encode};
use msg::{InfoMap, Msg};
use msg::{PropMode, PropType};

use esp_idf_svc::sys::{
    esp, mcpwm_cap_channel_handle_t, mcpwm_cap_channel_t, mcpwm_cap_timer_handle_t, mcpwm_capture_channel_config_t, mcpwm_capture_channel_config_t__bindgen_ty_1, mcpwm_capture_channel_enable, mcpwm_capture_channel_register_event_callbacks, mcpwm_capture_event_callbacks_t, mcpwm_capture_event_cb_t, mcpwm_capture_event_data_t, mcpwm_capture_timer_config_t, mcpwm_capture_timer_enable, mcpwm_capture_timer_start, mcpwm_cmpr_handle_t, mcpwm_comparator_config_t, mcpwm_comparator_config_t__bindgen_ty_1, mcpwm_comparator_set_compare_value, mcpwm_gen_compare_event_action_t, mcpwm_gen_handle_t, mcpwm_gen_timer_event_action_t, mcpwm_generator_action_t_MCPWM_GEN_ACTION_HIGH, mcpwm_generator_action_t_MCPWM_GEN_ACTION_LOW, mcpwm_generator_config_t, mcpwm_generator_config_t__bindgen_ty_1, mcpwm_generator_set_action_on_compare_event, mcpwm_generator_set_action_on_timer_event, mcpwm_new_capture_channel, mcpwm_new_capture_timer, mcpwm_new_comparator, mcpwm_new_generator, mcpwm_new_operator, mcpwm_new_timer, mcpwm_operator_config_t, mcpwm_operator_config_t__bindgen_ty_1, mcpwm_operator_connect_timer, mcpwm_timer_config_t, mcpwm_timer_config_t__bindgen_ty_1, mcpwm_timer_count_mode_t_MCPWM_TIMER_COUNT_MODE_UP, mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP, mcpwm_timer_enable, mcpwm_timer_event_t_MCPWM_TIMER_EVENT_EMPTY, mcpwm_timer_handle_t, mcpwm_timer_start_stop, mcpwm_timer_start_stop_cmd_t_MCPWM_TIMER_START_NO_STOP, soc_module_clk_t_SOC_MOD_CLK_APB, soc_module_clk_t_SOC_MOD_CLK_PLL_F160M
};

const MCPWM_TIMER_CLK_SRC_DEFAULT: u32 = soc_module_clk_t_SOC_MOD_CLK_PLL_F160M;
const BLDC_MCPWM_TIMER_RESOLUTION_HZ: u32 = 10000000; // 10MHz, 1 tick = 0.1us
const BLDC_MCPWM_PERIOD: u32 = 500; // 50us, 20KHz
const GPIO_CAPTURE: i32 = 4; // gpio4
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
    Rxd { peer: [u8; 6], data: Vec<u8> },
    Msg { msg: Msg },
    Isr { cap_value: u32, pos_edge: bool },
}
#[derive(Clone)]
pub enum MotorCmd {
    Msg { msg: Vec<u8> },
    Stop,
}
enum TimerId {
    WatchdogTimer = 1,
}
pub struct MotorActor {
    cmds: CmdQueue<MotorCmd>,
    events: EventHandlers<MotorEvent>,
    timers: Timers,
    sender: async_channel::Sender<MotorEvent>,
    receiver: async_channel::Receiver<MotorEvent>,
    target_rpm: u32,
    measured_rpm: u32,
    target_current: u32,
    measured_current: u32,
    last_measured_time: u64,
    prev_measured_time: u64,
    rpm_kp: f32,
    rpm_ki: f32,
    rpm_kd: f32,
}

unsafe extern "C" fn capture_callback(
    cap_channel: *mut mcpwm_cap_channel_t,
    capture_event: *const mcpwm_capture_event_data_t,
    user_data: *mut c_void,
) -> bool {
    let sender = Box::from_raw(user_data as *mut Sender<MotorEvent>);
    let _ = sender.try_send(MotorEvent::Isr {
        cap_value: (*capture_event).cap_value,
        pos_edge: (*capture_event).cap_edge > 1,
    });
    true
}

impl MotorActor {
    fn send_msg(&self) -> MotorMsg {
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
    fn recv_msg(&mut self, msg: &MotorMsg) {
        msg.target_rpm.map(|rpm| self.target_rpm = rpm);
        msg.target_current
            .map(|current| self.target_current = current);
        msg.rpm_kp.map(|kp| self.rpm_kp = kp);
        msg.rpm_ki.map(|ki| self.rpm_ki = ki);
        msg.rpm_kd.map(|kd| self.rpm_kd = kd);
    }
    pub fn new() -> Self {
        let (sender, receiver) = async_channel::bounded(5);
        MotorActor {
            cmds: CmdQueue::new(5),
            events: EventHandlers::new(),
            timers: Timers::new(),
            sender,
            receiver,
            target_rpm: 0,
            measured_rpm: 0,
            target_current: 0,
            measured_current: 0,
            last_measured_time: 0,
            prev_measured_time: 0,
            rpm_kp: 0.0,
            rpm_ki: 0.0,
            rpm_kd: 0.0,
        }
    }
    fn get_info(&self, prop: u32) -> Result<InfoMap> {
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
    fn init_pwm(&mut self) -> Result<()> {
        unsafe {
            let mut timer_handle: mcpwm_timer_handle_t = std::ptr::null_mut();
            let mut flags = mcpwm_timer_config_t__bindgen_ty_1::default();
            flags.set_update_period_on_empty(1);
            flags.set_update_period_on_sync(1);

            let timer_config: mcpwm_timer_config_t = mcpwm_timer_config_t {
                group_id: 0,
                clk_src: MCPWM_TIMER_CLK_SRC_DEFAULT,
                resolution_hz: BLDC_MCPWM_TIMER_RESOLUTION_HZ,
                count_mode: mcpwm_timer_count_mode_t_MCPWM_TIMER_COUNT_MODE_UP,
                period_ticks: BLDC_MCPWM_PERIOD,
                flags,
                intr_priority: 1,
            };
            esp!(mcpwm_new_timer(&timer_config, &mut timer_handle))?;
            esp!(mcpwm_timer_enable(timer_handle))?;

            let flags = mcpwm_operator_config_t__bindgen_ty_1::default();

            let mut oper = std::ptr::null_mut();
            let operator_config = mcpwm_operator_config_t { group_id: 0 , intr_priority: 1, flags };
            esp!(mcpwm_new_operator(&operator_config, &mut oper))?;
            esp!(mcpwm_operator_connect_timer(oper, timer_handle))?;

            let flags = mcpwm_comparator_config_t__bindgen_ty_1::default();

            let mut comparator: mcpwm_cmpr_handle_t = std::ptr::null_mut();
            let comparator_config = mcpwm_comparator_config_t {
                flags,
                intr_priority: 1,
            };

            esp!(mcpwm_new_comparator(oper, &comparator_config, &mut comparator))?;
            let flags = mcpwm_generator_config_t__bindgen_ty_1::default();

            let generator: mcpwm_gen_handle_t = std::ptr::null_mut();
            let generator_config = mcpwm_generator_config_t {
                gen_gpio_num: GPIO_CAPTURE,
                flags,
            };
            esp!(mcpwm_new_generator(oper, &generator_config, &mut generator))?;

            let mut cap_channel: mcpwm_cap_channel_handle_t = std::ptr::null_mut();
            esp!(mcpwm_comparator_set_compare_value(
                comparator,
                example_angle_to_compare(0)
            ))?;
            esp!(mcpwm_generator_set_action_on_timer_event(
                generator,
                mcpwm_gen_timer_event_action_t {
                    direction: mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP,
                    event: mcpwm_timer_event_t_MCPWM_TIMER_EVENT_EMPTY,
                    action: mcpwm_generator_action_t_MCPWM_GEN_ACTION_HIGH
                }
            ))?;
            // go low on compare threshold
            esp!(mcpwm_generator_set_action_on_compare_event(
                generator,
                mcpwm_gen_compare_event_action_t {
                    direction: mcpwm_timer_direction_t_MCPWM_TIMER_DIRECTION_UP,
                    comparator,
                    action: mcpwm_generator_action_t_MCPWM_GEN_ACTION_LOW
                }
            ))?;

            esp!(mcpwm_timer_enable(timer_handle))?;
            esp!(mcpwm_timer_start_stop(
                timer_handle,
                mcpwm_timer_start_stop_cmd_t_MCPWM_TIMER_START_NO_STOP
            ))?;
        }
        Ok(())
    }
    fn pid_update(&mut self) -> Result<f32> {
        let error = (self.target_rpm - self.measured_rpm) as f32;
        let delta_t = (self.last_measured_time - self.prev_measured_time) as f32;
        let p = self.rpm_kp * error;
        let i = self.rpm_ki * error * delta_t;
        let d = self.rpm_kd * error / delta_t;
        Ok(p + i + d)
    }
    fn pwm_stop(&mut self) -> Result<()> {
        self.target_rpm = 0;
        Ok(())
    }
    fn init_capture(&mut self) -> Result<()> {
        unsafe {
            let mut cap_timer_handle: mcpwm_cap_timer_handle_t = std::ptr::null_mut(); // the handle is a pointer to mcpwm_cap_timer_t
                                                                                       // initialize timer
            let mut cap_timer_config = mcpwm_capture_timer_config_t::default();
            cap_timer_config.group_id = 0;
            cap_timer_config.clk_src = soc_module_clk_t_SOC_MOD_CLK_APB;
            esp!(mcpwm_new_capture_timer(
                &cap_timer_config,
                &mut cap_timer_handle
            ))?;
            // initialize capture channel
            let flags = mcpwm_capture_channel_config_t__bindgen_ty_1::default();
            flags.pos_edge(); // capture on positive edge
            flags.neg_edge(); // capture on negative edge
            let mut cap_channel: mcpwm_cap_channel_handle_t = std::ptr::null_mut(); // the handle is a pointer to mcpwm_cap_channel_t
            let mut cap_channel_config = mcpwm_capture_channel_config_t {
                prescale: 1,
                flags,
                gpio_num: 0,
                intr_priority: 1,
            };
            let cap_chan_gpio: i32 = GPIO_CAPTURE;
            cap_channel_config.gpio_num = cap_chan_gpio;
            esp!(mcpwm_new_capture_channel(
                cap_timer_handle,
                &cap_channel_config,
                &mut cap_channel
            ))?;
            // set capture callback
            let user_data = Box::into_raw(Box::new(self.sender.clone())) as *mut c_void;

            // TaskHandle_t task_to_notify = xTaskGetCurrentTaskHandle();
            let cbs = mcpwm_capture_event_callbacks_t {
                on_cap: Some(capture_callback),
            };

            esp!(mcpwm_capture_channel_register_event_callbacks(
                cap_channel,
                &cbs,
                user_data as *mut c_void
            ))?;
            // activate capture channel
            esp!(mcpwm_capture_channel_enable(cap_channel))?;
            esp!(mcpwm_capture_timer_enable(cap_timer_handle))?;
            esp!(mcpwm_capture_timer_start(cap_timer_handle))?;
        }
        Ok(())
    }
    fn on_cmd(&mut self, cmd: MotorCmd) {
        match cmd {
            MotorCmd::Msg { msg } => {
                let decoded_msg = minicbor::decode::<MotorMsg>(&msg).unwrap();
                self.recv_msg(&decoded_msg);
            }
            MotorCmd::Stop => {
                self.timers.remove_timer(TimerId::WatchdogTimer as u32);
            }
        }
    }
    fn on_timer(&mut self, id: u32) {
        match id {
            id if id == TimerId::WatchdogTimer as u32 => {
                self.timers.add_timer(Timer::new_repeater(
                    TimerId::WatchdogTimer as u32,
                    Duration::from_secs(1),
                ));
            }
            _ => {
                error!("Unknown timer id: {}", id);
            }
        }
    }
    fn on_event(&mut self, event: MotorEvent) {
        match event {
            MotorEvent::Rxd { peer, data } => {
                let msg = minicbor::decode::<MotorMsg>(&data).unwrap();
                self.recv_msg(&msg);
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
            Duration::from_secs(1),
        ));
        loop {
            match select3(self.cmds.next(), self.timers.alarm(), self.receiver.recv()).await {
                Either3::First(cmd) => {
                    self.on_cmd(cmd.unwrap());
                }
                Either3::Second(id) => {
                    self.on_timer(id);
                }
                Either3::Third(event) => {
                    self.on_event(event.unwrap());
                }
            }
        }
    }

    fn add_listener(&mut self, listener: Box<dyn Handler<MotorEvent>>) {
        self.events.add_listener(listener);
    }

    fn handler(&self) -> Box<dyn Handler<MotorCmd>> {
        self.cmds.handler()
    }
}

/*


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"

#define BLDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BLDC_MCPWM_PERIOD              500      // 50us, 20KHz
#define BLDC_SPIN_DIRECTION_CCW        false    // define the spin direction
#define BLDC_SPEED_UPDATE_PERIOD_US    200000   // 200ms

#define BLDC_DRV_EN_GPIO          46
#define BLDC_DRV_FAULT_GPIO       10
#define BLDC_PWM_UH_GPIO          47
#define BLDC_PWM_UL_GPIO          21
#define BLDC_PWM_VH_GPIO          14
#define BLDC_PWM_VL_GPIO          13
#define BLDC_PWM_WH_GPIO          12
#define BLDC_PWM_WL_GPIO          11
#define HALL_CAP_U_GPIO           4
#define HALL_CAP_V_GPIO           5
#define HALL_CAP_W_GPIO           6

#define BLDC_MCPWM_OP_INDEX_U     0
#define BLDC_MCPWM_OP_INDEX_V     1
#define BLDC_MCPWM_OP_INDEX_W     2
#define BLDC_MCPWM_GEN_INDEX_HIGH 0
#define BLDC_MCPWM_GEN_INDEX_LOW  1

static const char *TAG = "example";

typedef void (*bldc_hall_phase_action_t)(mcpwm_gen_handle_t (*gens)[2]);

static inline uint32_t bldc_get_hall_sensor_value(bool ccw)
{
    uint32_t hall_val = gpio_get_level(HALL_CAP_U_GPIO) * 4 + gpio_get_level(HALL_CAP_V_GPIO) * 2 + gpio_get_level(HALL_CAP_W_GPIO) * 1;
    return ccw ? hall_val ^ (0x07) : hall_val;
}

static bool IRAM_ATTR bldc_hall_updated(mcpwm_cap_channel_handle_t cap_channel, const mcpwm_capture_event_data_t *edata, void *user_data)
{
    TaskHandle_t task_to_notify = (TaskHandle_t)user_data;
    BaseType_t high_task_wakeup = pdFALSE;
    vTaskNotifyGiveFromISR(task_to_notify, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

// U+V-
static void bldc_set_phase_up_vm(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = PWM, U- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);

    // V+ = 0, V- = 1  --[because gen_low is inverted by dead time]--> V+ = 0, V- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);

    // W+ = 0, W- = 0  --[because gen_low is inverted by dead time]--> W+ = 0, W- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);
}

// W+U-
static void bldc_set_phase_wp_um(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = 0, U- = 1  --[because gen_low is inverted by dead time]--> U+ = 0, U- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);

    // V+ = 0, V- = 0  --[because gen_low is inverted by dead time]--> V+ = 0, V- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);

    // W+ = PWM, W- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);
}

// W+V-
static void bldc_set_phase_wp_vm(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = 0, U- = 0  --[because gen_low is inverted by dead time]--> U+ = 0, U- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);

    // V+ = 0, V- = 1  --[because gen_low is inverted by dead time]--> V+ = 0, V- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);

    // W+ = PWM, W- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);
}

// V+U-
static void bldc_set_phase_vp_um(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = 0, U- = 1  --[because gen_low is inverted by dead time]--> U+ = 0, U- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);

    // V+ = PWM, V- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);

    // W+ = 0, W- = 0  --[because gen_low is inverted by dead time]--> W+ = 0, W- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);
}

// V+W-
static void bldc_set_phase_vp_wm(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = 0, U- = 0  --[because gen_low is inverted by dead time]--> U+ = 0, U- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);

    // V+ = PWM, V- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);

    // W+ = 0, W- = 1  --[because gen_low is inverted by dead time]--> W+ = 0, W- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);
}

// U+W- / A+C-
static void bldc_set_phase_up_wm(mcpwm_gen_handle_t (*gens)[2])
{
    // U+ = PWM, U- = _PWM_
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_HIGH], -1, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_U][BLDC_MCPWM_GEN_INDEX_LOW], -1, true);

    // V+ = 0, V- = 0  --[because gen_low is inverted by dead time]--> V+ = 0, V- = 1
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_V][BLDC_MCPWM_GEN_INDEX_LOW], 1, true);

    // W+ = 0, W- = 1  --[because gen_low is inverted by dead time]--> W+ = 0, W- = 0
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true);
    mcpwm_generator_set_force_level(gens[BLDC_MCPWM_OP_INDEX_W][BLDC_MCPWM_GEN_INDEX_LOW], 0, true);
}

static const bldc_hall_phase_action_t s_hall_actions[] = {
    [2] = bldc_set_phase_up_vm,
    [6] = bldc_set_phase_wp_vm,
    [4] = bldc_set_phase_wp_um,
    [5] = bldc_set_phase_vp_um,
    [1] = bldc_set_phase_vp_wm,
    [3] = bldc_set_phase_up_wm,
};

static void update_motor_speed_callback(void *arg)
{
    static int step = 20;
    static int cur_speed = 0;
    if ((cur_speed + step) > 300 || (cur_speed + step) < 0) {
        step *= -1;
    }
    cur_speed += step;

    mcpwm_cmpr_handle_t *cmprs = (mcpwm_cmpr_handle_t *)arg;
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmprs[i], cur_speed));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Disable MOSFET gate");
    gpio_config_t drv_en_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << BLDC_DRV_EN_GPIO,
    };
    ESP_ERROR_CHECK(gpio_config(&drv_en_config));
    gpio_set_level(BLDC_DRV_EN_GPIO, 0);

    ESP_LOGI(TAG, "Create MCPWM timer");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = BLDC_MCPWM_TIMER_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = BLDC_MCPWM_PERIOD,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    ESP_LOGI(TAG, "Create MCPWM operator");
    mcpwm_oper_handle_t operators[3];
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operators[i]));
    }

    ESP_LOGI(TAG, "Connect operators to the same timer");
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operators[i], timer));
    }

    ESP_LOGI(TAG, "Create comparators");
    mcpwm_cmpr_handle_t comparators[3];
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_new_comparator(operators[i], &compare_config, &comparators[i]));
        // set compare value to 0, we will adjust the speed in a period timer callback
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparators[i], 0));
    }

    ESP_LOGI(TAG, "Create over current fault detector");
    mcpwm_fault_handle_t over_cur_fault = NULL;
    mcpwm_gpio_fault_config_t gpio_fault_config = {
        .gpio_num = BLDC_DRV_FAULT_GPIO,
        .group_id = 0,
        .flags.active_level = 0, // low level means fault, refer to DRV8302 datasheet
        .flags.pull_up = true,   // internally pull up
    };
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&gpio_fault_config, &over_cur_fault));

    ESP_LOGI(TAG, "Set brake mode on the fault event");
    mcpwm_brake_config_t brake_config = {
        .brake_mode = MCPWM_OPER_BRAKE_MODE_CBC,
        .fault = over_cur_fault,
        .flags.cbc_recover_on_tez = true,
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(operators[i], &brake_config));
    }

    ESP_LOGI(TAG, "Create PWM generators");
    mcpwm_gen_handle_t generators[3][2] = {};
    mcpwm_generator_config_t gen_config = {};
    const int gen_gpios[3][2] = {
        {BLDC_PWM_UH_GPIO, BLDC_PWM_UL_GPIO},
        {BLDC_PWM_VH_GPIO, BLDC_PWM_VL_GPIO},
        {BLDC_PWM_WH_GPIO, BLDC_PWM_WL_GPIO},
    };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            gen_config.gen_gpio_num = gen_gpios[i][j];
            ESP_ERROR_CHECK(mcpwm_new_generator(operators[i], &gen_config, &generators[i][j]));
        }
    }

    ESP_LOGI(TAG, "Set generator actions");
    // gen_high and gen_low output the same waveform after the following configuration
    // we will use the dead time module to add edge delay, also make gen_high and gen_low complementary
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH],
                                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH],
                                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparators[i], MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH],
                                                                  MCPWM_GEN_BRAKE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_OPER_BRAKE_MODE_CBC, MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH],
                                                                  MCPWM_GEN_BRAKE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_OPER_BRAKE_MODE_CBC, MCPWM_GEN_ACTION_LOW)));

        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generators[i][BLDC_MCPWM_GEN_INDEX_LOW],
                                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generators[i][BLDC_MCPWM_GEN_INDEX_LOW],
                                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparators[i], MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(generators[i][BLDC_MCPWM_GEN_INDEX_LOW],
                                                                  MCPWM_GEN_BRAKE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_OPER_BRAKE_MODE_CBC, MCPWM_GEN_ACTION_LOW)));
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(generators[i][BLDC_MCPWM_GEN_INDEX_LOW],
                                                                  MCPWM_GEN_BRAKE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_OPER_BRAKE_MODE_CBC, MCPWM_GEN_ACTION_LOW)));
    }

    ESP_LOGI(TAG, "Setup deadtime");
    mcpwm_dead_time_config_t dt_config = {
        .posedge_delay_ticks = 5,
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH], generators[i][BLDC_MCPWM_GEN_INDEX_HIGH], &dt_config));
    }
    dt_config = (mcpwm_dead_time_config_t) {
        .negedge_delay_ticks = 5,
        .flags.invert_output = true,
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(generators[i][BLDC_MCPWM_GEN_INDEX_LOW], generators[i][BLDC_MCPWM_GEN_INDEX_LOW], &dt_config));
    }

    ESP_LOGI(TAG, "Turn off all the gates");
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(generators[i][BLDC_MCPWM_GEN_INDEX_HIGH], 0, true));
        // because gen_low is inverted by dead time module, so we need to set force level to 1
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(generators[i][BLDC_MCPWM_GEN_INDEX_LOW], 1, true));
    }

    ESP_LOGI(TAG, "Create Hall sensor capture channels");
    mcpwm_cap_timer_handle_t cap_timer = NULL;
    mcpwm_capture_timer_config_t cap_timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_timer(&cap_timer_config, &cap_timer));
    mcpwm_cap_channel_handle_t cap_channels[3];
    mcpwm_capture_channel_config_t cap_channel_config = {
        .prescale = 1,
        .flags.pull_up = true,
        .flags.neg_edge = true,
        .flags.pos_edge = true,
    };
    const int cap_chan_gpios[3] = {HALL_CAP_U_GPIO, HALL_CAP_V_GPIO, HALL_CAP_W_GPIO};
    for (int i = 0; i < 3; i++) {
        cap_channel_config.gpio_num = cap_chan_gpios[i];
        ESP_ERROR_CHECK(mcpwm_new_capture_channel(cap_timer, &cap_channel_config, &cap_channels[i]));
    }

    ESP_LOGI(TAG, "Register event callback for capture channels");
    TaskHandle_t task_to_notify = xTaskGetCurrentTaskHandle();
    for (int i = 0; i < 3; i++) {
        mcpwm_capture_event_callbacks_t cbs = {
            .on_cap = bldc_hall_updated,
        };
        ESP_ERROR_CHECK(mcpwm_capture_channel_register_event_callbacks(cap_channels[i], &cbs, task_to_notify));
    }

    ESP_LOGI(TAG, "Enable capture channels");
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(mcpwm_capture_channel_enable(cap_channels[i]));
    }

    ESP_LOGI(TAG, "Enable and start capture timer");
    ESP_ERROR_CHECK(mcpwm_capture_timer_enable(cap_timer));
    ESP_ERROR_CHECK(mcpwm_capture_timer_start(cap_timer));

    ESP_LOGI(TAG, "Start a timer to adjust motor speed periodically");
    esp_timer_handle_t periodic_timer = NULL;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = update_motor_speed_callback,
        .arg = comparators,
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, BLDC_SPEED_UPDATE_PERIOD_US));

    ESP_LOGI(TAG, "Enable MOSFET gate");
    gpio_set_level(BLDC_DRV_EN_GPIO, 1);

    ESP_LOGI(TAG, "Start the MCPWM timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    uint32_t hall_sensor_value = 0;
    while (1) {
        // The rotation direction is controlled by inverting the hall sensor value
        hall_sensor_value = bldc_get_hall_sensor_value(BLDC_SPIN_DIRECTION_CCW);
        if (hall_sensor_value >= 1 && hall_sensor_value <= 6) {
            s_hall_actions[hall_sensor_value](generators);
        } else {
            ESP_LOGE(TAG, "invalid bldc phase, wrong hall sensor value:%"PRIu32, hall_sensor_value);
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
    */
