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

// threads can be run separately or share a thread
// Pinning all on CPU0 to avoid Bluetooth crash in rwbt.c line 360.
WifiActor wifi_actor("wifi");
McActor mc_actor("multicast");
SysActor sys_actor("sys");
LedActor led_actor("Led");
Thread actor_thread("actors", 9000, 40, 24, Cpu::CPU0);
Thread mc_thread("mc", 9000, 40, 23, Cpu::CPU_ANY);
EventBus eventbus(10);

Log logger;

// void zenoh_publish(const char *topic, Option<PublishSerdes> &serdes);
/*void publish(const char *topic, const Value &value)
{
  if (!mc_actor.is_connected())
  {
    INFO("Mc not connected, cannot publish");
    return;
  }
  Value mc_cmd;
  mc_cmd["publish_string"] = std::move(value);
  mc_cmd["publish_string"]["src"] = topic;
  mc_actor.ref().tell(new PublishMsg{std::string(topic), mc_cmd});
  // pulse led when we publish
  Value led_cmd;
  led_cmd["action"] = "PULSE";
  led_cmd["duration"] = 10;
  led_actor.tell(led_cmd);
}*/
esp_err_t nvs_init();
/*
| WIFI | = connect/disconnect => | ZENOH | ( set up session )
| SYS | = system events => | ZENOH | ( publish )
| ZENOH | = zenoh events => | ZENOH | ( publish )
| ZENOH | = publish events => | LED | (pulse)
*/
#include <value.h>

extern "C" void app_main()
{

  ESP_ERROR_CHECK(nvs_init());
  //  mc_actor.prefix(DEVICE_NAME); // set the zenoh prefix to src/esp3 and destination subscriber dst/esp3/**
  printf("Starting %ld  sizeof Value %d \n", esp_get_free_heap_size(), sizeof(Value));
  uxTaskGetStackHighWaterMark(NULL); // get the stack high water mark of the main task
  INFO("Free heap size: %ld ", esp_get_free_heap_size());
  INFO("Stack high water mark: %ld \n", uxTaskGetStackHighWaterMark(NULL));


  eventbus.register_actor(&wifi_actor);
  eventbus.register_actor(&sys_actor);
  eventbus.register_actor(&mc_actor);
  eventbus.register_actor(&led_actor);
  eventbus.register_message_handler([](const Msg& msg){
    INFO(" %ld Event '%s' => '%s' : %s", esp_get_free_heap_size(),msg.src.name(), msg.dst.name(), msg.type_id());
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
