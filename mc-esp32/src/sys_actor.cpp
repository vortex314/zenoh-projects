
#include <sys_actor.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

SysActor::SysActor() : SysActor("sys", 4096, 5, 5) {}

SysActor::SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth) : Actor(stack_size, name, priority, queue_depth)
{
    _timer_publish = timer_repetitive(1000);
    //   _timer_publish_props = timer_repetitive(5000);
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

void SysActor::on_cmd(SharedValue pcmd)
{
    const Value &cmd = *pcmd;
    cmd["action"]["reboot"].handle<bool>([&](auto value)
                                         { esp_restart(); });

    cmd["publish"]["utc"].handle<int64_t>([&](const int64_t &utc)
                                          { set_utc(utc); });
}
void SysActor::on_timer(int id)
{
    if (id == _timer_publish)
    {
        std::shared_ptr<Value> sv = std::make_shared<Value>();
        publish_props((*sv)["publish"]);
        emit(sv);
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

void SysActor::publish_props(Value &v)
{
    v["cpu"] = "ESP32-XTENSA";
    v["clock"] = 240000000;
    v["free_heap"] = (int64_t)esp_get_free_heap_size();
    v["uptime"] = esp_timer_get_time() / 1000;
}
