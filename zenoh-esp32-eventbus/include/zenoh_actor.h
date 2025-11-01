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
#include <led_actor.h>

DEFINE_MSG(ZenohPublish,std::string topic;
  Bytes payload;
  ZenohPublish(const std::string &topic, const Bytes &payload) : topic(topic), payload(payload){};
);
DEFINE_MSG(ZenohSubscribe, std::string topic;
  ZenohSubscribe(const std::string &topic) : topic(topic){};
);
DEFINE_MSG(ZenohUnsubscribe,
  std::string topic;
  ZenohUnsubscribe(const std::string &topic) : topic(topic){};
);
DEFINE_MSG(ZenohReceived,std::string topic;
  Bytes payload;
  ZenohReceived(const std::string &topic, const Bytes &payload) : topic(topic), payload(payload){};
);
DEFINE_MSG(ZenohConnect);
DEFINE_MSG(ZenohDisconnect);

class ZenohActor : public Actor
{
  int _timer_publish = -1;
  int _timer_publish_props = -1;
  int _prop_counter = 0;

public:
  ZenohActor(const char *name);
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
  void send_msg(const char *topic, const char *msg_type, const Bytes &bytes);
  Res collect_info();
  Res publish_props();
  //  Res publish_props_info();

  Result<bool> subscribe(const std::string &topic);
  void zenoh_unsubscribe(const std::string &topic);

  Result<z_owned_subscriber_t> declare_subscriber(const char *topic);
  void delete_subscriber(z_owned_subscriber_t sub);

  Result<z_owned_publisher_t> declare_publisher(const char *topic);

  static void subscription_handler(z_loaned_sample_t *sample, void *arg);

private:
  std::string _src_device;
  std::string _dst_device;
  z_owned_session_t _zenoh_session;
  bool _connected = false;
  z_owned_config_t config;
  z_put_options_t put_options;
  std::string zid;
  std::vector<std::string> _routers;
  std::vector<std::string> _peers;

  std::vector<std::string> _subscribed_topics;
  std::map<std::string, z_owned_subscriber_t> _subscribers;
  std::map<std::string, z_owned_publisher_t> _publishers;
};
#endif