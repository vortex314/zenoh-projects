#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include "cbor.h"
#include <vector>
#include "esp_camera.h"
// #include <map>

#define CAMERA_MODEL_AI_THINKER
#define LED_FLASH GPIO_NUM_4

struct CameraMsg : public Serializable
{
    std::optional<Bytes> image = std::nullopt;
    std::optional<bool> light = std::nullopt;

    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};

struct CameraEvent
{
    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<PublishSerdes> prop_info = std::nullopt;
};

struct CameraCmd
{
//    std::optional<PublishSerdes> serdes = std::nullopt;
    std::optional<CameraMsg> msg = std::nullopt;
};

class CameraActor : public Actor<CameraEvent, CameraCmd>
{
public:
    CameraActor();
    CameraActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~CameraActor();
    void on_cmd(CameraCmd &cmd);
    void on_timer(int timer_id);
    void on_start( void );
    Res camera_init();
    Res camera_capture();
    void process_image(int width, int height, int format, uint8_t *data, size_t len);

private:
    int _timer_publish = -1;
    bool _light = false;
    CameraMsg _camera_msg;
    Bytes _image;
    camera_config_t _camera_config;
};
