#include <actor.h>
#include <driver/gpio.h>
#include <soc/gpio_num.h>
#include <esp_ota_ops.h>

#ifndef GPIO_LED
#define GPIO_LED GPIO_NUM_2
#endif

typedef enum {
    OTA_BEGIN =0,
    OTA_END,
    OTA_WRITE,

} OtaOperation;

struct OtaMsg : public Serializable
{
    std::optional<OtaOperation> operation;
    std::optional<uint32_t> offset;
    std::optional<Bytes> image = std::nullopt;
    std::optional<int32_t> rc = std::nullopt;
    std::optional<std::string> message = std::nullopt;
    std::optional<std::string> reply_to = std::nullopt;
    std::optional<std::string> partition_label = std::nullopt;

    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};

struct OtaEvent
{
    std::optional<PublishSerdes> serdes = std::nullopt;
};

struct OtaCmd
{
    std::optional<OtaMsg> msg = std::nullopt;
};

class OtaActor : public Actor<OtaEvent, OtaCmd>
{
    int _timer_publish = -1;
    esp_ota_handle_t _ota_handle;
    esp_partition_t *_update_partition;

public:
    OtaActor();
    OtaActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
    ~OtaActor();
    void on_cmd(OtaCmd &cmd);
    void on_timer(int timer_id);
    void on_start();
 //   Res flash(const uint8_t *data, size_t size);
    Res ota_begin();
    Res ota_end();
    Res ota_write(uint32_t offset,Bytes& bytes );
};
