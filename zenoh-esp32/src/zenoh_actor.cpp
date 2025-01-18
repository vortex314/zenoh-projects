#include <zenoh_actor.h>

#define CLIENT_OR_PEER 0 // 0: Client mode; 1: Peer mode
#if CLIENT_OR_PEER == 0
#define MODE "client"
#define CONNECT "" // If empty, it will scout
#elif CLIENT_OR_PEER == 1
#define MODE "peer"
#define CONNECT "udp/224.0.0.123:7447#iface=lo"
#else
#error "Unknown Zenoh operation mode. Check CLIENT_OR_PEER value."
#endif

ZenohActor::ZenohActor() : ZenohActor("zenoh", 4096, 5, 5) {}

ZenohActor::ZenohActor(const char *name, size_t stack_size, int priority, size_t queue_depth)
    : Actor<ZenohEvent, ZenohCmd>(stack_size, name, priority, queue_depth)
{
  _timer_publish = timer_repetitive(1000); // timer for publishing properties
  prefix("device");                        // default prefix
}

void ZenohActor::on_timer(int id)
{
  if (id == _timer_publish)
  {
    INFO("Timer publish : Publishing Zenoh properties");
    publish_props();
  } else {
    INFO("Unknown timer id: %d", id);
  }
}

void ZenohActor::on_cmd(ZenohCmd &cmd)
{
  if (cmd.action)
  {
    switch (cmd.action.value())
    {
    case ZenohAction::Connect:
      if (!_connected)
      {
        Res res = connect();
        if (res.is_err())
        {
          INFO("Failed to connect to Zenoh: %s", res.msg().c_str());
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          tell(new ZenohCmd{.action = ZenohAction::Connect});
        }
        else
        {
          INFO("Connected to Zenoh.");
          std::string topic = _dst_prefix;
          topic += "/**";
          auto sub = declare_subscriber(topic.c_str());
          if (sub.is_err())
          {
            INFO("Failed to declare subscriber: %s", sub.msg().c_str());
          }
          else
          {
            _subscribers.emplace(topic, sub.value());
          }
        }
      }
      break;
    case ZenohAction::Disconnect:
      if (_connected)
      {
        disconnect();
        INFO("Disconnecting from Zenoh...");
      }
      break;
    case ZenohAction::Stop:
      INFO("Stopping ZenohActor...");
      return;
    }
  }
  if (cmd.publish && _connected)
  {
    if (zenoh_publish(cmd.publish.value().topic.c_str(), cmd.publish.value().payload).is_err())
    {
      INFO("Failed to publish message");
      //   disconnect();
      //   vTaskDelay(1000 / portTICK_PERIOD_MS);
      //   tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
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
  _connected = true;
  return Res::Ok();
}

Res ZenohActor::disconnect()
{
  INFO("Closing Zenoh Session...");
  if (z_session_is_closed(z_loan_mut(_zenoh_session)))
  {
    INFO("Zenoh session is not open");
    return Res::Ok();
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
  return Res::Ok();
}

void ZenohActor::prefix(const char *prefix)
{
  _src_prefix = "src/";
  _src_prefix += prefix;
  _dst_prefix = "dst/";
  _dst_prefix += prefix;
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
  INFO("OK");
  return Result<z_owned_subscriber_t>::Ok(sub);
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
}
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
  return Res::Ok();
}
*/
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

  actor->emit(ZenohEvent{.publish = PublishBytes{topic, std::move(buffer)}});
}

Res ZenohActor::zenoh_publish(const char *topic, const Bytes &value)
{
  std::string topic_name = _src_prefix;
  topic_name += "/";
  topic_name += topic;
  z_view_keyexpr_t keyexpr;
  z_owned_bytes_t payload;
  z_view_keyexpr_from_str(&keyexpr, topic_name.c_str());
  CHECK(z_bytes_copy_from_buf(&payload, value.data(), value.size()));
  CHECK(z_put(z_loan(_zenoh_session), z_loan(keyexpr), z_move(payload), NULL));
  z_drop(z_move(payload));
  return Res::Ok();
}

Res ZenohActor::publish_props()
{
  if (!_connected)
  {
    return Res::Err(ENOTCONN, "Not connected to Zenoh");
  }
  z_id_t _zid = z_info_zid(z_loan(_zenoh_session));
  z_owned_string_t z_str;
  z_id_to_string(&_zid, &z_str);
  _zenoh_msg.zid = std::string(z_str._val._slice.start, z_str._val._slice.start + z_str._val._slice.len);
  /*
    z_owned_string_t what_am_i_str;
    z_info_what_am_i(session, &what_am_i_str);
    what_am_i = std::string(what_am_i_str._val._slice.start, what_am_i_str._val._slice.start + what_am_i_str._val._slice.len);
  */
  emit(ZenohEvent{.serdes = PublishSerdes{.payload = _zenoh_msg}});
  z_drop(z_move(z_str));
  return Res::Ok();
}

//============================================================
//============================================================
//============================================================
//============================================================
//============================================================

Res ZenohMsg::serialize(Serializer &ser)
{
  int idx = 0;
  ser.reset();
  ser.map_begin();
  ser.serialize(idx++, zid);
  ser.serialize(idx++, what_am_i);
  ser.serialize(idx++, peers);
  ser.serialize(idx++, prefix);
  ser.serialize(idx++, routers);
  ser.serialize(idx++, connect);
  return ser.map_end();
}

Res ZenohMsg::deserialize(Deserializer &des)
{
  return Res::Err(EAFNOSUPPORT, "not implemented");
}

InfoProp info_props_zenoh[] = {
    InfoProp(0, "zid", "Zenoh ID", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "what_am_i", "What am I", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(2, "peers", "Peers", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(3, "prefix", "Prefix", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(4, "routers", "Routers", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(5, "connect", "Connect", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(6, "listen", "Listen", PropType::PROP_STR, PropMode::PROP_READ),
};