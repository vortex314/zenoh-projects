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

ZenohActor::ZenohActor() : cmds(10), ser(1024), des(1024) {
  //   cmds = Channel<ZenohCmd>(10);
  timers.add_timer(Timer::Repetitive(1, 10));
  timers.add_timer(Timer::Repetitive(2, 2000));
  prefix("device");
}

void ZenohActor::run() {
  ZenohCmd cmd;
  while (true) {
    cmds.receive(cmd, timers.sleep_time());
    for (int id : timers.get_expired_timers()) {
      switch (id) {
      case 1:
        if (_connected)
          publish_slow();
        break;
      case 2:
        printf("Timer 2 expired Zenoh Actor\n");
        break;
      default:
        printf("Unknown timer expired\n");
      }
      // execute timer
      timers.update();
    }
    if (cmd.action) {
      switch (cmd.action.value()) {
      case ZenohAction::Connect:
        if (!_connected) {
          Res res = connect();
          if (res.is_err()) {
            printf("Failed to connect to Zenoh: %s\n", res.msg().c_str());
          } else {
            printf("Connectied to Zenoh...\n");
          }
        }
        break;
      case ZenohAction::Disconnect:
        if (_connected) {
          disconnect();
          printf("Disconnecting from Zenoh...\n");
        }
        break;
      case ZenohAction::Stop:
        printf("Stopping ZenohActor...\n");
        return;
      }
    }
    if (cmd.publish_serialized && _connected) {
      zenoh_publish_serializable(cmd.publish_serialized.value().topic.c_str(),
                                 cmd.publish_serialized.value().value);
    }
    if (cmd.publish_binary && _connected) {
      zenoh_publish_binary(cmd.publish_binary.value().topic.c_str(),
                           cmd.publish_binary.value().value);
    }
  }
}

void ZenohActor::emit(ZenohEvent event) {
  for (auto handler : handlers) {
    handler(event);
  }
}

Res ZenohActor::connect(void) {
  // Initialize Zenoh Session and other parameters
  z_owned_config_t config;
  z_config_default(&config);
  RET_ERRI(zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE),
           "Failed to insert mode");
  if (strcmp(CONNECT, "") != 0) {
    //        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY,
    //        CONNECT);
    RET_ERRI(zp_config_insert(z_loan_mut(config), Z_CONFIG_LISTEN_KEY, CONNECT),
             "Failed to insert connect");
  }

  // Open Zenoh session
  printf("Opening Zenoh Session...");
  RET_ERRI(z_open(&zenoh_session, z_move(config), NULL),
           "Unable to open session");
  printf("Zenoh session opened \n");

  // Start the receive and the session lease loop for zenoh-pico
  RET_ERRI(zp_start_read_task(z_loan_mut(zenoh_session), NULL),
           "Failed to start read task");
  RET_ERRI(zp_start_lease_task(z_loan_mut(zenoh_session), NULL),
           "Failed to start lease task");
  _connected = true;
  return Res::Ok();
}

Res ZenohActor::disconnect() {
  printf("Closing Zenoh Session...");
  //  z_drop(z_move(zenoh_session));
  printf("Zenoh session closed \n");
  return Res::Ok();
}

void ZenohActor::prefix(const char *prefix) {
  _src_prefix = "src/";
  _src_prefix += prefix;
  _dst_prefix = "dst/";
  _dst_prefix += prefix;
}

Result<z_owned_subscriber_t> ZenohActor::declare_subscriber(const char *topic) {
  z_owned_subscriber_t sub;
  z_view_keyexpr_t ke;
  z_subscriber_options_t opts;
  z_subscriber_options_default(&opts);
  z_view_keyexpr_from_str_unchecked(&ke, topic);
  printf("Declaring subscriber for '%s'...", topic);
  z_owned_closure_sample_t callback;

  z_closure(&callback, ZenohActor::data_handler, NULL, this);

  TEST_RC(z_owned_subscriber_t,
          z_declare_subscriber(z_loan(zenoh_session), &sub, z_loan(ke),
                               z_move(callback), &opts),
          "Unable to declare subscriber for key expression");
  return Result<z_owned_subscriber_t>::Ok(sub);
}

void ZenohActor::delete_subscriber(z_owned_subscriber_t sub) {
  printf("Deleting subscriber...");
  // z_drop(z_loan(sub));
  printf("Subscriber deleted\n");
}

