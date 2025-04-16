
#include <motor_actor.h>

extern "C" bool capture_callback(
    mcpwm_cap_channel_t *cap_channel,
    const mcpwm_capture_event_data_t *capture_event,
    void *arg);

MotorActor::MotorActor() : MotorActor("sys", 4096, 5, 5) {}

MotorActor::MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<MotorEvent, MotorCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(100);
    _timer_watchdog = timer_repetitive(1000);
    _timer_pid = timer_repetitive(1000);
}

void MotorActor::on_cmd(MotorCmd &cmd)
{
    INFO("Received Motor command");
    if (cmd.msg)
    {
    }
    if (cmd.isr_msg)
    {
        uint32_t sum = cmd.isr_msg->sum;
        uint32_t count = cmd.isr_msg->count;
        float avg = static_cast<float>(sum) / count;
        float hz = 80'000'000.0f / avg;
        float rpm = (hz * 60.0f) / 4.0f;
        INFO("Isr sum: {} count: {} freq : {} Hz. RPM = {}", sum, count, hz, rpm);

        float delta_t = static_cast<float>(sum) / 80'000'000.0f; // sample time in seconds
        this->_rpm_measured = rpm;

        float pid = this->pid_update(delta_t, _rpm_target - _rpm_measured);
        this->_pwm_percent = MotorActor::pwm_clip(pid);
        this->_pwm_value = MotorActor::pwm_percent_to_ticks(_pwm_percent);

        if (this->_pwm_value > TICKS_PER_PERIOD)
        {
            this->_pwm_value = TICKS_PER_PERIOD;
        }

        INFO("sum:{} count:{} rpm:{}/{} pid:{} pwm:{} pwm_value:{}",
             sum, count, _rpm_measured, _rpm_target, pid, _pwm_percent, _pwm_value);

        esp_err_t err = mcpwm_comparator_set_compare_value(_cmpr, _pwm_value);
        if (err != ESP_OK)
        {
            ERROR("Failed to set compare value: %s", esp_err_to_name(err));
        }

        _last_rpm_measured = esp_timer_get_time();
    }
}

void MotorActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        INFO("Timer 1 : Publishing Motor properties");
    }
    else if ( id == _timer_pid )
    {

        float delta_t = static_cast<float>(esp_timer_get_time() - _last_rpm_measured) / 1'000'000.0f; // Convert microseconds to seconds
        if (delta_t > 1.0f)
        {
            INFO("No recent rpm measurements, delta_t:{} sec", delta_t);
            _rpm_measured = _rpm_target / 2.0f;

            float pid = pid_update(delta_t, _rpm_target - _rpm_measured);
            _pwm_percent = MotorActor::pwm_clip(pid);
            _pwm_value = MotorActor::pwm_percent_to_ticks(_pwm_percent);

            if (_pwm_value > TICKS_PER_PERIOD)
            {
                _pwm_value = TICKS_PER_PERIOD;
            }

            INFO("pid:{} pwm:{} pwm_value:{}", pid, _pwm_percent, _pwm_value);

            esp_err_t err = mcpwm_comparator_set_compare_value(_cmpr, _pwm_value);
            if (err != ESP_OK)
            {
                ERROR("Failed to set compare value: %s", esp_err_to_name(err));
            }

            _last_rpm_measured = esp_timer_get_time();
        }
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

Res MotorActor::pwm_init()
{

    mcpwm_timer_config_t timer_config = mcpwm_timer_config_t{
        group_id : 0,
        clk_src : MCPWM_TIMER_CLK_SRC_DEFAULT,          // 160MHz
        resolution_hz : BLDC_MCPWM_TIMER_RESOLUTION_HZ, // 10MHz, 1 tick = 0.1us
        count_mode : MCPWM_TIMER_COUNT_MODE_UP,
        period_ticks : TICKS_PER_PERIOD, // 50us, 20KHz
        intr_priority : 1,
        flags : {
            update_period_on_empty : 1,
            update_period_on_sync : 1,
            allow_pd : 0, // set to allow power down. When this flag set, the driver will backup/restore the MCPWM registers before/after entering/exist sleep mode.
        },                // when to change the value
    };

    CHECK_ESP(mcpwm_new_timer(&timer_config, &_timer));
    CHECK_ESP(mcpwm_timer_enable(_timer));

    // define operator

    mcpwm_operator_config_t operator_config = mcpwm_operator_config_t{
        group_id : 0,
        intr_priority : 1,
        flags : {
            update_gen_action_on_tez : 1,  // update generator action when timer counts to zero
            update_gen_action_on_tep : 1,  // update generator action when timer counts to peak
            update_gen_action_on_sync : 1, // update generator action on sync event

            update_dead_time_on_tez : 1,  // update dead time when timer counts to zero
            update_dead_time_on_tep : 1,  // update dead time when timer counts to peak
            update_dead_time_on_sync : 1, // update dead time on sync event
        },                                // when to change the value
    };
    CHECK_ESP(mcpwm_new_operator(&operator_config, &_oper));
    CHECK_ESP(mcpwm_operator_connect_timer(_oper, _timer));

    mcpwm_comparator_config_t comparator_config = mcpwm_comparator_config_t{
        intr_priority : 1,
        flags : {
            update_cmp_on_tez : 1,  // update compare value on timer event zero
            update_cmp_on_tep : 1,  // update compare value on timer event peak
            update_cmp_on_sync : 1, // update compare value on sync event

        },
    };
    CHECK_ESP(mcpwm_new_comparator(_oper, &comparator_config, &_cmpr));

    config_gpio_to_value(GPIO_PWM_2, 1); // value 0 leads to high spike voltage , as the induction current has no way to go

    mcpwm_generator_config_t generator_config = mcpwm_generator_config_t{
        gen_gpio_num : GPIO_PWM_1,
        flags : {
            invert_pwm : 0,   // invert the PWM signal (done by GPIO matrix)
            io_loop_back : 0, // For debug/test, the signal output from the GPIO will be fed to the input path as well
            io_od_mode : 0,   // Configure the GPIO as open-drain mode
            pull_up : 1,      // pull up the GPIO
            pull_down : 0,    // pull down the GPIO
        },
    };
    CHECK_ESP(mcpwm_new_generator(_oper, &generator_config, &_gen));

    CHECK_ESP(mcpwm_comparator_set_compare_value(_cmpr, _pwm_value));
    // at start set GPIO high
    CHECK_ESP(mcpwm_generator_set_action_on_timer_event(
        _gen,
        mcpwm_gen_timer_event_action_t{
            direction : MCPWM_TIMER_DIRECTION_UP,
            event : MCPWM_TIMER_EVENT_EMPTY,
            action : MCPWM_GEN_ACTION_HIGH
        }));
    // go low on GPIO on compare threshold
    CHECK_ESP(mcpwm_generator_set_action_on_compare_event(
        _gen,
        mcpwm_gen_compare_event_action_t{
            direction : MCPWM_TIMER_DIRECTION_UP,
            comparator : _cmpr,
            action : MCPWM_GEN_ACTION_LOW
        }));

    //  CHECK_ESP(mcpwm_timer_enable(self.timer_handle));
    CHECK_ESP(mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP));

    return Res::Ok();
}

Res MotorActor::capture_init()
{
    mcpwm_capture_timer_config_t cap_timer_config;
    bzero(&cap_timer_config, sizeof(cap_timer_config));
    cap_timer_config.group_id = 0;

    cap_timer_config.clk_src = (mcpwm_capture_clock_source_t)SOC_MOD_CLK_PLL_F160M;
    CHECK_ESP(mcpwm_new_capture_timer(&cap_timer_config, &_cap_timer));

    mcpwm_capture_channel_config_t cap_channel_config = mcpwm_capture_channel_config_t{
        gpio_num : GPIO_CAPTURE,
        intr_priority : 1,
        prescale : 1,
        flags : {
            pos_edge : 0, // capture on positive edge
            neg_edge : 1, // capture on negative edge
            pull_up : 1,
            pull_down : 0,            // pull up the GPIO
            invert_cap_signal : 0,    // Invert the input capture signal
            io_loop_back : 0,         // For debug/test, the signal output from the GPIO will be fed to the input path as well
            keep_io_conf_at_exit : 0, // Deprecated. Driver won't change the GPIO configuration in deinilization.
        },
    };

    CHECK_ESP(mcpwm_new_capture_channel(_cap_timer, &cap_channel_config, &_cap_channel));

    // TaskHandle_t task_to_notify = xTaskGetCurrentTaskHandle();
    mcpwm_capture_event_callbacks_t cbs = mcpwm_capture_event_callbacks_t{
        on_cap : capture_callback
    };

    CHECK_ESP(mcpwm_capture_channel_register_event_callbacks(
        _cap_channel,
        &cbs,
        this));
    // activate capture channel
    CHECK_ESP(mcpwm_capture_channel_enable(_cap_channel));
    CHECK_ESP(mcpwm_capture_timer_enable(_cap_timer));
    CHECK_ESP(mcpwm_capture_timer_start(_cap_timer));

    return Res::Ok();
}

