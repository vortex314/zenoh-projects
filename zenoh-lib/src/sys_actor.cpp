
#include <sys_actor.h>

SysActor::SysActor() : Actor<SysEvent, SysCmd>(4096, "sys", 5, 10)
{
    INFO("Starting Sys actor sizeof(SysCmd ) : %d ", sizeof(SysCmd));
    add_timer(Timer::Repetitive(1, 1000));
}

void SysActor::on_cmd(SysCmd *cmd)
{
    if (cmd->actionr)
    {
        switch (cmd->actionr.value())
        {
        case SysAction::Reboot:
            esp_restart();
            break;
        default:
            INFO("Unknown action");
        }
    }
}

void SysActor::on_timer(int id)
{
    switch (id)
    {
    case 1:
    {
        INFO("Timer 1 expired : publish ");

        sys_msg.fill();
        CborSerializer ser(120);
        ser.serialize(sys_msg);
        Bytes buffer;
        ser.get_bytes(buffer);

        emit(SysEvent{.publish = PublishMsg{"sys", buffer}});
        break;
    }
    default:
        INFO("Unknown timer expired");
    }
}

SysActor::~SysActor()
{
    INFO("Stopping Sys actor");
}

const static InfoProp info_props_sys_msg[8] = {
    InfoProp(0, "cpu", "CPU", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "clock", "Clock", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(2, "flash_size", "Flash Size", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(3, "ram_size", "RAM Size", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(4, "free_heap", "Free Heap", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(5, "up_time", "Up Time", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(6, "log_message", "Log Message", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(7, "state", "State", PropType::PROP_STR, PropMode::PROP_READ),
};

Res SysMsg::serialize(Serializer &ser)
{
    int idx = 0;
    ser.reset();
    ser.serialize(idx++, cpu);
    ser.serialize(idx++, clock);
    ser.serialize(idx++, flash_size);
    ser.serialize(idx++, ram_size);
    ser.serialize(idx++, free_heap);
    ser.serialize(idx++, up_time);
    ser.serialize(idx++, log_message);
    return ser.serialize(idx++, state);
}

Res SysMsg::deserialize(Deserializer &des)
{
    des.deserialize(cpu);
    des.deserialize(clock);
    des.deserialize(flash_size);
    des.deserialize(ram_size);
    des.deserialize(free_heap);
    des.deserialize(up_time);
    des.deserialize(log_message);
    return des.deserialize(state);
}

Res SysMsg::fill()
{
    cpu = "ESP32";
    clock = 240000000;
    /*esp_flash_t chip;
    esp_flash_init(&chip);
    uint32_t size;
    RET_ERRI(esp_flash_get_size(&chip, &size), "Failed to get flash size");
    flash_size = size;*/
    ram_size = esp_get_free_heap_size();
    up_time = esp_timer_get_time();
    return Res::Ok();
}

const InfoProp *SysMsg::info(int idx)
{
    if (idx >= sizeof(info_props_sys_msg) / sizeof(InfoProp))
        return nullptr;
    return &info_props_sys_msg[idx];
}