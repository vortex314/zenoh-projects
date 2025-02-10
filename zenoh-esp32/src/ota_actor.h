#include <actor.h>
#include <driver/gpio.h>
#include <soc/gpio_num.h>

#ifndef GPIO_LED
#define GPIO_LED GPIO_NUM_2
#endif

struct OtaMsg : public Serializable
{
    std::optional offset;
    std::optional<Bytes> image = std::nullopt;
    std::optional<int> rc = std::nullopt;
    std::optional<std::string> message = std::nullopt;

    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};

struct OtaEvent
{
    std::optional<OtaMsg> msg = std::nullopt;
};

struct OtaCmd
{
    std::optional<OtaMsg> msg = std::nullopt;
};

class OtaActor : public Actor<OtaEvent, OtaCmd>
{
    int _timer_publish = -1;

public:
    OtaActor();
    OtaActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~OtaActor();
    void on_cmd(OtaCmd &cmd);
    void on_timer(int timer_id);
    void on_start();
};