ZenohActor::~ZenohActor() {
  printf("Closing Zenoh Session...");
  zp_stop_read_task(z_loan_mut(zenoh_session));
  zp_stop_lease_task(z_loan_mut(zenoh_session));
  z_drop(z_move(zenoh_session));
  printf("Zenoh session closed \n");
}

Result<z_owned_publisher_t> ZenohActor::declare_publisher(const char *topic) {
  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  z_publisher_options_t opts;
  z_owned_encoding_t encoding;
  z_publisher_options_default(&opts);
  z_encoding_from_str(&encoding, "application/cbor"); // encoding type 8
  opts.encoding = z_move(encoding);
  z_view_keyexpr_from_str_unchecked(&ke, topic);
  printf("Declaring publisher for '%s'...", topic);
  if (z_declare_publisher(z_loan(zenoh_session), &pub, z_loan(ke), &opts) < 0) {
    printf("Unable to declare publisher for key expression!\n");
    return Result<z_owned_publisher_t>::Err(
        -1, "Unable to declare publisher for key expression");
  }
  printf("OK\n");
  return Result<z_owned_publisher_t>::Ok(pub);
}

Res ZenohActor::zenoh_publish_serializable(const char *topic,
                                           Serializable &value) {
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

void ZenohActor::data_handler(z_loaned_sample_t *sample, void *arg) {
  ZenohActor *actor = (ZenohActor *)arg;
  z_view_string_t keystr;
  z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
  z_owned_string_t value;
  z_bytes_to_string(z_sample_payload(sample), &value);
  actor->emit(ZenohEvent{
      ZenohBinary{std::string(z_string_data(z_view_string_loan(&keystr)),
                              z_string_len(z_view_string_loan(&keystr))),
                  Bytes{z_string_data(z_string_loan(&value)),
                        z_string_data(z_string_loan(&value)) +
                            z_string_len(z_string_loan(&value))}}});
  printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n",
         (int)z_string_len(z_view_string_loan(&keystr)),
         z_string_data(z_view_string_loan(&keystr)),
         (int)z_string_len(z_string_loan(&value)),
         z_string_data(z_string_loan(&value)));
  z_string_drop(z_string_move(&value));
}

#include <msg_sys.h>
#include <msg_wifi.h>

SysMsg sys_msg;
WifiMsg wifi_msg;
const char *src_motor = "motor"; // motor control
const char *src_sys = "sys";     // wifi, time, etc
const char *src_wifi = "wifi";   // wifi, time, etc

Res ZenohActor::publish_slow() {
  sys_msg.fill();
  RET_ERR(publish_topic_value(src_sys, sys_msg), "Failed to publish sys msg");
  wifi_msg.fill();
  RET_ERR(publish_topic_value(src_wifi, wifi_msg),
          "Failed to publish wifi msg");
  return Res::Ok();
}

Res ZenohActor::publish_info() {
  static int idx = 0;
  const InfoProp *prop = sys_msg.info(idx);
  if (prop == NULL) {
    idx = 0;
    prop = sys_msg.info(idx);
  }
  publish_topic_value("info/sys", *(InfoProp *)prop);

  static int idx_wifi = 0;
  const InfoProp *prop_wifi = wifi_msg.info(idx_wifi);
  if (prop_wifi == NULL) {
    idx_wifi = 0;
    prop_wifi = wifi_msg.info(idx_wifi);
  }
  publish_topic_value("info/wifi", *(InfoProp *)prop_wifi);
  return Res::Ok();
}

Res ZenohActor::zenoh_publish_binary(const char *topic, const Bytes &value) {
  std::string topic_name = _src_prefix;
  topic_name += "/";
  topic_name += topic;
  z_view_keyexpr_t keyexpr;
  z_owned_bytes_t payload;
  z_view_keyexpr_from_str(&keyexpr, topic_name.c_str());
  RET_ERRI(z_bytes_copy_from_buf(&payload, value.data(), value.size()),
           "Failed to copy buffer to payload");
  RET_ERRI(z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL),
           "Failed to publish message");
  z_drop(z_move(payload));
  return Res::Ok();
}

Res ZenohActor::publish_topic_value(const char *topic, Serializable &value) {
  RET_ERR(ser.reset(), "Failed to reset serializer");
  Bytes buffer;
  RET_ERR(ser.serialize(value), "Failed to serialize");
  RET_ERR(ser.get_bytes(buffer), "Failed to get bytes");
  std::string topic_name = _src_prefix;
  topic_name += "/";
  topic_name += topic;

  return zenoh_publish_binary(topic_name.c_str(), buffer);
}