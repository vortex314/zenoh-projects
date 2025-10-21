
#include <sys_actor.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <esp_flash.h>

SysActor::SysActor(const char *name) : Actor(name)
{
    _timer_publish = timer_repetitive(5000);
}

void SysActor::on_start()
{
    INFO("Starting SysActor");
    emit(new ZenohSubscribe("src/time_server/clock/utc"));
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
    INFO("Current time: %s", buffer);
    publish_info();
}

void SysActor::reboot(bool b)
{
    if (b)
        esp_restart();
}

void SysActor::on_message(const Envelope &env)
{
    const Msg &msg = *env.msg;
    msg.handle<SysCmd>([&](auto sys_cmd)
                       { if ( sys_cmd.reboot) reboot(*sys_cmd.reboot);
                         if ( sys_cmd.set_time) set_utc(*sys_cmd.set_time); 
                        if ( sys_cmd.console ) INFO("SysCmd.console from %s: %s", sys_cmd.src.c_str(), (*sys_cmd.console).c_str()); });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { on_timer(msg.timer_id); });
}
void SysActor::on_timer(int id)
{
    if (id == _timer_publish)
        publish_info();
    else
        INFO("Unknown timer id: %d", id);
}

void SysActor::publish_info()
{
    SysInfo *sys_info = new SysInfo();
    sys_info->cpu_board = "ESP32-DEVKIT1";
    sys_info->free_heap = (int64_t)esp_get_free_heap_size();
    sys_info->uptime = esp_timer_get_time() / 1000;
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    sys_info->flash = (int64_t)flash_size;
    sys_info->build_date = __DATE__ " " __TIME__;
    struct timeval tv1;
    gettimeofday(&tv1, NULL);
    sys_info->utc = tv1.tv_sec;
    emit(sys_info);
}
