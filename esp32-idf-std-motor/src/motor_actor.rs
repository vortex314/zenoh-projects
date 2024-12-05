use log::{debug, error, info};

use anyhow::Result;
use limero::{timer::Timer, timer::Timers};
use limero::{Actor, CmdQueue, EventHandlers, Handler};
use msg::fnv;
use msg::MsgHeader;

pub const MAC_BROADCAST: [u8; 6] = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF];

#[derive(Encode, Decode, Default, Clone, Debug)]
#[cbor(map)]
struct MotorMsg {
    #[n(0)]
    target_rpm : Option<u32>,
    #[n(1)]
    measured_rpm : Option<u32>,
    #[n(2)]
    target_current : Option<u32>,
    #[n(3)]
    measured_current : Option<u32>,
    #[n(4)]
    rpm_kp : Option<f32>,
    #[n(5)]
    rpm_ki : Option<f32>,
    #[n(6)]
    rpm_kd : Option<f32>,
}/*
    #[n(0)]
    pub id: PropertyId,
    #[n(1)]
    pub name: String,
    #[n(2)]
    pub desc: String,
    #[n(3)]
    pub prop_type: Option<PropType>,
    #[n(4)]
    pub prop_mode: Option<PropMode>, */

struct Info {
    actor_info : ActorInfo { name : "Motor", id : fnv("Motor")}, },
    props_info : Vec<InfoMap>,
}

const  motor_interface = vec![
    InfoMap { id:0 , name :"target_rpm",desc="target desired RPM ", prop_type: PropType::U32, prop_mode: PropMode::ReadWrite },
    InfoMap { id:1 , name :"measured_rpm",desc="measured RPM ", prop_type: PropType::U32, prop_mode: PropMode::ReadOnly },
    InfoMap { id:2 , name :"target_current",desc="target desired current ", prop_type: PropType::U32, prop_mode: PropMode::ReadWrite },
    InfoMap { id:3 , name :"measured_current",desc="measured current ", prop_type: PropType::U32, prop_mode: PropMode::ReadOnly },
    InfoMap { id:4 , name :"rpm_kp",desc="RPM PID Kp ", prop_type: PropType::F32, prop_mode: PropMode::ReadWrite },
    InfoMap { id:5 , name :"rpm_ki",desc="RPM PID Ki ", prop_type: PropType::F32, prop_mode: PropMode::ReadWrite },
    InfoMap { id:6 , name :"rpm_kd",desc="RPM PID Kd ", prop_type: PropType::F32, prop_mode: PropMode::ReadWrite },
    ] ;

    const PWM_HZ :u32 = 10000;
    


#[derive(Clone, Debug)]
pub enum MotorEvent {
    Rxd { peer: [u8; 6], data: Vec<u8> },
}

#[derive(Clone)]
pub enum MotorCmd {
    Msg ( msg : MotorMsg),
    Stop,
}

enum TimerId {
    WatchdogTimer = 1,
}

pub struct MotorActor {
    cmds: CmdQueue<MotorCmd>,
    events: EventHandlers<MotorEvent>,
    timers: Timers,
    sender : async_channel::Sender<MotorEvent>,
    receiver: async_channel::Receiver<MotorEvent>,
    target_rpm : u32,
    measured_rpm : u32,
    target_current : u32,
    measured_current : u32,
    rpm_kp : f32,
    rpm_ki : f32,
    rpm_kd : f32,
}

impl MotorActor {
    fn send_msg(&self) -> MotorMsg {
        MotorMsg {
            target_rpm : self.target_rpm,
            measured_rpm : self.measured_rpm,
            target_current : self.target_current,
            measured_current : self.measured_current,
            rpm_kp : self.rpm_kp,
            rpm_ki : self.rpm_ki,
            rpm_kd : self.rpm_kd,
        }
    }
    fn recv_msg(&mut self, msg : MotorMsg) {
        if let Some(target_rpm) = msg.target_rpm {
            self.target_rpm = target_rpm;
        }
        if let Some(target_current) = msg.target_current {
            self.target_current = target_current;
        }
        if let Some(rpm_kp) = msg.rpm_kp {
            self.rpm_kp = rpm_kp;
        }
        if let Some(rpm_ki) = msg.rpm_ki {
            self.rpm_ki = rpm_ki;
        }
        if let Some(rpm_kd) = msg.rpm_kd {
            self.rpm_kd = rpm_kd;
        }
    }
    fn new() -> Self {
        let (sender, receiver) = async_channel::unbounded();
        MotorActor {
            cmds: CmdQueue::new(),
            events: EventHandlers::new(),
            timers: Timers::new(),
            sender,
            receiver,
            target_rpm : 0,
            measured_rpm : 0,
            target_current : 0,
            measured_current : 0,
            rpm_kp : 0.0,
            rpm_ki : 0.0,
            rpm_kd : 0.0,
        }
    }

    fn get_info(&self,prop:u32) -> Result<&InfoMap> {
        motor_interface.filter( _.id == prop )
    }
}



impl Actor<MotorCmd, MotorEvent> for MotorActor {
}

