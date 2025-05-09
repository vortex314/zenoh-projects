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
    cmd.publish.for_each([&](auto msg){
        msg.operation.for_each([&](auto operation){
            if (operation == OTA_BEGIN)
            {
                INFO("OTA_BEGIN");
                result = ota_begin();
            }
            else if (operation == OTA_END)
            {
                INFO("OTA_END");
                result = ota_end();
            }
            else if (operation == OTA_WRITE && msg.offset && msg.image)
            {
                INFO("OTA_WRITE offset %d [%d]", *msg.offset, (*msg.image).size());
                result = ota_write(*msg.offset, msg.image.ref());
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
            reply.reply_to = msg.reply_to;
            emit(OtaEvent{.publish = reply});
        });
    });

}

void OtaActor::on_timer(int timer_id)
{
    const esp_partition_t *part = esp_ota_get_running_partition();
    OtaMsg event;
    event.partition_label = part->label;
    emit(OtaEvent{.publish = event});
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

Res OtaActor::ota_write(uint32_t offset,const Bytes &data)
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

Res OtaMsg::serialize(Serializer &ser) const
{
    ser.reset();
    ser.map_begin();
    ser.serialize(KEY("operation"), operation);
    ser.serialize(KEY("offset"), offset);
    ser.serialize(KEY("image"), image);
    ser.serialize(KEY("rc"), rc);
    ser.serialize(KEY("message"), message);
    ser.serialize(KEY("reply_to"), reply_to);
    ser.serialize(KEY("partition_label"), partition_label);
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
        case H("operation"):
            return d.deserialize((std::optional<uint32_t>&)operation);
        case H("offset"):
            return d.deserialize(offset);
        case H("image"):
            return d.deserialize(image);
        case H("rc"):
            return d.deserialize(rc);
        case H("message"):
            return d.deserialize(message);
        case H("partition_label"):
            return d.deserialize(reply_to);
        default:
            INFO("unknown key %d",key);
            return d.skip_next();
        } });

    return Res::Ok();
}