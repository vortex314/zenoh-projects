#include <actor.h>
#include <driver/gpio.h>
#include <soc/gpio_num.h>

#ifndef GPIO_LED
#define GPIO_LED GPIO_NUM_2
#endif

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
    LedActor();
    LedActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~LedActor();
    void on_cmd(const Value& sv);
    void on_timer(int timer_id);
    void on_start();
};
    