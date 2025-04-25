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
  std::optional<int64_t> utc;
  std::optional<std::string> cpu = std::nullopt;
  std::optional<uint32_t> clock = std::nullopt;
  std::optional<uint32_t> flash_size = std::nullopt;
  std::optional<uint32_t> ram_size = std::nullopt;
  std::optional<uint32_t> free_heap = std::nullopt;      // dynamic
  std::optional<uint64_t> up_time = std::nullopt;        // dynamic
  std::optional<std::string> log_message = std::nullopt; // dynamic
  std::optional<std::string> state = std::nullopt;       // dynamic

  Res serialize(Serializer &ser) const;
  Res deserialize(Deserializer &des);
};

struct SysEvent
{
  std::optional<SysMsg> msg = std::nullopt;
};

enum SysAction
{
  Reboot
};

struct SysCmd
{
  std::optional<SysMsg> msg = std::nullopt;
  std::optional<SysAction> action = std::nullopt;
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
