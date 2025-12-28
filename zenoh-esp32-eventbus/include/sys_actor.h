#ifndef _SYS_ACTOR_H_
#define _SYS_ACTOR_H_
#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <option.h>
#include <ArduinoJson.h>
#include <msgs.h>
#include <zenoh_actor.h>

class SysActor : public Actor
{
private:
  int _timer_publish = -1;
  int _prop_counter = 0;

public:
  SysActor(const char *name);
  ~SysActor();
  void on_message(const Envelope &msg);
  void on_timer(int timer_id);
  void on_start();
  void set_utc(int64_t utc);
  void publish_info();
  static void reboot(bool b);
};

#endif
