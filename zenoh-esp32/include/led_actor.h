#include <actor.h>
#include <driver/gpio.h>
#include <soc/gpio_num.h>

#ifndef GPIO_LED
#define GPIO_LED GPIO_NUM_2
#endif

struct LedMsg{};

enum LedAction {
    LED_ON,
    LED_OFF,
    LED_BLINK,
    LED_PULSE
};

struct LedCmd {
    Option<LedAction> action = nullptr;
    Option<int> duration = nullptr;
    Option<LedMsg> serdes = nullptr;
};

struct LedEvent {
    Option<LedAction> action = nullptr;
};  



class LedActor : public Actor<LedEvent, LedCmd> {
    typedef enum State {
        LED_STATE_OFF,
        LED_STATE_ON,
        LED_STATE_BLINK,
        LED_STATE_PULSE
    } State;
    State _state = LED_STATE_BLINK;
    int _duration = 100;
    int _gpio_led = GPIO_LED;
    bool _led_is_on = false;
    int _timer_publish = -1;
public:
    LedActor();
    LedActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~LedActor();
    void on_cmd(LedCmd &cmd);
    void on_timer(int timer_id);
    void on_start();
};
    