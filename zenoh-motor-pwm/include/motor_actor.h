#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include "cbor.h"
#include <vector>

#define LED_FLASH GPIO_NUM_4

struct MotorMsg : public Serializable
{
    std::optional<uint32_t> rpm_target = std::nullopt;
    std::optional<uint32_t> rpm_measured = std::nullopt;
    std::optional<float> current = std::nullopt;
    std::optional<float> Kp = std::nullopt;
    std::optional<float> Ki = std::nullopt;
    std::optional<float> Kd = std::nullopt;


    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};

struct MotorEvent
{
    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<PublishSerdes> prop_info = std::nullopt;
};

struct MotorCmd
{
//    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<MotorMsg> msg = std::nullopt;
};

class MotorActor : public Actor<MotorEvent, MotorCmd>
{
public:
    MotorActor();
    MotorActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~MotorActor();
    void on_cmd(MotorCmd &cmd);
    void on_timer(int timer_id);
    void on_start( void );
    Res camera_init();
    Res camera_capture();
    void process_image(int width, int height, int format, uint8_t *data, size_t len);

private:
    int _timer_publish = -1;
    bool _light = false;
    MotorMsg _motor_msg;
    Bytes _image;
    float _Kp,_Ki,_Kd;
};
