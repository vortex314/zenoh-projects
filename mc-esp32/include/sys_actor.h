#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <serdes.h>
#include <vector>
#include <value.h>

class SysActor : public Actor
{
private:
  void init_properties();
  int _timer_publish = -1;
  int _timer_publish_props = -1;
  int _prop_counter = 0;

public:
  SysActor();
  SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~SysActor();
  void on_cmd(const Value&  );
  void on_timer(int timer_id);
  Result<Value> publish_props();
  void set_utc(int64_t utc);
  Result<Value> publish_info();
};
