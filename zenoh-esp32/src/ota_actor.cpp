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
    Res result;
    if (cmd.msg)
    {
        OtaMsg &msg = cmd.msg.value();
        if (msg.operation)
        {
            if (msg.operation.value() == OTA_BEGIN)
            {
                result = ota_begin();
            }
            else if (msg.operation.value() == OTA_END)
            {
                result = ota_end();
            }
            else if (msg.operation.value() == OTA_WRITE && msg.offset && msg.image)
            {
                result = ota_write(msg.offset.value(), msg.image.value());
            }
            else
            {
                INFO(" invalid message ");
                result = Res::Err(-1, "invalid message ");
            }
            OtaMsg reply;
            reply.rc = result.rc();
            reply.message = result.msg();
            reply.offset = msg.offset;
            emit(OtaEvent{.serdes = PublishSerdes(msg.reply_to, reply)});
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

Res OtaActor::ota_begin()
{
    _update_partition = (esp_partition_t *)esp_ota_get_next_update_partition(NULL);
    if (_update_partition == NULL)
    {
        ERROR("Unable to get next OTA partition");
        return Res::Err(EBADF, "Unable to get next OTA partition");
    }
    INFO("Writing firmware to partition: %s", _update_partition->label);
    CHECK(esp_ota_begin(_update_partition, OTA_SIZE_UNKNOWN, &_ota_handle));
    return Res::Ok();
}

Res OtaActor::ota_end()
{
    CHECK(esp_ota_end(_ota_handle));

    // Set the new firmware as bootable
    CHECK(esp_ota_set_boot_partition(_update_partition));
    INFO("OTA update successful, restarting...");
    esp_restart();
}

Res OtaActor::ota_write(uint32_t offset, Bytes &data)
{
    CHECK(esp_ota_write_with_offset(_ota_handle, data.data(), data.size(),offset));
    return Res::Ok();
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
    ser.serialize((std::optional<uint32_t>)operation);
    ser.serialize(offset);
    ser.serialize(image);
    ser.serialize(rc);
    ser.serialize(message);
    ser.serialize(reply_to);
    ser.array_end();
    return Res::Ok();
}

Res OtaMsg::deserialize(Deserializer &des)
{
    TR(des.array_begin());
    std::optional<uint32_t> op;
    TR(des.deserialize(op));
    if (op)
        operation = (OtaOperation)op.value();
    TR(des.deserialize(offset));
    TR(des.deserialize(image));
    TR(des.deserialize(rc));
    TR(des.deserialize(message));
    TR(des.deserialize(reply_to));
    TR(des.array_end());
    return Res::Ok();
}