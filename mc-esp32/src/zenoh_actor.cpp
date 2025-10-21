#include <zenoh_actor.h>

#define CLIENT_OR_PEER 1 // 0: Client mode; 1: Peer mode
#if CLIENT_OR_PEER == 0
#define MODE "client"
#define CONNECT "" // If empty, it will scout
#elif CLIENT_OR_PEER == 1
#define MODE "peer"
#define CONNECT "udp/224.0.0.123:7447#iface=lo"
#else
#error "Unknown Zenoh operation mode. Check CLIENT_OR_PEER value."
#endif

ZenohActor::ZenohActor(const char *name)
    : Actor(name)
{
  _timer_publish = timer_repetitive(5000); // timer for publishing properties
  z_put_options_default(&put_options);
  z_owned_encoding_t encoding;
  z_result_t r = z_encoding_from_str(&encoding, "application/cbor"); // ZP_ENCODING_APPLICATION_CBOR;
  if (r != Z_OK)
  {
    INFO("Failed to get encoding for application/cbor ");
  }
  // put_options.encoding = z_move(encoding); // commenting this line out makes it work fine
  put_options.congestion_control = Z_CONGESTION_CONTROL_DROP;
}

void ZenohActor::handle_timer(int id)
{
  if (id == _timer_publish)
  {
    publish_props();
  }
  else
    INFO("Unknown timer id: %d", id);
}

void ZenohActor::send_msg(const char *src, const Msg *msg)
{
  char topic[100];
  sprintf(topic, "%s/%s/%s", _src_device.c_str(), src, msg->type_id());
  zenoh_publish(topic, msg->serialize());
}

void ZenohActor::on_message(const Envelope &env)
{
  const Msg &msg = *env.msg;
  msg.handle<SysInfo>([&](const auto &msg)
                      { send_msg(env.src->name(), &msg); });
  msg.handle<WifiInfo>([&](const auto &msg)
                       { send_msg(env.src->name(), &msg); });
  msg.handle<ZenohInfo>([&](const auto &msg)
                        { send_msg(env.src->name(), &msg); });
  msg.handle<TimerMsg>([&](const TimerMsg &msg)
                       { handle_timer(msg.timer_id); });
  msg.handle<ZenohPublish>([&](const ZenohPublish &pub)
                           { if (zenoh_publish(pub.topic.c_str(), pub.payload).is_err())
                             {
                               INFO("Failed to publish message");
                             } });
  msg.handle<ZenohSubscribe>([&](const ZenohSubscribe &sub)
                             { subscribe(sub.topic); });
  msg.handle<ZenohUnsubscribe>([&](const ZenohUnsubscribe &unsub)
                               { zenoh_unsubscribe(unsub.topic); });
  msg.handle<ZenohConnect>([&](auto _)
                           { if (!_connected)
                             {
                               Res res = connect();
                               if (res.is_err())
                               {
                                 INFO("Failed to connect to Zenoh: %s", res.msg());
                                 vTaskDelay(1000 / portTICK_PERIOD_MS);
                                 emit(new ZenohConnect());
                             } else {
                               INFO("Connected to Zenoh");
                             }
                             subscribe("dst/esp1/**");
                           } });
  msg.handle<ZenohDisconnect>([&](auto _)
                              { if (_connected)
                                { disconnect(); } });
}

void zid_to_hex(const z_id_t *id, char *hex)
{
  char *offset = hex;
  for (int i = 15; i >= 0; i--)
  {
    sprintf(offset, "%02x", id->id[i]);
    offset += 2;
  }
}

Res ZenohActor::connect(void)
{
  // Initialize Zenoh Session and other parameters
  z_owned_config_t config;
  z_config_default(&config);
  CHECK(zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE));
  if (strcmp(CONNECT, "") != 0)
  {
    //        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY,
    //        CONNECT);
    CHECK(zp_config_insert(z_loan_mut(config), Z_CONFIG_LISTEN_KEY, CONNECT));
  }
  // Open Zenoh session
  CHECK(z_open(&_zenoh_session, z_move(config), NULL));

  // Start the receive and the session lease loop for zenoh-pico
  CHECK(zp_start_read_task(z_loan_mut(_zenoh_session), NULL));
  CHECK(zp_start_lease_task(z_loan_mut(_zenoh_session), NULL));
  collect_info();
  INFO("Zenoh session opened with ZID %s", zid.c_str());
  _connected = true;
  return ResOk;
}

std::string z_str_to_string(z_owned_string_t &z_str)
{
  return std::string(z_string_data(z_loan(z_str)), z_string_len(z_loan(z_str)));
}

