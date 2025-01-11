#include <actor.h>
#include <driver/gpio.h>

enum LedAction {
    LED_ON,
    LED_OFF,
    LED_BLINK,
    LED_PULSE
};

struct LedCmd {
    std::optional<LedAction> action = std::nullopt;
    std::optional<int> duration = std::nullopt;
};

struct LedEvent {
    std::optional<LedAction> action = std::nullopt;
};  

#define GPIO_LED GPIO_NUM_2

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
public:
    LedActor();
    ~LedActor();
    void on_cmd(LedCmd &cmd);
    void on_timer(int timer_id);
    void on_start();
};
    