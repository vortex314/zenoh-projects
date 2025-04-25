#include <led_actor.h>

#define LED_ON_VALUE 1
#define LED_OFF_VALUE 0

LedActor::LedActor() : LedActor("led", 4096, 5, 5) {}

LedActor::LedActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<LedEvent, LedCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);
}

LedActor::~LedActor()
{
}

void LedActor::on_cmd(LedCmd &cmd)
{
    if (cmd.action)
    {
        switch (*cmd.action)
        {
        case LED_ON:
        {
            _state = LED_STATE_ON;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            break;
        }
        case LED_OFF:
        {
            _state = LED_STATE_OFF;
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
            break;
        }
        case LED_PULSE:
        {
            _state = LED_STATE_PULSE;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            _led_is_on = true;
            if (cmd.duration)
            {
                _duration = *cmd.duration;
                timer_fire(_timer_publish, _duration);
            }
            break;
        }
        case LED_BLINK:
        {
            _state = LED_STATE_BLINK;
            _led_is_on = true;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            if (cmd.duration)
            {
                _duration = *cmd.duration;
                timer_fire(_timer_publish, _duration);
            }
            break;
        }
        }
    }
}

void LedActor::on_timer(int timer_id)
{
    if (timer_id == _timer_publish)
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
        }
    }
    else
    {
        INFO("timer_id %d not handled", timer_id);
    }
}

void LedActor::on_start()
{
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED, LED_OFF_VALUE);
}