#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <functional>
#include <vector>

std::string ip4addr_to_str(esp_ip4_addr_t *ip);

class WifiActor : public Actor
{
private:
  esp_netif_t *esp_netif;
  esp_event_handler_instance_t handler_any_id;
  esp_event_handler_instance_t handler_got_ip;
  bool _wifi_connected = false;
  int _timer_publish;
  int _timer_publish_props;
  int _prop_counter = 0;
  std::string wifi_ssid;
  std::string wifi_password;
  int channel;
  std::vector<std::string> ssid_list;

public:
  WifiActor();
  WifiActor(const char *name, size_t stack_size, int priority, size_t queue_depth);
  ~WifiActor();
  void on_cmd(SharedValue cmd);
  void on_timer(int timer_id);
  void on_start();
  Res net_init();
  Res wifi_set_config(const char *ssid, const char *password);
  Result<Value> pubish_props(esp_netif_t *esp_netif);
  Result<Value> publish_info();

  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);
  Res scan();
  Res connect();
};
