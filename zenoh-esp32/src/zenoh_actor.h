#ifndef ZENOH_ACTOR_H
#define ZENOH_ACTOR_H

#include "cbor.h"
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
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"
#include <zenoh-pico.h>

enum ZenohAction
{
  Connect,
  Disconnect,
  Stop
};
/*
struct ZenohSerial
{
  std::string topic;
  Serializable &value;
};*/

struct ZenohMsg : public Serializable
{
  std::optional<std::string> zid;
  std::optional<std::string> what_am_i;
  std::optional<std::string> peers;
  std::optional<std::string> prefix;
  std::optional<std::string> routers;
  std::optional<std::string> connect;
  std::optional<std::string> listen;

  Res serialize(Serializer &ser);
  Res deserialize(Deserializer &des);
};

struct ZenohEvent
{
  std::optional<PublishBytes> publish = std::nullopt; // publish a message
  std::optional<PublishSerdes> serdes = std::nullopt; // publish a serializable object
  std::optional<PublishSerdes> prop_info = std::nullopt; // publish a serializable object of propinfo
};

struct ZenohCmd
{
  std::optional<ZenohAction> action = std::nullopt;
  std::optional<PublishSerdes> serdes = std::nullopt;
  std::optional<PublishBytes> publish = std::nullopt;
};

class ZenohActor : public Actor<ZenohEvent, ZenohCmd>
{
  int _timer_publish=-1;
  int _timer_publish_props=-1;
  int _prop_counter = 0;
public:
  ZenohActor();
  ZenohActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~ZenohActor();
  void run();
  void on_timer(int id);
  void on_cmd(ZenohCmd &cmd);
  void prefix(const char *prefix);
  Res connect(void);
  Res disconnect();
  // Res zenoh_publish_serializable(const char *topic, Serializable &value);

  Res zenoh_publish(const char *topic, const Bytes &value);
  Res publish_props();
  Res publish_props_info();

  void zenoh_subscribe(const std::string &topic);
  void zenoh_unsubscribe(const std::string &topic);

  Result<z_owned_subscriber_t> declare_subscriber(const char *topic);
  void delete_subscriber(z_owned_subscriber_t sub);

  Result<z_owned_publisher_t> declare_publisher(const char *topic);

  static void subscription_handler(z_loaned_sample_t *sample, void *arg);

private:
  std::string _src_prefix;
  std::string _dst_prefix;
  z_owned_session_t _zenoh_session;
  ZenohMsg _zenoh_msg;
  bool _connected = false;
  z_owned_config_t config;
  std::map<std::string, z_owned_subscriber_t> _subscribers;
  std::map<std::string, z_owned_publisher_t> _publishers;
  std::map<std::string, PropertyCommon *> _properties;
};
#endif