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
  std::optional<std::string> cpu = std::nullopt;
  std::optional<uint32_t> clock = std::nullopt;
  std::optional<uint32_t> flash_size = std::nullopt;
  std::optional<uint32_t> ram_size = std::nullopt;
  std::optional<uint32_t> free_heap = std::nullopt;      // dynamic
  std::optional<uint64_t> up_time = std::nullopt;        // dynamic
  std::optional<std::string> log_message = std::nullopt; // dynamic
  std::optional<std::string> state = std::nullopt;       // dynamic

  Res serialize(Serializer &ser);
  Res deserialize(Deserializer &des);
};

struct SysEvent
{
  std::optional<PublishSerdes> serdes = std::nullopt;
};

enum SysAction
{
  Reboot
};

struct SysCmd
{
  std::optional<PublishSerdes> serdes = std::nullopt;
  std::optional<SysAction> action = std::nullopt;
};

class SysActor : public Actor<SysEvent, SysCmd>
{
private:
  SysMsg sys_msg;
  const char *src_sys = "sys"; // wifi, time, etc
  int _timer_publish=-1;
public:
  SysActor();
  SysActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~SysActor();
  void on_cmd(SysCmd &cmd);
  void on_timer(int timer_id);
  Res publish_props();

private:
  void init_properties();
  SysMsg _sys_msg;
  Bytes _buffer;

  Property<uint64_t> _up_time;
  Property<uint32_t> _free_heap;
  Property<std::string> _cpu;
};
