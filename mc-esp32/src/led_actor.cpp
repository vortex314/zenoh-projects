#include <led_actor.h>

#define LED_ON_VALUE 1
#define LED_OFF_VALUE 0

LedActor::LedActor(const char* name) : Actor(name) { _timer_led = timer_repetitive(_duration); }

LedActor::~LedActor()
{
}

void LedActor::on_message(const Msg &msg)
{
    msg.handle<LedOn>([&](auto _)
                      { 
                        _state = LED_STATE_ON;
            gpio_set_level(GPIO_LED, LED_ON_VALUE); });

    msg.handle<LedOff>([&](auto _)
                       {             
                        _state = LED_STATE_OFF;
            gpio_set_level(GPIO_LED, LED_OFF_VALUE); });

    msg.handle<LedPulse>([&](auto led_pulse)
                         {
        _duration = led_pulse.duration_msec;
        _state = LED_STATE_PULSE;
        _led_is_on = true;
        gpio_set_level(GPIO_LED, LED_ON_VALUE);
        timer_fire(_timer_led, _duration); });

    msg.handle<LedBlink>([&](auto led_blink)
                         {
        _duration = led_blink.interval_msec;
        _state = LED_STATE_BLINK;
        _led_is_on = true;
        gpio_set_level(GPIO_LED, LED_ON_VALUE);
        timer_fire(_timer_led, _duration); });
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