Res ZenohActor::collect_info()
{

  z_id_t _zid = z_info_zid(z_loan(_zenoh_session));
  z_owned_string_t z_str;
  z_id_to_string(&_zid, &z_str);
  this->zid = z_str_to_string(z_str);
  z_drop(z_move(z_str));

  auto callback_push = [](const z_id_t *id, void *context)
  {
    z_owned_string_t z_str;
    z_id_to_string(id, &z_str);
    std::vector<std::string> *vec = (std::vector<std::string> *)context;
    std::string s = z_str_to_string(z_str);
    vec->push_back(s);
    INFO("Lambda : %s", s.c_str());
    z_drop(z_move(z_str));
  };
  z_owned_closure_zid_t zid_closure;

  _routers.clear();
  CHECK(z_closure_zid(&zid_closure, callback_push, NULL, &_routers));
  CHECK(z_info_routers_zid(z_loan(_zenoh_session), z_move(zid_closure)));

  _peers.clear();
  CHECK(z_closure_zid(&zid_closure, callback_push, NULL, &_peers));
  CHECK(z_info_peers_zid(z_loan(_zenoh_session), z_move(zid_closure)));

  z_drop(z_move(zid_closure));

  return ResOk;
}

Res ZenohActor::disconnect()
{
  INFO("Closing Zenoh Session...");
  if (z_session_is_closed(z_loan_mut(_zenoh_session)))
  {
    INFO("Zenoh session is not open");
    return ResOk;
  }

  _connected = false;
  for (auto &sub : _subscribers)
  {
    z_drop(z_move(sub.second));
    delete_subscriber(sub.second);
  }
  _subscribers.clear();
  zp_stop_read_task(z_loan_mut(_zenoh_session));
  zp_stop_lease_task(z_loan_mut(_zenoh_session));
  z_drop(z_move(_zenoh_session));
  //  z_drop(z_move(zenoh_session));
  INFO("Zenoh session closed ");
  return ResOk;
}

bool ZenohActor::is_connected() const
{
  return _connected;
}

void ZenohActor::prefix(const char *device_name)
{
  _src_device = "src/";
  _src_device += device_name;
  _dst_device = "dst/";
  _dst_device += device_name;
  _subscribed_topics.push_back(_dst_device + "/**");
}

Result<Void> ZenohActor::subscribe(const std::string &topic)
{
  if (_connected)
  {
    auto sub = declare_subscriber(topic.c_str());
    if (sub.is_err())
    {
      return Result<Void>(-1, "Failed to declare subscriber");
    }
    else
    {
      _subscribers.emplace(topic, sub.ref());
    }
  }
  else
  {
    return Result<Void>(-1, "Not connected to Zenoh");
  }
  return Result<Void>(true);
}

Result<z_owned_subscriber_t> ZenohActor::declare_subscriber(const char *topic)
{
  z_owned_subscriber_t sub;
  z_view_keyexpr_t ke;
  z_subscriber_options_t opts;
  z_subscriber_options_default(&opts);
  z_view_keyexpr_from_str_unchecked(&ke, topic);
  INFO("Declaring subscriber for '%s'...", topic);
  z_owned_closure_sample_t callback;

  z_closure(&callback, ZenohActor::subscription_handler, NULL, this);

  TEST_RC(z_owned_subscriber_t,
          z_declare_subscriber(z_loan(_zenoh_session), &sub, z_loan(ke),
                               z_move(callback), &opts),
          "Unable to declare subscriber for key expression");
  return Result<z_owned_subscriber_t>(sub);
}

void ZenohActor::delete_subscriber(z_owned_subscriber_t sub)
{
  INFO("Deleting subscriber...");
  // z_drop(z_loan(sub));
  INFO("Subscriber deleted");
}

