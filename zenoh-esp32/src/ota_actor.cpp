#include <ota_actor.h>
#include <esp_partition.h>

OtaActor::OtaActor() : OtaActor("led", 4096, 5, 5) {}

OtaActor::OtaActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<OtaEvent, OtaCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(5000);
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
                INFO("OTA_BEGIN");
                result = ota_begin();
            }
            else if (msg.operation.value() == OTA_END)
            {
                INFO("OTA_END");
                result = ota_end();
            }
            else if (msg.operation.value() == OTA_WRITE && msg.offset && msg.image)
            {
                INFO("OTA_WRITE offset %d [%d]", msg.offset.value(), msg.image.value().size());
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
            reply.operation = msg.operation;
            emit(OtaEvent{.serdes = PublishSerdes(msg.reply_to, reply)});
        }
    }
}

void OtaActor::on_timer(int timer_id)
{
    const esp_partition_t *part = esp_ota_get_running_partition();
    OtaMsg event;
    event.partition_label = part->label;
    emit(OtaEvent{.serdes = PublishSerdes(event)});
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
    //    INFO("Writing firmware to partition: %s", _update_partition->label);
    CHECK_ESP(esp_ota_begin(_update_partition, OTA_SIZE_UNKNOWN, &_ota_handle));
    return Res::Ok();
}

Res OtaActor::ota_end()
{
    CHECK(esp_ota_end(_ota_handle));

    // Set the new firmware as bootable
    CHECK_ESP(esp_ota_set_boot_partition(_update_partition));
    INFO("OTA update successful, restarting...");
    esp_restart();
}

Res OtaActor::ota_write(uint32_t offset, Bytes &data)
{
    CHECK_ESP(esp_ota_write_with_offset(_ota_handle, data.data(), data.size(), offset));
    return Res::Ok();
}

/*Res OtaActor::flash(const uint8_t *data, size_t size)
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
}*/

Res OtaMsg::serialize(Serializer &ser)
{
    int idx = 0;
    ser.reset();
    ser.map_begin();
    ser.serialize(idx++, (std::optional<uint32_t>)operation);
    ser.serialize(idx++, offset);
    ser.serialize(idx++, image);
    ser.serialize(idx++, rc);
    ser.serialize(idx++, message);
    ser.serialize(idx++, reply_to);
    ser.serialize(idx++, partition_label);
    ser.map_end();
    return Res::Ok();
}

Res OtaMsg::deserialize(Deserializer &des)
{
  //  INFO("OtaMsg::deserialize");
    des.iterate_map([&](Deserializer &d, uint32_t key) -> Res
                    {
    //    INFO("key %d", key);
        switch (key)
        {
        case 0:
            return d.deserialize((std::optional<uint32_t>&)operation);
        case 1:
            return d.deserialize(offset);
        case 2:
            return d.deserialize(image);
        case 3:
            return d.deserialize(rc);
        case 4:
            return d.deserialize(message);
        case 5:
            return d.deserialize(reply_to);
        default:
            INFO("unknown key %d",key);
            return d.skip_next();
        } });

    return Res::Ok();
}