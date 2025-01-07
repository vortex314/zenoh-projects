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

ZenohActor::ZenohActor() : Actor<ZenohEvent, ZenohCmd>(4000, "zenoh", 5, 6),
                           _ser(1024),_des(1024)
{
  INFO("Starting WiFi actor sizeof(ZenohCmd ) : %d ", sizeof(ZenohCmd));
  add_timer(Timer::Repetitive(1, 1000));
  add_timer(Timer::Repetitive(2, 2000));
  prefix("device");
}

void ZenohActor::on_timer(int id)
{
  switch (id)
  {
  case 1:
    break;
  case 2: {
    INFO("Timer 2 expired Zenoh Actor");
    _zenoh_msg.fill(z_loan_mut(_zenoh_session));
    break;
  }
  default:
    INFO("Unknown timer expired");
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
    zenoh_publish(cmd.publish.value().topic.c_str(), cmd.publish.value().payload);
  }
}

Res ZenohActor::connect(void)
{
  // Initialize Zenoh Session and other parameters
  z_owned_config_t config;
  z_config_default(&config);
  RET_ERRI(zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE),
           "Failed to insert mode");
  if (strcmp(CONNECT, "") != 0)
  {
    //        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY,
    //        CONNECT);
    RET_ERRI(zp_config_insert(z_loan_mut(config), Z_CONFIG_LISTEN_KEY, CONNECT),
             "Failed to insert connect");
  }
  // Open Zenoh session
  RET_ERRI(z_open(&_zenoh_session, z_move(config), NULL),
           "Unable to open session");
  // Start the receive and the session lease loop for zenoh-pico
  RET_ERRI(zp_start_read_task(z_loan_mut(_zenoh_session), NULL),
           "Failed to start read task");
  RET_ERRI(zp_start_lease_task(z_loan_mut(_zenoh_session), NULL),
           "Failed to start lease task");
  _connected = true;
  return Res::Ok();
}

Res ZenohActor::disconnect()
{
  INFO("Closing Zenoh Session...");
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
  RET_ERRI(z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size()),
           "Failed to copy buffer to payload");
  RET_ERRI(z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL),
           "Failed to publish message");
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
  RET_ERRI(z_bytes_copy_from_buf(&payload, value.data(), value.size()),
           "Failed to copy buffer to payload");
  RET_ERRI(z_put(z_loan(_zenoh_session), z_loan(keyexpr), z_move(payload), NULL),
           "Failed to publish message");
  z_drop(z_move(payload));
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
  ser.serialize(idx++, zid);
  ser.serialize(idx++, what_am_i);
  ser.serialize(idx++, peers);
  ser.serialize(idx++, prefix);
  ser.serialize(idx++, routers);
  return ser.serialize(idx++, connect);
}

Res ZenohMsg::deserialize(Deserializer &des)
{
  return Res::Ok();
}

Res ZenohMsg::fill(z_loaned_session_t *session)
{
  z_id_t _zid = z_info_zid(session);
  z_owned_string_t z_str;
  ;
  z_id_to_string(&_zid, &z_str);
  zid = std::string(z_str._val._slice.start, z_str._val._slice.start + z_str._val._slice.len);
  /*
    z_owned_string_t what_am_i_str;
    z_info_what_am_i(session, &what_am_i_str);
    what_am_i = std::string(what_am_i_str._val._slice.start, what_am_i_str._val._slice.start + what_am_i_str._val._slice.len);
  */
  return Res::Ok();
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