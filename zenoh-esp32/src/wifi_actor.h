#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <functional>
#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include <vector>

std::string ip4addr_to_str(esp_ip4_addr_t *ip);

/*

const static InfoProp info_props_wifi[11] = {
    InfoProp(0, "mac_address", "MAC Address", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "ip_address", "IP Address", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(2, "gateway", "Gateway", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(3, "netmask", "Netmask", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(4, "dns", "DNS", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(5, "ssid", "SSID", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(6, "channel", "Channel", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(7, "rssi", "RSSI", PropType::PROP_SINT, PropMode::PROP_READ),
    InfoProp(8, "encryption", "Encryption", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(9, "wifi_mode", "Wifi Mode", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(10, "ap_scan", "AP Scan", PropType::PROP_ARRAY, PropMode::PROP_READ),
};
*/

struct WifiMsg : public Serializable
{
  // WIFI & ethernet
  std::optional<std::string> mac_address = std::nullopt;
  std::optional<std::string> ip_address = std::nullopt;
  std::optional<std::string> gateway = std::nullopt;
  std::optional<std::string> netmask = std::nullopt;
  std::optional<std::string> dns = std::nullopt;
  std::optional<std::string> ssid = std::nullopt;
  std::optional<uint8_t> channel = std::nullopt; // dynamic
  std::optional<int8_t> rssi = std::nullopt;
  std::optional<std::string> encryption = std::nullopt;
  std::optional<uint8_t> wifi_mode = std::nullopt;
  std::optional<std::string> ap_scan = std::nullopt;

  Res serialize(Serializer &ser);
  Res deserialize(Deserializer &des);
  Res fill(esp_netif_t *esp_netif);
  const InfoProp *info(int idx);
};

typedef enum
{
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
} WifiSignal;

struct WifiEvent
{
  std::optional<WifiSignal> signal = std::nullopt;
  std::optional<PublishSerdes> serdes = std::nullopt;
};

struct WifiCmd
{
  std::optional<bool> stop_actor = std::nullopt;
  std::optional<PublishSerdes> serdes = std::nullopt;
};

class WifiActor : public Actor<WifiEvent, WifiCmd>
{
  int _timer_publish;
public:
  WifiActor();
  WifiActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~WifiActor();
  void on_cmd(WifiCmd &cmd);
  void on_timer(int timer_id);
  void on_start();
  Res net_init();
  Res wifi_init_sta(void);
  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);
  Res scan();
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
