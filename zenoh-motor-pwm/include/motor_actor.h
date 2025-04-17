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

#define LED_FLASH GPIO_NUM_4

struct MotorMsg : public Serializable
{
    std::optional<uint32_t> rpm_target = std::nullopt;
    std::optional<uint32_t> rpm_measured = std::nullopt;
    std::optional<float> pwm = std::nullopt;
    std::optional<float> Kp = std::nullopt;
    std::optional<float> Ki = std::nullopt;
    std::optional<float> Kd = std::nullopt;

    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};

struct MotorEvent
{
    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<PublishSerdes> prop_info = std::nullopt;
};

struct IsrMsg {

    uint32_t sum = 0;
    uint32_t count = 0;
};

struct MotorCmd
{
    //    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<MotorMsg> msg = std::nullopt;
    std::optional<IsrMsg> isr_msg = std::nullopt;
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
    static uint32_t pwm_percent_to_ticks(float percent);
    static float pwm_clip(float value);
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
};

// constexpr mcpwm_timer_clock_source_t MCPWM_TIMER_CLK_SRC_DEFAULT = (mcpwm_timer_clock_source_t)SOC_MOD_CLK_PLL_F160M;
constexpr uint32_t BLDC_MCPWM_TIMER_RESOLUTION_HZ = 10000000; // 10MHz, 1 tick = 0.1us
constexpr uint32_t TICKS_PER_PERIOD = 500;                    // 50us, 20KHz
constexpr gpio_num_t GPIO_CAPTURE = (gpio_num_t)14;           // gpio14
constexpr gpio_num_t GPIO_PWM_1 = (gpio_num_t)13;             // gpio13
constexpr gpio_num_t GPIO_PWM_2 = (gpio_num_t)12;             // gpio12

Res config_gpio_to_value(gpio_num_t gpio_num, uint8_t value);

#endif