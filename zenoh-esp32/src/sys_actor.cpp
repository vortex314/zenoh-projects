
#include <sys_actor.h>
#include <sys/time.h>
#include <time.h>

SysActor::SysActor() : SysActor("sys", 4096, 5, 5) {}

SysActor::SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor<SysEvent, SysCmd>(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);
    //   _timer_publish_props = timer_repetitive(5000);
}

void SysActor::on_cmd(SysCmd &cmd)
{
    cmd.action.for_each([](auto action)
                        {
        switch (action ) {
            case SysAction::Reboot:
            esp_restart();
            break;
        } });
    cmd.publish.for_each([](auto msg)
                         {
                            INFO("Sys actor received message");
                            msg.utc.for_each([](auto utc)
                                            {
                                                INFO("Setting UTC time to %lld", utc);
            // https://github.com/espressif/esp-idf/issues/10876
            // set local time to UTC https://gist.github.com/igrr/d7db8a78170bf6981f2e606b42c4361c 
            struct timeval tv = {
                .tv_sec = utc/1000,
                .tv_usec = static_cast<long int>((utc%1000)* 1000)
                };
                struct timezone tz = {0,0};
                tz.tz_minuteswest = 0;
                tz.tz_dsttime = DST_MET;
            // set timezone to UTC
            ERRNO(settimeofday(&tv, &tz)); }); 
            struct timeval tv1;
            struct tm *timeinfo;
            char buffer[80];
            
            // Get current time using gettimeofday()
            int rc = gettimeofday(&tv1, NULL);
            
            // Convert to local time
            timeinfo = localtime(&tv1.tv_sec);
            
            // Format the time as string
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            // Print with microseconds
            INFO("Current time: %s", buffer); });
}
void SysActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        publish_props();
    }
    else
    {
        INFO("Unknown timer id: %d", id);
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
    _sys_msg.ram_size = nullptr;
    _sys_msg.free_heap = esp_get_free_heap_size();
    _sys_msg.up_time = esp_timer_get_time() / 1000;
    emit(SysEvent{.publish = _sys_msg});
    return ResOk;
}
/*
InfoProp info_props_sys_msg[8] = {
    InfoProp(0, "cpu", "CPU", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "clock", "Clock in Hz", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(2, "flash_size", "Flash Size in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(3, "ram_size", "RAM Size in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(4, "free_heap", "Free Heap in bytes", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(5, "up_time", "Up Time in msec", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(6, "log_message", "Log Message", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(7, "state", "State", PropType::PROP_STR, PropMode::PROP_READ),
};

Res SysActor::publish_props_info()
{
    emit(SysEvent{.prop_info = PublishSerdes( info_props_sys_msg[_prop_counter])});
    _prop_counter = (_prop_counter + 1) % (sizeof(info_props_sys_msg) / sizeof(InfoProp));
    return ResOk;
}*/

Res SysMsg::serialize(Serializer &ser) const
{
    int idx = 0;
    ser.reset();
    ser.map_begin();
    ser.serialize(KEY("cpu"), cpu);
    ser.serialize(KEY("clock"), clock);
    ser.serialize(KEY("flash_size"), flash_size);
    ser.serialize(KEY("ram_size"), ram_size);
    ser.serialize(KEY("free_heap"), free_heap);
    ser.serialize(KEY("up_time"), up_time);
    ser.serialize(KEY("log_message"), log_message);
    ser.map_end();
    return ser.serialize(idx++, state);
}

Res SysMsg::deserialize(Deserializer &des)
{
    des.iterate_map([&](Deserializer &d, uint32_t key) -> Res
                    {
       // INFO("key %d", key);
        switch (key)
        {
        case H("utc"):
           // INFO("utc");
            return d.deserialize(utc);
        default:
            ERROR("unknown key %d",key);
            return d.skip_next();
        } });

    return ResOk;
}
