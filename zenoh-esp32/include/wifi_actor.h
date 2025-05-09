#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include <vector>

std::string ip4addr_to_str(esp_ip4_addr_t *ip);

struct WifiMsg : public Serializable
{
  // WIFI & ethernet
  Option<std::string> mac_address = nullptr ;
  Option<std::string> ip_address = nullptr;
  Option<std::string> gateway = nullptr;
  Option<std::string> netmask = nullptr;
  Option<std::string> dns = nullptr;
  Option<std::string> ssid = nullptr;
  Option<uint8_t> channel = nullptr; // dynamic
  Option<int8_t> rssi = nullptr;
  Option<std::string> encryption = nullptr;
  Option<uint8_t> wifi_mode = nullptr;
  Option<std::string> ap_scan = nullptr;

  Res serialize(Serializer &ser) const;
  Res deserialize(Deserializer &des);
  Res fill(esp_netif_t *esp_netif);
//  const InfoProp *info(int idx);
};

constexpr uint32_t SIZE = sizeof(WifiMsg);

typedef enum
{
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
} WifiSignal;

struct WifiEvent
{
  Option<WifiSignal> signal = nullptr;
  Option<std::string> topic= nullptr;
  Option<WifiMsg> publish = nullptr;
};

struct WifiCmd
{
  Option<bool> stop_actor = nullptr;
  Option<WifiMsg> publish = nullptr;
};

class WifiActor : public Actor<WifiEvent, WifiCmd>
{
  int _timer_publish;
  int _timer_publish_props;
  int _prop_counter = 0;
public:
  WifiActor();
  WifiActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~WifiActor();
  void on_cmd(WifiCmd &cmd);
  void on_timer(int timer_id);
  void on_start();
  Res net_init();
  Res wifi_set_config(const char* ssid, const char* password);
//  Res publish_props_info();
  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);
  Res scan();
  Res connect();
  std::string wifi_ssid;
  std::string wifi_password;
  int channel;
  std::vector<std::string> ssid_list;

private:
  WifiMsg wifi_msg;
  esp_netif_t *esp_netif;
  esp_event_handler_instance_t handler_any_id;
  esp_event_handler_instance_t handler_got_ip;
  bool _wifi_connected = false;
};
