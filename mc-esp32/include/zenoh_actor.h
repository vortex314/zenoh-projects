#ifndef ZENOH_ACTOR_H
#define ZENOH_ACTOR_H

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
#include <sys_actor.h>
#include <wifi_actor.h>
#include <limero.cpp>

enum ZenohAction
{
  Connect,
  Disconnect,
  Subscribe,
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
  Option<std::string> zid;
  Option<std::string> what_am_i;
  Option<std::string> peers;
  Option<std::string> prefix;
  Option<std::string> routers;
  Option<std::string> connect;
  Option<std::string> listen;

  Res serialize(Serializer &ser) const;
  Res deserialize(Deserializer &des);
};

MSG(ZenohPublish, std::string topic; Bytes payload; ZenohPublish(const std::string &topic, const Bytes &payload) : topic(topic), payload(payload){});
MSG(ZenohSubscribe, std::string topic; ZenohSubscribe(const std::string &topic) : topic(topic){});
MSG(ZenohUnsubscribe, std::string topic; ZenohUnsubscribe(const std::string &topic) : topic(topic){});
MSG(ZenohReceived, std::string topic; Bytes payload; ZenohReceived(const std::string &topic, const Bytes &payload) : topic(topic), payload(payload){});
MSG(ZenohConnect);
MSG(ZenohDisconnect);



class ZenohActor : public Actor
{
  int _timer_publish=-1;
  int _timer_publish_props=-1;
  int _prop_counter = 0;
public:
  ZenohActor(const char* name);
  ~ZenohActor();
  void run();
  void on_message(const Envelope &env);
  void on_start();
  void on_stop();
  void handle_timer(int id);
  void prefix(const char *prefix);
  bool is_connected() const;
  Res connect(void);
  Res disconnect();
  // Res zenoh_publish_serializable(const char *topic, Serializable &value);

  Res zenoh_publish(const char *topic, const Bytes &value);
  Res publish_props();
//  Res publish_props_info();

  Result<Void> subscribe(const std::string &topic);
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
  z_put_options_t put_options;

  std::vector<std::string> _subscribed_topics;
  std::map<std::string, z_owned_subscriber_t> _subscribers;
  std::map<std::string, z_owned_publisher_t> _publishers;
};
#endif