/*

https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_bdc_speed_control

/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include "pid_ctrl.h"

static const char *TAG = "example";

// Enable this config,  we will print debug formated string, which in return can be captured and parsed by Serial-Studio
#define SERIAL_STUDIO_DEBUG           CONFIG_SERIAL_STUDIO_DEBUG

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ             25000    // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks
#define BDC_MCPWM_GPIO_A              7
#define BDC_MCPWM_GPIO_B              15

#define BDC_ENCODER_GPIO_A            36
#define BDC_ENCODER_GPIO_B            35
#define BDC_ENCODER_PCNT_HIGH_LIMIT   1000
#define BDC_ENCODER_PCNT_LOW_LIMIT    -1000

#define BDC_PID_LOOP_PERIOD_MS        10   // calculate the motor speed every 10ms
#define BDC_PID_EXPECT_SPEED          400  // expected motor speed, in the pulses counted by the rotary encoder

typedef struct {
    bdc_motor_handle_t motor;
    pcnt_unit_handle_t pcnt_encoder;
    pid_ctrl_block_handle_t pid_ctrl;
    int report_pulses;
} motor_control_context_t;

static void pid_loop_cb(void *args)
{
    static int last_pulse_count = 0;
    motor_control_context_t *ctx = (motor_control_context_t *)args;
    pcnt_unit_handle_t pcnt_unit = ctx->pcnt_encoder;
    pid_ctrl_block_handle_t pid_ctrl = ctx->pid_ctrl;
    bdc_motor_handle_t motor = ctx->motor;

    // get the result from rotary encoder
    int cur_pulse_count = 0;
    pcnt_unit_get_count(pcnt_unit, &cur_pulse_count);
    int real_pulses = cur_pulse_count - last_pulse_count;
    last_pulse_count = cur_pulse_count;
    ctx->report_pulses = real_pulses;

    // calculate the speed error
    float error = BDC_PID_EXPECT_SPEED - real_pulses;
    float new_speed = 0;

    // set the new speed
    pid_compute(pid_ctrl, error, &new_speed);
    bdc_motor_set_speed(motor, (uint32_t)new_speed);
}

void app_main(void)
{
    static motor_control_context_t motor_ctrl_ctx = {
        .pcnt_encoder = NULL,
    };

    ESP_LOGI(TAG, "Create DC motor");
    bdc_motor_config_t motor_config = {
        .pwm_freq_hz = BDC_MCPWM_FREQ_HZ,
        .pwma_gpio_num = BDC_MCPWM_GPIO_A,
        .pwmb_gpio_num = BDC_MCPWM_GPIO_B,
    };
    bdc_motor_mcpwm_config_t mcpwm_config = {
        .group_id = 0,
        .resolution_hz = BDC_MCPWM_TIMER_RESOLUTION_HZ,
    };
    bdc_motor_handle_t motor = NULL;
    ESP_ERROR_CHECK(bdc_motor_new_mcpwm_device(&motor_config, &mcpwm_config, &motor));
    motor_ctrl_ctx.motor = motor;

    ESP_LOGI(TAG, "Init pcnt driver to decode rotary signal");
    pcnt_unit_config_t unit_config = {
        .high_limit = BDC_ENCODER_PCNT_HIGH_LIMIT,
        .low_limit = BDC_ENCODER_PCNT_LOW_LIMIT,
        .flags.accum_count = true, // enable counter accumulation
    };
    pcnt_unit_handle_t pcnt_unit = NULL;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = BDC_ENCODER_GPIO_A,
        .level_gpio_num = BDC_ENCODER_GPIO_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = BDC_ENCODER_GPIO_B,
        .level_gpio_num = BDC_ENCODER_GPIO_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, BDC_ENCODER_PCNT_LOW_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
    motor_ctrl_ctx.pcnt_encoder = pcnt_unit;

    ESP_LOGI(TAG, "Create PID control block");
    pid_ctrl_parameter_t pid_runtime_param = {
        .kp = 0.6,
        .ki = 0.4,
        .kd = 0.2,
        .cal_type = PID_CAL_TYPE_INCREMENTAL,
        .max_output   = BDC_MCPWM_DUTY_TICK_MAX - 1,
        .min_output   = 0,
        .max_integral = 1000,
        .min_integral = -1000,
    };
    pid_ctrl_block_handle_t pid_ctrl = NULL;
    pid_ctrl_config_t pid_config = {
        .init_param = pid_runtime_param,
    };
    ESP_ERROR_CHECK(pid_new_control_block(&pid_config, &pid_ctrl));
    motor_ctrl_ctx.pid_ctrl = pid_ctrl;

    ESP_LOGI(TAG, "Create a timer to do PID calculation periodically");
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = pid_loop_cb,
        .arg = &motor_ctrl_ctx,
        .name = "pid_loop"
    };
    esp_timer_handle_t pid_loop_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &pid_loop_timer));

    ESP_LOGI(TAG, "Enable motor");
    ESP_ERROR_CHECK(bdc_motor_enable(motor));
    ESP_LOGI(TAG, "Forward motor");
    ESP_ERROR_CHECK(bdc_motor_forward(motor));

    ESP_LOGI(TAG, "Start motor speed loop");
    ESP_ERROR_CHECK(esp_timer_start_periodic(pid_loop_timer, BDC_PID_LOOP_PERIOD_MS * 1000));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        // the following logging format is according to the requirement of serial-studio frame format
        // also see the dashboard config file `serial-studio-dashboard.json` for more information
#if SERIAL_STUDIO_DEBUG
        printf("/*%d*/\r\n", motor_ctrl_ctx.report_pulses);
#endif
    }
}
*/