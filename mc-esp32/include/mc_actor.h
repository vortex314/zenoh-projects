#ifndef ZENOH_ACTOR_H
#define ZENOH_ACTOR_H

#include "json.h"
#include <actor.h>
#include <functional>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <log.h>
#include <msg_info.h>
#include <map>
#include <util.h>
#include <result.h>
#include <option.h>
#include <value.h>
#include <memory>

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"

enum McAction
{
  Connect,
  Disconnect,
  Subscribe,
  Stop
};
/*
struct McSerial
{
  std::string topic;
  Serializable &value;
};*/



class McActor : public Actor
{
  private:
  std::string _src_prefix;
  std::string _dst_prefix;
  bool _connected = false;
  std::vector<std::string> _subscribed_topics;
  int _timer_publish=-1;
  int _timer_publish_props=-1;
  int _prop_counter = 0;
  int _socket = -1; // socket for multicast communication

public:
  McActor();
  McActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~McActor();
  void run();
  void on_timer(int id);
  void on_cmd(SharedValue cmd);
  void on_start() override;
  void prefix(const char *prefix);
  bool is_connected() const;
  Result<Void> connect(void);
  Result<Void> disconnect();
  // Result zenoh_publish_serializable(const char *topic, Serializable &value);
  Result<Bytes> receive();
  Result<Void> send(const std::string& data);
  Result<Void> publish_props();
//  Result publish_props_info();

  Result<Void> subscribe(const std::string &topic);
  void get_props(Value& v) const;



};
#endif