#ifndef ZENOH_ACTOR_H
#define ZENOH_ACTOR_H

#include "cbor.h"
#include <channel.h>
#include <functional>
#include <optional>
#include <serdes.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <log.h>
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
//#pragma GCC diagnostic ignored "-Wunused-variable"
#include <zenoh-pico.h>

enum ZenohAction { Connect, Disconnect, Stop };

struct ZenohSerial {
  std::string topic;
  Serializable &value;
};

struct ZenohBinary {
  std::string topic;
  Bytes value;
};

struct ZenohEvent {
  std::optional<ZenohBinary> binary;
};

struct ZenohCmd {
  std::optional<ZenohAction> action=std::nullopt;
  std::optional<ZenohSerial> publish_serialized=std::nullopt;
  std::optional<ZenohBinary> publish_binary=std::nullopt;
};

class ZenohActor {
public:
  std::vector<std::function<void(ZenohEvent)>> handlers;
  Channel<ZenohCmd*> cmds;
  Timers timers;
  ZenohActor();
  ~ZenohActor();
  void run();
  Res on_timeout(int id);
  Res on_cmd(ZenohCmd& cmd);
  void emit(ZenohEvent event);
  void prefix(const char *prefix);
  Res connect(void);
  Res disconnect();
  Res zenoh_publish_serializable(const char *topic, Serializable &value);
  Res zenoh_publish_binary(const char *topic, const Bytes &value);
  void zenoh_subscribe(const std::string &topic);
  void zenoh_unsubscribe(const std::string &topic);
  Result<z_owned_subscriber_t> declare_subscriber(const char *topic);
  void delete_subscriber(z_owned_subscriber_t sub);
  Result<z_owned_publisher_t> declare_publisher(const char *topic);
  static void data_handler(z_loaned_sample_t *sample, void *arg);
  Res publish_topic_value(const char *topic, Serializable &value);
  Res publish_slow();
  Res publish_info();
  void zenoh_publish(const std::string &topic, const Bytes &value);

private:
  std::string _src_prefix;
  std::string _dst_prefix;
  CborSerializer ser;
  CborDeserializer des;
  z_owned_session_t zenoh_session;
  bool _connected = false;
  z_owned_config_t config;
  z_owned_subscriber_t subscriber;
  z_owned_publisher_t publisher;
};
#endif