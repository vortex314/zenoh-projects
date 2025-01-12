#include <led_actor.h>

#define LED_ON_VALUE 1
#define LED_OFF_VALUE 0

LedActor::LedActor() : Actor<LedEvent, LedCmd>(2048, "led", 5, 10)
{
    INFO("Starting LED actor sizeof(LedCmd ) : %d ", sizeof(LedCmd));
    add_timer(Timer::Repetitive(1, 1000));
}

void LedActor::on_cmd(LedCmd &cmd)
{
    if (cmd.action)
    {
        switch (cmd.action.value())
        {
        case LED_ON:
            //  INFO("LedActor::on_cmd LED_ON");
            _state = LED_STATE_ON;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            break;
        case LED_OFF:
            INFO("LedActor::on_cmd LED_OFF");
            _state = LED_STATE_OFF;
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
            break;
        case LED_PULSE:
        {
            _state = LED_STATE_PULSE;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            _led_is_on = true;
            if (cmd.duration)
            {
                //      INFO("LedActor::on_cmd duration %d", cmd.duration.value());
                _duration = cmd.duration.value();
                timer_one_shot(1, _duration);
            }
            break;
        }
        case LED_BLINK:
        {
            INFO("LedActor::on_cmd LED_BLINK");
            _state = LED_STATE_BLINK;
            _led_is_on = true;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            if (cmd.duration)
            {
                _duration = cmd.duration.value();
                timer_repetitive(1, _duration);
            }
            break;
        }
        }
    }
}

void LedActor::on_timer(int timer_id)
{
    switch (timer_id)
    {
    case 1:
    {
        switch (_state)
        {
        case LED_STATE_OFF:
        {
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
            timer_stop(1);
            break;
        }

        case LED_STATE_ON:
        {
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            timer_stop(1);
            break;
        }
        case LED_STATE_BLINK:
        {
            if (_led_is_on)
            {
                gpio_set_level(GPIO_LED, LED_OFF_VALUE);
                _led_is_on = false;
            }
            else
            {
                gpio_set_level(GPIO_LED, LED_ON_VALUE);
                _led_is_on = true;
            }
            break;
        }
        case LED_STATE_PULSE:
        {
            _led_is_on = false;
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
            timer_stop(1);
            break;
        }
        break;
        }
        break;
    }
    default:
        INFO("Unknown timer expired led");
    }
}

void LedActor::on_start()
{
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED, 0);
}