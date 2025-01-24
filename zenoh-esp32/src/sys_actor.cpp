
#include <sys_actor.h>

SysActor::SysActor() : SysActor("sys", 4096, 5, 5) {}

SysActor::SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<SysEvent, SysCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);
}

void SysActor::on_cmd(SysCmd &cmd)
{
    if (cmd.action)
    {
        switch (cmd.action.value())
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
    if (id == _timer_publish)
    {
        INFO("Timer 1 : Publishing Sys properties");
        publish_props();
    }
}

SysActor::~SysActor()
{
    INFO("Stopping Sys actor");
}

Res SysActor::publish_props()
{
    _sys_msg.cpu = "ESP32";
    _sys_msg.clock = 240000000;
    _sys_msg.flash_size = 0;
    _sys_msg.ram_size = std::nullopt;
    _sys_msg.free_heap = esp_get_free_heap_size();
    _sys_msg.up_time = esp_timer_get_time()/1000;
    emit(SysEvent{.serdes = PublishSerdes{.payload = _sys_msg}});
    return Res::Ok();
}

const static InfoProp info_props_sys_msg[8] = {
    InfoProp(0, "cpu", "CPU", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "clock", "Clock in Hz", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(2, "flash_size", "Flash Size in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(3, "ram_size", "RAM Size in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(4, "free_heap", "Free Heap in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(5, "up_time", "Up Time in msec", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(6, "log_message", "Log Message", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(7, "state", "State", PropType::PROP_STR, PropMode::PROP_READ),
};

Res SysMsg::serialize(Serializer &ser)
{
    int idx = 0;
    ser.reset();
    ser.map_begin();
    ser.serialize(idx++, cpu);
    ser.serialize(idx++, clock);
    ser.serialize(idx++, flash_size);
    ser.serialize(idx++, ram_size);
    ser.serialize(idx++, free_heap);
    ser.serialize(idx++, up_time);
    ser.serialize(idx++, log_message);
    ser.map_end();
    return ser.serialize(idx++, state);
}

Res SysMsg::deserialize(Deserializer &des)
{
    return Res::Err(EAFNOSUPPORT, "Not implemented");
}
