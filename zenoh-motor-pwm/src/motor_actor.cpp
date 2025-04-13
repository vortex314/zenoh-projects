
#include <motor_actor.h>

MotorActor::MotorActor() : MotorActor("sys", 4096, 5, 5) {}

MotorActor::MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<MotorEvent, MotorCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(100);
}

void MotorActor::on_cmd(MotorCmd &cmd)
{
    INFO("Received Motor command");
    if (cmd.msg)
    {
    }
}

void MotorActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        INFO("Timer 1 : Publishing Motor properties");
    }
    else
    {
        INFO("Unknown timer id: %d", id);
    }
}

void MotorActor::on_start(void)
{
}

MotorActor::~MotorActor()
{
    INFO("Stopping Motor actor");
}

Res MotorMsg::serialize(Serializer &ser)
{
    ser.reset();
    ser.map_begin();
    ser.serialize(H("rpm_measured"), rpm_measured);
    ser.serialize(H("rpm_target"), rpm_target);
    ser.serialize(H("current"), current);
    ser.serialize(H("Kp"), Kp);
    ser.serialize(H("Ki"), Ki);
    ser.serialize(H("Kd"), Kd);
    ser.map_end();
    return Res::Ok();
}

Res MotorMsg::deserialize(Deserializer &des)
{
    des.map_begin();
    des.iterate_map([&](Deserializer &d, uint32_t key) -> Res
                    {
//    INFO("key %d", key);
        switch (key)
        {
        case H("rpm_target"):
            return d.deserialize(rpm_target);
        case H("Kp"):   
           return d.deserialize(Kp);
        case H("Ki"):   
           return d.deserialize(Ki);
        case H("Kd"):   
           return d.deserialize(Kd);
        default:
            INFO("unknown key %d",key);
            return d.skip_next();
        } });
    des.map_end();
    return Res::Ok();
}

#define SERVO_MIN_PULSEWIDTH_US 500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500 // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE -90         // Minimum angle
#define SERVO_MAX_DEGREE 90          // Maximum angle

#define SERVO_PULSE_GPIO 0                   // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000          // 20000 ticks, 20ms

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

Res MotorActor::pwm_init()
{
    INFO("Create timer and operator");
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
    };
    CHECK_ESP(mcpwm_new_timer(&timer_config, &_timer));

    _operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    CHECK_ESP(mcpwm_new_operator(&_operator_config, &_oper));

    INFO("Connect timer and operator");
    CHECK_ESP(mcpwm_operator_connect_timer(_oper, _timer));

    INFO("Create comparator and generator from the operator");

    mcpwm_comparator_config_t comparator_config ;
    bzero(&comparator_config, sizeof(mcpwm_comparator_config_t));
    comparator_config.flags.update_cmp_on_tez = true;

    CHECK_ESP(mcpwm_new_comparator(_oper, &comparator_config, &_comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVO_PULSE_GPIO,
    };
    CHECK_ESP(mcpwm_new_generator(_oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    CHECK_ESP(mcpwm_comparator_set_compare_value(_comparator, example_angle_to_compare(0)));

    INFO("Set generator action on timer and compare event");
    // go high on counter empty
    CHECK_ESP(mcpwm_generator_set_action_on_timer_event(generator,
                                                        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    CHECK_ESP(mcpwm_generator_set_action_on_compare_event(generator,
                                                          MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, _comparator, MCPWM_GEN_ACTION_LOW)));

    INFO("Enable and start timer");
    CHECK_ESP(mcpwm_timer_enable(_timer));
    CHECK_ESP(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP));

    return Res::Ok();
}
Res MotorActor::pwm_test()
{

    int angle = 0;
    int step = 2;
    while (1)
    {
        INFO("Angle of rotation: %d", angle);
        CHECK_ESP(mcpwm_comparator_set_compare_value(_comparator, example_angle_to_compare(angle)));
        // Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        vTaskDelay(pdMS_TO_TICKS(500));
        if ((angle + step) > 60 || (angle + step) < -60)
        {
            step *= -1;
        }
        angle += step;
    }
}
