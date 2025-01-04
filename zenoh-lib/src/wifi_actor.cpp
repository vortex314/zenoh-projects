#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <strings.h>
#include <wifi_actor.h>

static int s_retry_count = 0;
#define STRINGIFY(X) #X
#define S(X) STRINGIFY(X)
#define ESP_MAXIMUM_RETRY 5

WifiActor::WifiActor() : cmds(10) {
  //   cmds = Channel<WifiCmd>(10);
  timers.add_timer(Timer::Repetitive(1, 1000));
  timers.add_timer(Timer::Repetitive(2, 2000));
}

void WifiActor::run() {
  wifi_init_sta();
  WifiCmd cmd;
  while (true) {
    cmds.receive(cmd, timers.sleep_time());
    for (int id : timers.get_expired_timers()) {
      switch (id) {
      case 1:
        printf("Timer 1 expired wifi\n");
        break;
      case 2:
        printf("Timer 2 expired wifi \n");
        break;
      default:
        printf("Unknown timer expired wifi\n");
      }
      // execute timer
      timers.update();
    }
    if (cmd.stop_actor) {
      break;
    }
  }
}

void WifiActor::emit(WifiEvent event) {
  for (auto handler : handlers) {
    handler(event);
  }
}

void WifiActor::event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
  WifiActor *actor = (WifiActor *)arg;
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    actor->emit(WifiEvent{.signal = WifiSignal::WIFI_DISCONNECTED});
    actor->_wifi_connected = false;
    if (s_retry_count < ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_count++;
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    actor->emit(WifiEvent{.signal = WifiSignal::WIFI_CONNECTED});
    actor->_wifi_connected = true;
    s_retry_count = 0;
  }
}

void WifiActor::wifi_init_sta(void) {

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &handler_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, &handler_got_ip));

  wifi_config_t wifi_config;
  bzero(&wifi_config, sizeof(wifi_config));
  strcpy((char *)wifi_config.sta.ssid, "Merckx4");
  strcpy((char *)wifi_config.sta.password, S(WIFI_PASS));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
