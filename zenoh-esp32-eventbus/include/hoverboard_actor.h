#ifndef _HOVERBOARD_ACTOR_H_
#define _HOVERBOARD_ACTOR_H_
#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <value.h>
#include <option.h>
#include <ArduinoJson.h>
#include <limero.cpp>
#include <zenoh_actor.h>


class HoverboardActor : public Actor
{
private:
    int _timer_publish = -1;
    int _prop_counter = 0;

public:
    HoverboardActor(const char *name);
    ~HoverboardActor();
    void on_message(const Envelope &msg);
    void on_timer(int timer_id);
    void on_start();
    void publish_info();
};

#endif
