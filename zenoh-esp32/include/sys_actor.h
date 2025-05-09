#include <actor.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include "cbor.h"
#include <vector>
// #include <map>

struct SysMsg : public Serializable
{
  Option<int64_t> utc;
  Option<std::string> cpu = nullptr;
  Option<uint32_t> clock = nullptr;
  Option<uint32_t> flash_size = nullptr;
  Option<uint32_t> ram_size = nullptr;
  Option<uint32_t> free_heap = nullptr;      // dynamic
  Option<uint64_t> up_time = nullptr;        // dynamic
  Option<std::string> log_message = nullptr; // dynamic
  Option<std::string> state = nullptr;       // dynamic

  Res serialize(Serializer &ser) const;
  Res deserialize(Deserializer &des);
};

struct SysEvent
{
  Option<SysMsg> publish = nullptr;
};

enum SysAction
{
  Reboot
};

struct SysCmd
{
  Option<SysMsg> publish = nullptr;
  Option<SysAction> action = nullptr;
};

class SysActor : public Actor<SysEvent, SysCmd>
{
private:
  SysMsg sys_msg;
  const char *src_sys = "sys"; // wifi, time, etc
  int _timer_publish=-1;
  int _timer_publish_props=-1;
  int _prop_counter = 0;
public:
  SysActor();
  SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~SysActor();
  void on_cmd(SysCmd &cmd);
  void on_timer(int timer_id);
  Res publish_props();
//  Res publish_props_info();

private:
  void init_properties();
  SysMsg _sys_msg;
  Bytes _buffer;
};
