
#include <hoverboard_actor.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <esp_flash.h>

HoverboardActor::HoverboardActor(const char *name) : Actor(name)
{
    _timer_publish = timer_repetitive(5000);
}

void HoverboardActor::on_start()
{
    INFO("Starting HoverboardActor");
    emit(new ZenohSubscribe("src/time_server/clock/utc"));
}

HoverboardActor::~HoverboardActor()
{
    INFO("Destroying HoverboardActor");
}


void HoverboardActor::on_message(const Envelope &env)
{
    const Msg &msg = *env.msg;
    msg.handle<HoverboardCmd>([&](auto hb_cmd)
                       { 
                           INFO("Received HoverboardCmd: speed=%d direction=%d", hb_cmd.speed, hb_cmd.steer);
                       });
    msg.handle<TimerMsg>([&](const TimerMsg &msg)
                         { on_timer(msg.timer_id); });
}
void HoverboardActor::on_timer(int id)
{
    if (id == _timer_publish)
        publish_info();
    else
        INFO("Unknown timer id: %d", id);
}

void HoverboardActor::publish_info()
{
    HoverboardInfo *hb_info = new HoverboardInfo();
    hb_info->temp = 37;
    emit(hb_info);
    _prop_counter++;
}
