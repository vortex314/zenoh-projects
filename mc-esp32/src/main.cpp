// #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-variable"

#include <nvs_flash.h>
#include <optional>
#include <string>
#include <vector>
#include <actor.h>
#include <wifi_actor.h>
#include <mc_actor.h>
#include <sys_actor.h>
#include <led_actor.h>
#include <log.h>

#include <esp_wifi.h>
#include <esp_coexist.h>
#include <esp_event.h>

#define DEVICE_NAME "esp1"
#define DEVICE_PREFIX DEVICE_NAME "/"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

WifiActor wifi_actor(DEVICE_PREFIX "wifi");
McActor mc_actor(DEVICE_PREFIX "multicast");
SysActor sys_actor(DEVICE_PREFIX "sys");
LedActor led_actor(DEVICE_PREFIX "led");
EventBus eventbus(10);
Log logger;
esp_err_t nvs_init();

#include <value.h>

extern "C" void app_main()
{

  ESP_ERROR_CHECK(nvs_init());

  INFO("Free heap size: %ld ", esp_get_free_heap_size());
  INFO("Stack high water mark: %ld \n", uxTaskGetStackHighWaterMark(NULL));

  eventbus.register_actor(&wifi_actor);
  eventbus.register_actor(&sys_actor);
  eventbus.register_actor(&mc_actor);
  eventbus.register_actor(&led_actor);
  eventbus.register_handler([](const Envelope &env)
                            { 
                              const char* src = env.src ? env.src->name() : "";
                              const char* dst = env.dst ? env.dst->name() : "";
                              INFO(" %ld Event '%s' => '%s' : %s", esp_get_free_heap_size(), src, dst, env.msg->type_id()); });
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
