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
WifiActor wifi_actor("wifi", 9000, 40, 5);
McActor mc_actor("multicast", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 24, Cpu::CPU0);
Thread mc_thread("mc", 9000, 40, 23, Cpu::CPU_ANY);

Log logger;

// void zenoh_publish(const char *topic, Option<PublishSerdes> &serdes);
void publish(const char *topic, const Value &value)
{
  if (!mc_actor.is_connected())
  {
    INFO("Mc not connected, cannot publish");
    return;
  }
  Value mc_cmd;
  mc_cmd["publish_string"] = std::move(value);
  mc_cmd["publish_string"]["src"] = topic;
  mc_actor.tell(mc_cmd);
  // pulse led when we publish
  Value led_cmd;
  led_cmd["action"] = "PULSE";
  led_cmd["duration"] = 10;
  led_actor.tell(led_cmd);
}
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

  wifi_actor.ref().tell( new AddListenerMsg( mc_actor.ref() ) );


  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.on_event([&](const Value &event)
                      { event["connected"].handle<bool>([&](auto connected)
                                                        {
                          Value v;
                          v["wifi_connected"]=connected;
                          mc_actor.tell(v); }); });

  // WIRING the actors together
  wifi_actor.on_event([&](const Value &event)
                      { event["pub"].handle<Value::ObjectType>([&](auto v)
                                                                   { publish(DEVICE_PREFIX "wifi", event); }); });
  sys_actor.on_event([&](const Value &event)
                     { event["pub"].handle<Value::ObjectType>([&](auto v)
                                                                  { publish(DEVICE_PREFIX "sys", event); }); });
  mc_actor.on_event([&](const Value &event)
                    { event["pub"].handle<Value::ObjectType>([&](auto v)
                                                                 { publish(DEVICE_PREFIX "multicast", event); }); });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  mc_actor.on_event([&](const Value &v)
                    {
        if (v["pub"].is<Value::ObjectType>() && v["dst"].is<std::string>())
        {
          const std::string topic = v["dst"].as<std::string>();
          if (topic == DEVICE_PREFIX "sys")
            sys_actor.tell(v);
          else if (topic == DEVICE_PREFIX "wifi")
            wifi_actor.tell(v);
          else if (topic == DEVICE_PREFIX "multicast")
            mc_actor.tell(v);
        } });

  actor_thread.add_actor(wifi_actor);
  actor_thread.add_actor(sys_actor);
  actor_thread.add_actor(led_actor);
  actor_thread.start();
  mc_thread.add_actor(mc_actor);
  mc_thread.start();

  // log heap size, monitoring thread in main, we could exit also
  while (true)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    INFO(" free heap size: %lu biggest block : %lu ", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
  }
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
