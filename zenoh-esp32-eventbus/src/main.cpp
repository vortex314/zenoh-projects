// #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"

#include <nvs_flash.h>
#include <optional>
#include <string>
#include <vector>
#include <actor.h>
#include <wifi_actor.h>
#include <sys_actor.h>
#include <led_actor.h>
#include <zenoh_actor.h>
#include <hoverboard_actor.h>
#include <log.h>

#include <esp_wifi.h>
#include <esp_coexist.h>
#include <esp_event.h>

#define DEVICE_PREFIX DEVICE_NAME "/"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

WifiActor wifi_actor("wifi");
ZenohActor zenoh_actor("zenoh");
SysActor sys_actor("sys");
LedActor led_actor("led");
HoverboardActor hoverboard_actor("hoverboard");
EventBus eventbus(20);
Log logger;
esp_err_t nvs_init();

extern "C" void app_main()
{

  ESP_ERROR_CHECK(nvs_init());

  INFO("Free heap size: %ld ", esp_get_free_heap_size());
  INFO("Stack high water mark: %ld \n", uxTaskGetStackHighWaterMark(NULL));

  zenoh_actor.prefix(DEVICE_NAME);

  eventbus.register_actor(&wifi_actor); // manage wifi connection
  eventbus.register_actor(&sys_actor); // manage the system
  //  eventbus.register_actor(&mc_actor);
  eventbus.register_actor(&zenoh_actor); // bridge the eventbus
  eventbus.register_actor(&led_actor); // blink the led
  eventbus.register_actor(&hoverboard_actor); // exchange data via serial with hoverboard via UART
  eventbus.register_handler([](const Envelope &env) // just log eventbus traffic
                            {
                              const char *src = env.src ? env.src->name() : "";
                              const char *dst = env.dst ? env.dst->name() : "";
                              INFO(" %ld Event '%s' => '%s' : %s", esp_get_free_heap_size(), src, dst, env.msg->type_name()); // comment for beauty
                            });
  eventbus.loop();
}

esp_err_t nvs_init()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  return ret;
}
