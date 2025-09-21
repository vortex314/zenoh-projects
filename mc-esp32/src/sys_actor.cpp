
#include <sys_actor.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

SysActor::SysActor(const char *name) : Actor(name)
{
    _timer_publish = timer_repetitive(5000);
}

void SysActor::on_start()
{
    INFO("Starting SysActor");
    eventbus()->push(new Envelope(new Subscribe("time_server", this->name(), UINT32_MAX)));
}

SysActor::~SysActor()
{
    INFO("Destroying SysActor");
}
void SysActor::set_utc(int64_t utc)
{
    INFO("Setting UTC time to %lld", utc);
    // https://github.com/espressif/esp-idf/issues/10876
    // set local time to UTC https://gist.github.com/igrr/d7db8a78170bf6981f2e606b42c4361c
    struct timeval tv = {
        .tv_sec = utc / 1000,
        .tv_usec = static_cast<long int>((utc % 1000) * 1000)};
    struct timezone tz = {0, 0};
    tz.tz_minuteswest = 0;
    tz.tz_dsttime = DST_MET;
    // set timezone to UTC
    ERRNO(settimeofday(&tv, &tz));
    struct timeval tv1;
    struct tm *timeinfo;
    char buffer[80];

    setenv("TZ", "MET", 1);
    tzset();
    // Get current time using gettimeofday()
    gettimeofday(&tv1, NULL);

    // Convert to local time
    timeinfo = localtime(&tv1.tv_sec);

    // Format the time as string
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Print with microseconds
    INFO("Current time: %s", buffer);
}

void SysActor::reboot(bool b)
{
    if (b)
    {
        esp_restart();
    }
}

void SysActor::on_message(const Envelope &env)
{
    const Msg& msg = *env.msg;
    msg.handle<SysCmd>([](auto sys_cmd)
                       { if ( sys_cmd.reboot) reboot(*sys_cmd.reboot);
                         if ( sys_cmd.set_time) set_utc(*sys_cmd.set_time); });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { on_timer(msg.timer_id); });
}
void SysActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        SysInfo* sys_info = new SysInfo();
        sys_info->cpu_board = "ESP32-DEVKIT1";
        sys_info->free_heap = (int64_t)esp_get_free_heap_size();
        sys_info->uptime = esp_timer_get_time() / 1000;
        emit(sys_info);
    }
    else
    {
        INFO("Unknown timer id: %d", id);
    }
}

static PropInfo prop_info[] = {
    {"cpu", "S", "CPU core", "R", nullptr, nullptr},
    {"clock", "I", "cpu clock frequency in Hz", "R", nullptr, nullptr},
    {"free_heap", "I", "available memory on the heap", "R", nullptr, nullptr},
    {"uptime", "I", "millisec since last boot of system", "R", nullptr, nullptr},

};

constexpr int info_size = sizeof(prop_info) / sizeof(PropInfo);

Result<Value> SysActor::publish_info()
{
    Value v;
    int idx = _prop_counter % info_size;
    PropInfo &pi = prop_info[idx];
    v[pi.name]["type"] = pi.type;
    v[pi.name]["desc"] = pi.desc;
    v[pi.name]["mode"] = pi.mode;
    pi.min.inspect([&](const float &min)
                   { v[pi.name]["min"] = min; });
    pi.max.inspect([&](const float &max)
                   { v[pi.name]["max"] = max; });
    return Result<Value>(v);
}

Result<Value> SysActor::publish_props()
{
    Value v;
    v["cpu"] = "ESP32-XTENSA";
    v["clock"] = 240000000;
    v["free_heap"] = (int64_t)esp_get_free_heap_size();
    v["uptime"] = esp_timer_get_time() / 1000;
    return Result<Value>(v);
}
