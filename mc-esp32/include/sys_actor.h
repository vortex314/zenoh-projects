#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include <vector>
#include <value.h>
// #include <map>

struct SysEvent
{
  Option<Value> publish = nullptr;
};

enum SysAction
{
  Reboot
};

struct SysCmd
{
  Option<Value> publish_value = nullptr;
  Option<SysAction> action = nullptr;
};

class SysActor : public Actor
{
private:
  int _timer_publish=-1;
  int _timer_publish_props=-1;
  int _prop_counter = 0;
public:
  SysActor();
  SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~SysActor();
  void on_cmd(SharedValue sv);
  void on_timer(int timer_id);
  void publish_props(Value& v);
  void set_utc(int64_t utc);


private:
  void init_properties();
};
