#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <value.h>

MSG(SysReboot);

class SysActor : public Actor
{
private:
  int _timer_publish = -1;
  int _prop_counter = 0;

public:
  SysActor(const char *name);
  ~SysActor();
  void on_message(const Msg& msg);
  void on_timer(int timer_id);
  Result<Value> publish_props();
  void set_utc(int64_t utc);
  Result<Value> publish_info();
};