ZenohActor::~ZenohActor()
{
  INFO("Closing Zenoh Session...");
  zp_stop_read_task(z_loan_mut(_zenoh_session));
  zp_stop_lease_task(z_loan_mut(_zenoh_session));
  z_drop(z_move(_zenoh_session));
  INFO("Zenoh session closed ");
}
/*
Result<z_owned_publisher_t> ZenohActor::declare_publisher(const char *topic)
{
  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  z_publisher_options_t opts;
  z_owned_encoding_t encoding;
  z_publisher_options_default(&opts);
  z_encoding_from_str(&encoding, "application/cbor"); // encoding type 8
  opts.encoding = z_move(encoding);
  z_view_keyexpr_from_str_unchecked(&ke, topic);
  INFO("Declaring publisher for '%s'...", topic);
  if (z_declare_publisher(z_loan(_zenoh_session), &pub, z_loan(ke), &opts) < 0)
  {
    INFO("Unable to declare publisher for key expression!");
    return Result<z_owned_publisher_t>::Err(
        -1, "Unable to declare publisher for key expression");
  }
  INFO("OK");
  return Result<z_owned_publisher_t>::Ok(pub);
}*/
/*
Res ZenohActor::zenoh_publish_serializable(const char *topic,
                                           Serializable &value)
{
  z_view_keyexpr_t keyexpr;
  z_owned_bytes_t payload;
  RET_ERR(ser.reset(), "Failed to reset serializer");
  Bytes buffer;
  RET_ERR(ser.serialize(value), "Failed to serialize");
  RET_ERR(ser.get_bytes(buffer), "Failed to get bytes");
  std::string topic_name = _src_prefix;
  topic_name += "/";
  topic_name += topic;

  z_view_keyexpr_from_str_unchecked(&keyexpr, topic_name.c_str());
  CHECK(z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size()));
  CHECK(z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL));
  z_drop(z_move(payload));
  return ResOk;
}
*/
struct Topic
{
  const char *delimiter = "/";
  const char *src_dst = "";
  const char *device = "";
  const char *component = "";
  const char *message_type = "";
  const char *serialization = "JSON";
  bool deserialize(const char *topic_arg)
  {
    char topic[64];
    strncpy(topic, topic_arg, sizeof(topic));
    char *token = strtok(topic, delimiter);
    int cnt = 0;
    while (token != NULL)
    {
      if (cnt == 0)
        src_dst = token;
      if (cnt == 1)
        device = token;
      if (cnt == 2)
        component = token;
      if (cnt == 3)
        message_type = token;
      if (cnt == 4)
        serialization = token;
      cnt++;
      token = strtok(NULL, delimiter);
    }
    return cnt < 4;
  }
};

void ZenohActor::subscription_handler(z_loaned_sample_t *sample, void *arg)
{
  ZenohActor *actor = (ZenohActor *)arg;

  z_view_string_t key_str;
  z_keyexpr_as_view_string(z_sample_keyexpr(sample), &key_str);
  const char *key = (const char *)key_str._val._slice.start;
  int length = key_str._val._slice.len;
  std::string topic = std::string(key, length);

  const z_loaned_bytes_t *payload = z_sample_payload(sample);
  z_bytes_reader_t reader = z_bytes_get_reader(payload);
  Bytes buffer;
  size_t len = z_bytes_len(payload);
  buffer.resize(len);
  _z_bytes_reader_read(&reader, buffer.data(), len);

  INFO("Received message on topic '%s' (%d bytes)", topic.c_str(), len);
  Topic t;
  t.deserialize(topic.c_str());

  if (strcmp(t.message_type, SysCmd::id) == 0)
  {
    auto msg_opt = SysCmd().deserialize(buffer);
    if ( msg_opt ) actor->emit(msg_opt);
    else ERROR("Failed to deserialize SysCmd message");
  } 
  else
    actor->emit(new ZenohReceived(topic, buffer));
}

Res ZenohActor::zenoh_publish(const char *topic, const Bytes &value)
{
  INFO("Publishing message on topic '%s' (%d bytes)", topic, value.size());
  z_view_keyexpr_t keyexpr;
  z_owned_bytes_t payload;
  z_view_keyexpr_from_str(&keyexpr, topic);

  CHECK(z_bytes_copy_from_buf(&payload, value.data(), value.size()));
  CHECK(z_put(z_loan(_zenoh_session), z_loan(keyexpr), z_move(payload), &put_options));
  z_drop(z_move(payload));
  emit(new LedPulse(10));

  return ResOk;
}

Res ZenohActor::publish_props()
{
  if (!_connected)
  {
    return Res(ENOTCONN, "Not connected to Zenoh");
  }
  ZenohInfo *zenoh_info = new ZenohInfo();
  zenoh_info->zid = zid;
  zenoh_info->what_am_i = MODE;
  zenoh_info->peers = _peers;
  zenoh_info->routers = _routers;
  zenoh_info->connect = CONNECT;
  emit(zenoh_info);
  return ResOk;
}

//============================================================

void ZenohActor::on_start()
{
  INFO("Starting Zenoh Actor '%s'", name());
}

void ZenohActor::on_stop()
{
  INFO("Stopping Zenoh Actor '%s'", name());
}

void ZenohActor::zenoh_unsubscribe(const std::string &topic)
{
  auto it = _subscribers.find(topic);
  if (it != _subscribers.end())
  {
    z_drop(z_move(it->second));
    delete_subscriber(it->second);
    _subscribers.erase(it);
    INFO("Unsubscribed from topic '%s'", topic.c_str());
  }
  else
  {
    WARN("No subscription found for topic '%s'", topic.c_str());
  }
}
