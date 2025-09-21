#ifndef LED_ACTOR_H
#define LED_ACTOR_H

#include <actor.h>
#include <driver/gpio.h>
#include <soc/gpio_num.h>

#ifndef GPIO_LED
#define GPIO_LED GPIO_NUM_2
#endif

MSG(LedBlink,uint32_t interval_msec);
MSG(LedOff);
MSG(LedOn);
MSG(LedPulse,uint32_t duration_msec;LedPulse( uint32_t duration_msec) : duration_msec(duration_msec) { });

class LedActor : public Actor {
    typedef enum State {
        LED_STATE_OFF,
        LED_STATE_ON,
        LED_STATE_BLINK,
        LED_STATE_PULSE
    } State;
    State _state = LED_STATE_BLINK;
    int64_t _duration = 100;
    int _gpio_led = GPIO_LED;
    bool _led_is_on = false;
    int _timer_led = -1;
public:
    LedActor(const char* name);
    ~LedActor();
    void on_message(const Envelope& msg);
    void on_timer(int timer_id);
    void on_start();
};
    
#endif