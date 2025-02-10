#include <ota_actor.h>

OtaActor::OtaActor() : OtaActor("led", 4096, 5, 5) {}

OtaActor::OtaActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<OtaEvent, OtaCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);
}

OtaActor::~OtaActor()
{
}

void OtaActor::on_cmd(OtaCmd &cmd)
{
    if (cmd.msg && cmd.msg.value().image)
    {
        CHECK(flash(cmd.msg.value().image.value().data(), cmd.msg.value().image.value().size()), "Failed to flash firmware");
        return Res::Ok();
    }
}

void OtaActor::on_timer(int timer_id)
{
    INFO("timer_id %d not handled", timer_id);
}

void OtaActor::on_start()
{
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED, LED_OFF_VALUE);
}

Res OtaActor::flash(const uint8_t *data, size_t size)
{
    esp_err_t err;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    CHECK(update_partition, "Failed to find OTA update partition");
    INFO("Writing firmware to partition: %s", update_partition->label);

    // erase partition

    // Initialize OTA update
    esp_ota_handle_t ota_handle;
    CHECK(esp_ota_begin(update_partition, size, &ota_handle), "Failed to begin OTA update");

    // Write firmware data to the partition
    CHECK(esp_ota_write(ota_handle, data, size), "Failed to write OTA data")
    "

    // Finalize OTA update
    CHECK(esp_ota_end(ota_handle), "Failed to end OTA update");

    // Set the new firmware as bootable
    CHECK(esp_ota_set_boot_partition(update_partition), "Failed to set boot partition""
    INFO( "OTA update successful, restarting...");
    esp_restart();
}

Res OtaMsg::serialize(Serializer &ser)
{
    ser.begin_array();
    RET_ERR(ser.put_bytes(offset), "Failed to serialize offset");
    RET_ERR(ser.put_bytes(image), "Failed to serialize image");
    ser.put_int(rc);
    ser.put_string(message);
    ser.end_array();
    return Res::Ok();
}

Res OtaMsg::deserialize(Deserializer &des)
{
    des.begin_array();
    RET_ERR(des.get_bytes(offset), "Failed to deserialize offset");
    RET_ERR(des.get_bytes(image), "Failed to deserialize image");
    des.get_int(rc);
    des.get_string(message);
    des.end_array();
    return Res::Ok();
}