Res config_gpio_to_value(gpio_num_t gpio_num, uint8_t value)
{
    gpio_config_t io_conf = gpio_config_t{
        pin_bit_mask : (uint64_t)1 << gpio_num,
        mode : GPIO_MODE_OUTPUT,
        pull_up_en : GPIO_PULLUP_DISABLE,
        pull_down_en : GPIO_PULLDOWN_DISABLE,
        intr_type : GPIO_INTR_DISABLE,
    };
    CHECK_ESP(gpio_config(&io_conf));
    CHECK_ESP(gpio_set_level(gpio_num, value));
    return Res::Ok();
}

static struct IsrData
{
    uint32_t last_capture;
    uint32_t sum;
    uint32_t count;
} isr_data;

extern "C" bool capture_callback(
    mcpwm_cap_channel_t *cap_channel,
    const mcpwm_capture_event_data_t *capture_event,
    void *arg)
{
    uint32_t cap_value = (*capture_event).cap_value;
    //      let pos_edge = (*capture_event).cap_edge == mcpwm_capture_edge_t_MCPWM_CAP_EDGE_POS;

    uint32_t ticks_per_period;
    if (cap_value > isr_data.last_capture)
    {
        ticks_per_period = cap_value - isr_data.last_capture;
    }
    else
    {
        ticks_per_period = (0xFFFFFFFF - isr_data.last_capture) + cap_value;
    };
    isr_data.sum += ticks_per_period;
    isr_data.count += 1;

    if (isr_data.sum > 8000000)
    { // 100 msec at 80 MHz
        INFO("Capture sum: %d", isr_data.sum);
        INFO("Capture count: %d", isr_data.count);
        MotorCmd cmd;
        IsrMsg isr_msg;
        isr_msg.sum = isr_data.sum;
        isr_msg.count = isr_data.count;
        cmd.isr_msg = isr_msg;
        ((MotorActor *)arg)->tellFromIsr(&cmd);

        /*let _ = isr_data.sender.try_send(MotorEvent::Isr{
            sum : isr_data.sum,
            count : isr_data.count,
        });*/
        isr_data.sum = 0;
        isr_data.count = 0;
    };

    isr_data.last_capture = cap_value;
    return true;
}

float MotorActor::pid_update(float delta_t, float error)
{
    float p = _Kp * error;
    float i = (_Ki * error * delta_t) + _rpm_integral;
    float d = _Kd * error / delta_t;

    INFO("error:{} p:{} i:{} d:{}", error, p, i, d);

    if (i > 50.0f)
    {
        _rpm_integral = 50.0f;
    }
    else if (i < -50.0f)
    {
        _rpm_integral = -50.0f;
    }
    else
    {
        _rpm_integral = i;
    }

    return p + _rpm_integral + d;
}

uint32_t MotorActor::pwm_percent_to_ticks(float percent)
{
    float ticks = 500.0f - ((percent * static_cast<float>(TICKS_PER_PERIOD)) / 100.0f);
    return static_cast<uint32_t>(ticks);
}

float MotorActor::pwm_clip(float pwm)
{
    if (pwm > 100.0f)
    {
        return 100.0f;
    }
    else if (pwm < 0.0f)
    {
        return 0.0f;
    }
    else
    {
        return pwm;
    }
}

Res MotorActor::pwm_stop()
{
    this->_rpm_target = 0.0f;
    this->_pwm_value = 0;
    return Res::Ok();
}
