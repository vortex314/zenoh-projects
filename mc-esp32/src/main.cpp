#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <nvs_flash.h>
#include <optional>
#include <string>
#include <vector>
#include <actor.h>
#include <wifi_actor.h>
#include <mc_actor.h>
#include <sys_actor.h>
#include <led_actor.h>
#include <ota_actor.h>
#include <log.h>

#include <esp_wifi.h>
#include <esp_coexist.h>
#include <esp_event.h>

#define DEVICE_NAME "mtr1"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

// threads can be run separately or share a thread
// Pinning all on CPU0 to avoid Bluetooth crash in rwbt.c line 360.
WifiActor wifi_actor("wifi", 9000, 40, 5);
McActor mc_actor("zenoh", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
OtaActor ota_actor("ota", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 23, Cpu::CPU0);
Th

Log logger;

// void zenoh_publish(const char *topic, Option<PublishSerdes> &serdes);
void publish(const char *topic, const Serializable &serializable)
{
  if (mc_actor.is_connected() == false)
  {
    INFO("Mc not connected, cannot publish");
    return;
  }
  Bytes buffer;
  JsonSerializer ser(buffer);
  serializable.serialize(ser);
  mc_actor.tell(new McCmd{.publish_bytes = PublishBytes{topic, buffer}});
  // pulse led when we publish
  led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
}
esp_err_t nvs_init();
/*
| WIFI | = connect/disconnect => | ZENOH | ( set up session )
| SYS | = system events => | ZENOH | ( publish )
| ZENOH | = zenoh events => | ZENOH | ( publish )
| ZENOH | = publish events => | LED | (pulse)
*/
extern "C" void app_main()
{
  ESP_ERROR_CHECK(nvs_init());
  mc_actor.prefix(DEVICE_NAME); // set the zenoh prefix to src/esp3 and destination subscriber dst/esp3/**

  /*mc_actor.tell(new McCmd{
      .action = McAction::Subscribe,
      .topic = Option<std::string>("src/brain/ **")}); // connect to zenoh*/
  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.on_event([&](const WifiEvent &event)
                      {
                        event.signal.filter([&](WifiSignal sig){ return sig == WifiSignal::WIFI_CONNECTED;})
                          .and_then([](auto sig ){  return mc_actor.tell(new McCmd{.action = McAction::Connect});});
                        event.signal.filter([&](WifiSignal sig){ return sig == WifiSignal::WIFI_DISCONNECTED;})
                          .and_then([](auto sig ){  return mc_actor.tell(new McCmd{.action = McAction::Disconnect});}); });
  // WIRING the actors together
  wifi_actor.on_event([&](const WifiEvent &event)
                      { event.publish.for_each([](auto msg)
                                               { publish(SRC_DEVICE "wifi", msg); }); });
  sys_actor.on_event([&](SysEvent event)
                     { event.publish >> [](auto msg)
                       { publish(SRC_DEVICE "sys", msg); }; });
  mc_actor.on_event([&](McEvent event)
                       { event.publish >> [](auto msg)
                         { publish(SRC_DEVICE "zenoh", msg); }; });
  ota_actor.on_event([&](OtaEvent event)
                     { event.publish >> [](auto msg)
                       { publish(SRC_DEVICE "ota", msg); }; });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  mc_actor.on_event([&](McEvent event)
                       {
                        INFO("Mc event received %s %s", event.publish_bytes ? "publish_bytes":"-" , event.publish ? "publish" : "-" );
                        event.publish_bytes
                          .filter([&](auto pb){ return pb.topic == DST_DEVICE "sys" ;})
                          .inspect([&](auto pb){ INFO("Sys msg received %s", pb.topic.c_str() ) ;})
                          .and_then([&](auto pb ){ return json_deserialize<SysMsg>(pb.payload);})
                          .for_each([&](auto msg){ sys_actor.tell(new SysCmd{.publish = msg}); });
                        event.publish_bytes
                          .filter([&](auto pb){ return pb.topic == DST_DEVICE "wifi" ;})
                          .and_then([&](auto pb ){ return json_deserialize<WifiMsg>(pb.payload);})
                          .for_each([&](auto msg){ wifi_actor.tell(new WifiCmd{.publish = msg}); });
                        event.publish_bytes
                          .filter([&](auto pb){ return pb.topic == DST_DEVICE "zenoh" ;})
                          .and_then([&](auto pb ){ return json_deserialize<McMsg>(pb.payload);})
                          .for_each([&](auto msg){ mc_actor.tell(new McCmd{.publish = msg}); });
                        event.publish_bytes
                          .filter([&](auto pb){ return pb.topic == DST_DEVICE "ota" ;})
                          .and_then([&](auto pb ){ return json_deserialize<OtaMsg>(pb.payload);})
                          .for_each([&](auto msg){ ota_actor.tell(new OtaCmd{.publish = msg}); }); });

  // actor_thread.add_actor(camera_actor);
  actor_thread.add_actor(wifi_actor);
  actor_thread.add_actor(mc_actor);
  actor_thread.add_actor(sys_actor);
  actor_thread.add_actor(led_actor);
  actor_thread.add_actor(ota_actor);
  actor_thread.start();

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
