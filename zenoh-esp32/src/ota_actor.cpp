#include <ota_actor.h>
#include <esp_partition.h>

OtaActor::OtaActor() : OtaActor("led", 4096, 5, 5) {}

OtaActor::OtaActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<OtaEvent, OtaCmd>(stack_size, name, priority, queue_depth)
{
  //  _timer_publish = timer_repetitive(1000);
}

OtaActor::~OtaActor()
{
}

void OtaActor::on_cmd(OtaCmd &cmd)
{
    if (cmd.msg && cmd.msg.value().image)
    {
        if (flash(cmd.msg.value().image.value().data(), cmd.msg.value().image.value().size()).is_err())
        {
            OtaMsg msg;
            msg.rc = -1;
            msg.message = "Failed to flash image";
            emit(OtaEvent{.serdes = PublishSerdes(msg)});
        }
    }
}

void OtaActor::on_timer(int timer_id)
{
    INFO("timer_id %d not handled", timer_id);
}

void OtaActor::on_start()
{
}

Res OtaActor::flash(const uint8_t *data, size_t size)
{
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL)
    {
        ERROR("Unable to get next OTA partition");
        return Res::Err(EBADF, "Unable to get next OTA partition");
    }
    INFO("Writing firmware to partition: %s", update_partition->label);

    // erase partition

    // Initialize OTA update
    esp_ota_handle_t ota_handle;
    CHECK(esp_ota_begin(update_partition, size, &ota_handle));

    // Write firmware data to the partition
    CHECK(esp_ota_write(ota_handle, data, size));

    // Finalize OTA update
    CHECK(esp_ota_end(ota_handle));

    // Set the new firmware as bootable
    CHECK(esp_ota_set_boot_partition(update_partition));
    INFO("OTA update successful, restarting...");
    esp_restart();
}

Res OtaMsg::serialize(Serializer &ser)
{
    ser.array_begin();
    ser.serialize(offset);
    ser.serialize(image);
    ser.serialize(rc);
    ser.serialize(message);
    ser.array_end();
    return Res::Ok();
}

Res OtaMsg::deserialize(Deserializer &des)
{
    TR(des.array_begin());
    TR(des.deserialize(offset));
    TR(des.deserialize(image));
    TR(des.deserialize(rc));
    TR(des.deserialize(message));
    TR(des.array_end());
    return Res::Ok();
}