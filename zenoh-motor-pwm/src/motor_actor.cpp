
#include <motor_actor.h>

extern "C" bool capture_callback(
    mcpwm_cap_channel_t *cap_channel,
    const mcpwm_capture_event_data_t *capture_event,
    void *arg);

MotorActor::MotorActor() : MotorActor("sys", 4096, 5, 5) {}

MotorActor::MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<MotorEvent, MotorCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(300);
    _timer_watchdog = timer_repetitive(1000);
    _timer_pid = timer_repetitive(100);
    _Kd = 0.0f;
    _Kp = 0.5f;
    _Ki = 0.003f;
    _rpm_target = 2000.0f;
    _rpm_measured = 0.0f;
    _pwm_value = 350;
    _pwm_percent = 20.0f;
    _last_rpm_measured = 0;
    _rpm_integral = 0.0f;
}

void MotorActor::on_cmd(MotorCmd &cmd)
{
    if (cmd.msg)
    {
    }
    if (cmd.isr_msg)
    {
        uint32_t sum = cmd.isr_msg->sum;
        uint32_t count = cmd.isr_msg->count;
        float sum_as_float =  sum;
        float avg = sum_as_float / count;

        float hz = 80'000'000.0f / avg;
        float rpm = (hz * 60.0f) / 4.0f; // 4 polar tacho ?? maybe 
        INFO("Isr sum: %d count: %d freq : %f Hz. RPM = %f", sum, count, hz, rpm);

        float delta_t = sum_as_float / 80'000'000.0f; // sample time in seconds
        this->_rpm_measured = rpm;

        float pid = this->pid_update(delta_t, _rpm_target - _rpm_measured);
        this->_pwm_percent = MotorActor::pwm_clip(pid);
        this->_pwm_value = MotorActor::pwm_percent_to_ticks(_pwm_percent);

        if (this->_pwm_value > TICKS_PER_PERIOD)
        {
            this->_pwm_value = TICKS_PER_PERIOD;
        }

        INFO("sum:%d count:%d rpm:%f/%f pid:%f pwm:%f pwm_value:%d",
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
        INFO("Timer 1 : target : %f measured : %f pwm_percent : %d", _rpm_target, _rpm_measured, _pwm_percent);
        _motor_msg.rpm_measured = _rpm_measured;
        _motor_msg.rpm_target = _rpm_target;
        _motor_msg.pwm = _pwm_percent;
        _motor_msg.Kp = _Kp;
        _motor_msg.Ki = _Ki;
        _motor_msg.Kd = _Kd;

        emit(MotorEvent{.serdes = PublishSerdes(_motor_msg)});
    }
    else if (id == _timer_pid)
    {

        float delta_t = static_cast<float>(esp_timer_get_time() - _last_rpm_measured) / 1'000'000.0f; // Convert microseconds to seconds
        if (delta_t > 1.0f)
        {
            INFO("No recent rpm measurements, delta_t:%f sec", delta_t);
            _rpm_measured = _rpm_target / 2.0f;

            float pid = pid_update(delta_t, _rpm_target - _rpm_measured);
            _pwm_percent = MotorActor::pwm_clip(pid);
            _pwm_value = MotorActor::pwm_percent_to_ticks(_pwm_percent);

            if (_pwm_value > TICKS_PER_PERIOD)
            {
                _pwm_value = TICKS_PER_PERIOD;
            }

            INFO("pid:%f pwm:%f pwm_value:%d", pid, _pwm_percent, _pwm_value);

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

    if (capture_init().is_err())
    {
        ERROR("Failed to init capture");
        return;
    }
    if (pwm_init().is_err())
    {
        ERROR("Failed to init PWM");
        return;
    }
    INFO("Motor actor started");
}

MotorActor::~MotorActor()
{
    INFO("Stopping Motor actor");
}

/**
 * @brief Initializes the PWM (Pulse Width Modulation) for the MotorActor.
 *
 * This function configures and initializes the MCPWM (Motor Control Pulse Width Modulation) timer, operator, comparator, 
 * and generator to control the motor's PWM signals. It sets up the necessary configurations for the timer, operator, 
 * comparator, and generator, and defines the actions for timer and compare events.
 *
 * @return Res::Ok on successful initialization, or an appropriate error code if initialization fails.
 *
 * Configuration Details:
 * - Timer:
 *   - Group ID: 0
 *   - Clock Source: Default (160 MHz)
 *   - Resolution: 10 MHz (1 tick = 0.1 µs)
 *   - Count Mode: Up
 *   - Period: 50 µs (20 kHz)
 *   - Interrupt Priority: 1
 *   - Flags:
 *     - Update period on empty and sync events.
 *     - Power-down mode disabled.
 *
 * - Operator:
 *   - Group ID: 0
 *   - Interrupt Priority: 1
 *   - Flags:
 *     - Update generator actions and dead time on timer zero, peak, and sync events.
 *
 * - Comparator:
 *   - Interrupt Priority: 1
 *   - Flags:
 *     - Update compare value on timer zero, peak, and sync events.
 *
 * - Generator:
 *   - GPIO Pin: Configured for PWM output.
 *   - Flags:
 *     - Pull-up enabled.
 *     - Pull-down and open-drain mode disabled.
 *     - PWM signal inversion disabled.
 *
 * Actions:
 * - Sets the GPIO high when the timer event is empty.
 * - Sets the GPIO low when the compare threshold is reached.
 *
 * Additional Notes:
 * - The GPIO pin for PWM is configured using `config_gpio_to_value`.
 * - The timer is started without stopping after initialization.
 */
Res MotorActor::pwm_init()
{

    mcpwm_timer_config_t timer_config = mcpwm_timer_config_t{
        group_id : 0,
        clk_src : MCPWM_TIMER_CLK_SRC_DEFAULT,          // 160MHz PLL
        resolution_hz : BLDC_MCPWM_TIMER_RESOLUTION_HZ, // 10MHz, 1 tick = 0.1us
        count_mode : MCPWM_TIMER_COUNT_MODE_UP,
        period_ticks : TICKS_PER_PERIOD, // 50us, 20KHz, 500 ticks
        intr_priority : 1,
        flags : {
            update_period_on_empty : 1, // change 
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
            update_gen_action_on_tez : 0,  // update generator action when timer counts to zero
            update_gen_action_on_tep : 1,  // update generator action when timer counts to peak
            update_gen_action_on_sync : 0, // update generator action on sync event

            update_dead_time_on_tez : 0,  // update dead time when timer counts to zero
            update_dead_time_on_tep : 0,  // update dead time when timer counts to peak
            update_dead_time_on_sync : 0, // update dead time on sync event
        },                                // when to change the value
    };
    CHECK_ESP(mcpwm_new_operator(&operator_config, &_oper));
    CHECK_ESP(mcpwm_operator_connect_timer(_oper, _timer));

    mcpwm_comparator_config_t comparator_config = mcpwm_comparator_config_t{
        intr_priority : 1,
        flags : {
            update_cmp_on_tez : 0,  // update compare value on timer event zero
            update_cmp_on_tep : 1,  // update compare value on timer event peak
            update_cmp_on_sync : 0, // update compare value on sync event

        },
    };
    CHECK_ESP(mcpwm_new_comparator(_oper, &comparator_config, &_cmpr));

    TR(config_gpio_to_value(GPIO_PWM_2, 1)); // value 0 leads to high spike voltage , as the induction current has no way to go

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

/**
 * @brief Initializes the motor capture functionality.
 *
 * This method configures and initializes the MCPWM capture timer and channel
 * for capturing PWM signals. It sets up the timer, configures the capture
 * channel with specific GPIO and edge detection settings, and registers the
 * callback for handling capture events. Finally, it enables and starts the
 * capture timer and channel.
 *
 * @return Res::Ok() if the initialization is successful, otherwise an error code.
 */
Res MotorActor::capture_init()
{
    mcpwm_capture_timer_config_t cap_timer_config;
    bzero(&cap_timer_config, sizeof(cap_timer_config));
    cap_timer_config.group_id = 0;
    cap_timer_config.clk_src = MCPWM_CAPTURE_CLK_SRC_APB;
    cap_timer_config.resolution_hz = 80'000'000; // 80 MHz

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



/**
 * @brief Callback function for handling capture events from an MCPWM capture channel.
 *
 * This function is invoked when a capture event occurs on the specified MCPWM capture channel.
 * It calculates the time period between consecutive capture events, accumulates the total
 * time period, and counts the number of events. When the accumulated time exceeds a threshold,
 * it sends a message to the associated MotorActor instance with the captured data.
 *
 * @param cap_channel Pointer to the MCPWM capture channel that triggered the event.
 * @param capture_event Pointer to the data structure containing details about the capture event,
 *                      including the captured value and edge type.
 * @param arg Pointer to user-defined data passed to the callback. In this case, it is expected
 *            to be a pointer to a MotorActor instance.
 *
 * @return Always returns true to indicate successful handling of the capture event.
 */

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
    uint32_t cap_value = capture_event->cap_value;
    uint32_t ticks_per_period = (cap_value >= isr_data.last_capture)
                                    ? (cap_value - isr_data.last_capture)
                                    : (cap_value + (0xFFFFFFFF - isr_data.last_capture) + 1);
    isr_data.sum += ticks_per_period;
    ++isr_data.count;

    if (isr_data.sum > 8'000'000)
    { // 100 msec at 80 MHz
        MotorCmd cmd;
        IsrMsg isr_msg;
        isr_msg.sum = isr_data.sum;
        isr_msg.count = isr_data.count;
        cmd.isr_msg = isr_msg;
        ((MotorActor *)arg)->tellFromIsr(new MotorCmd{isr_msg : isr_msg});
        // Reset the sum and count for the next period
        isr_data.sum = 0;
        isr_data.count = 0;
    };

    isr_data.last_capture = cap_value;
    return true;
}

float MotorActor::pid_update(float delta_t, float error)
{
    float p = _Kp * error;
    _rpm_integral += error * delta_t;
    float i = _Ki * _rpm_integral;
    float d = _Kd * (error - _previous_error) / delta_t;

    INFO("error:%f p:%f i:%f d:%f", error, p, i, d);
    // Anti-windup: Clamp the integral term
    _rpm_integral = std::min(50.0f, std::max(-50.0f, i));

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

Res MotorMsg::serialize(Serializer &ser)
{
    ser.reset();
    ser.map_begin();
    ser.serialize(KEY("rpm_measured"), rpm_measured);
    ser.serialize(KEY("rpm_target"), rpm_target);
    ser.serialize(KEY("Kp"), Kp);
    ser.serialize(KEY("Ki"), Ki);
    ser.serialize(KEY("Kd"), Kd);
    ser.serialize(KEY("pwm"), pwm);
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