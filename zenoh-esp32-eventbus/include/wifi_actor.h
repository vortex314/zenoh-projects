#ifndef _WIFI_ACTOR_H_
#define _WIFI_ACTOR_H_
#include <actor.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <functional>
#include <vector>
#include <limero.h>


DEFINE_MSG(WifiConnected);
DEFINE_MSG(WifiDisconnected);

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
  WifiActor(const char *name);
  ~WifiActor();
  void on_message(const Envelope &);
  void on_start();
  void handle_timer(int id);
  Res net_init();
  Res wifi_set_config(const char *ssid, const char *password);
  void publish_info(esp_netif_t *esp_netif);

  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);
  Res scan();
  Res connect();
};

std::string ip4addr_to_str(esp_ip4_addr_t *ip);

#endif
