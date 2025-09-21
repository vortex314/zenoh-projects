#ifndef _SYS_ACTOR_H_
#define _SYS_ACTOR_H_
#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <value.h>
#include <option.h>
#include <ArduinoJson.h>
#include <limero.cpp>

MSG(SysPub,Option<uint64_t> uptime;Option<std::string> version;Option<std::string> cpu_board;Option<uint32_t> free_heap);

class SysActor : public Actor
{
private:
  int _timer_publish = -1;
  int _prop_counter = 0;

public:
  SysActor(const char *name);
  ~SysActor();
  void on_message(const Envelope& msg);
  void on_timer(int timer_id);
  void on_start();
  Result<Value> publish_props();
  static void set_utc(int64_t utc);
  Result<Value> publish_info();
  static void reboot(bool b);
};

#endif
