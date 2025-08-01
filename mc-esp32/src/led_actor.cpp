#include <led_actor.h>

#define LED_ON_VALUE 1
#define LED_OFF_VALUE 0

LedActor::LedActor() : LedActor("led", 4096, 5, 5) {}

LedActor::LedActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor(stack_size, name, priority, queue_depth)
{
    _timer_led = timer_repetitive(_duration);
}

LedActor::~LedActor()
{
}

void LedActor::on_cmd(const Value &cmd)
{
    //   INFO("Led Received command: %s", cmd.toJson().c_str());
    cmd["action"].handle<std::string>([&](const std::string &action)
                                      {
        if (action == "ON")
        {
            _state = LED_STATE_ON;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
        }
        else if (action == "OFF")
        {
            _state = LED_STATE_OFF;
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
        }
        else if (action == "PULSE")
        {
            cmd["duration"].set<int64_t>(_duration);
             _state = LED_STATE_PULSE;
            _led_is_on = true;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            timer_fire(_timer_led, _duration); 
        }
        else if (action == "BLINK")
        {
            cmd["duration"].set<int64_t>(_duration);
            _state = LED_STATE_BLINK;
            _led_is_on = true;
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            timer_fire(_timer_led, _duration); 
        } });
}

void LedActor::on_timer(int timer_id)
{
    if (timer_id == _timer_led)
    {
        switch (_state)
        {
        case LED_STATE_OFF:
        {
            gpio_set_level(GPIO_LED, LED_OFF_VALUE);
            timer_stop(_timer_led);
            break;
        }

        case LED_STATE_ON:
        {
            gpio_set_level(GPIO_LED, LED_ON_VALUE);
            timer_stop(_timer_led);
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
            timer_stop(_timer_led);
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