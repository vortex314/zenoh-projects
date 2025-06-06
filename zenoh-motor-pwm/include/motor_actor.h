#ifndef MOTOR_ACTOR_H
#define MOTOR_ACTOR_H

#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include "cbor.h"
#include <util.h>
#include <vector>
#include "driver/mcpwm_prelude.h"
#include "driver/mcpwm_types.h"
//#include "driver/mcpwm.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_cap.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_fault.h"
#include <driver/gpio.h>

struct MotorMsg : public Serializable
{
    Option<uint32_t> rpm_target = nullptr;
    Option<uint32_t> rpm_measured = nullptr;
    Option<float> pwm = nullptr;
    Option<float> Kp = nullptr;
    Option<float> Ki = nullptr;
    Option<float> Kd = nullptr;
    Option<float> p = nullptr;
    Option<float> i = nullptr;
    Option<float> d = nullptr;

    Res serialize(Serializer &ser) const;
    Res deserialize(Deserializer &des);
};

struct MotorEvent
{
    Option<MotorMsg> publish = nullptr;
};

struct IsrMsg {
    uint32_t sum = 0;
    uint32_t count = 0;
};

struct MotorCmd
{
    //    Option<PublishSerdes> serdes = nullptr;
    Option<MotorMsg> publish = nullptr;
    Option<IsrMsg> isr_msg = nullptr;
};

class MotorActor : public Actor<MotorEvent, MotorCmd>
{
public:
    MotorActor();
    MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~MotorActor();
    void on_cmd(MotorCmd &cmd);
    void on_timer(int timer_id);
    void on_start(void);
    Res pwm_init();
    Res capture_init();
    Res pwm_test();
    Res pwm_set_duty(int duty);
    float pid_update(float delta_t, float error);
    void publish_props();
    static uint32_t pwm_percent_to_ticks(float percent);
    static float clip(float value,float min=0.0f, float max=100.0f);
    static uint32_t pwm_ticks_to_percent(uint32_t ticks);
    static uint32_t pwm_ticks_to_duty(uint32_t ticks);
    Res pwm_stop();

private:
    int _timer_publish = -1;
    int _timer_watchdog = -1;
    int _timer_pid = -1;
    bool _light = false;
    MotorMsg _motor_msg;
    Bytes _image;
    float _Kp, _Ki, _Kd;
    float _p,_i, _d;
    mcpwm_timer_handle_t _timer = nullptr;
    mcpwm_oper_handle_t _oper = nullptr;
    mcpwm_gen_handle_t _gen = nullptr;
    mcpwm_cmpr_handle_t _cmpr = nullptr;
    mcpwm_cap_timer_handle_t _cap_timer = nullptr;
    mcpwm_cap_channel_handle_t _cap_channel = nullptr;
    uint32_t _pwm_value = 0;
    float _pwm_percent = 0;
    float _rpm_measured = 0;
    uint64_t _last_rpm_measured = 0;
    float _rpm_target = 0;
    float _rpm_integral = 0;
    float _previous_error = 0;
};

// constexpr mcpwm_timer_clock_source_t MCPWM_TIMER_CLK_SRC_DEFAULT = (mcpwm_timer_clock_source_t)SOC_MOD_CLK_PLL_F160M;
constexpr uint32_t BLDC_MCPWM_TIMER_RESOLUTION_HZ = 10000000; // 10MHz, 1 tick = 0.1us
constexpr uint32_t TICKS_PER_PERIOD = 500;                    // 50us, 20KHz
constexpr gpio_num_t GPIO_CAPTURE = (gpio_num_t)14;           // gpio14
constexpr gpio_num_t GPIO_PWM_1 = (gpio_num_t)13;             // gpio13
constexpr gpio_num_t GPIO_PWM_2 = (gpio_num_t)12;             // gpio12

Res config_gpio_to_value(gpio_num_t gpio_num, uint8_t value);

#endif