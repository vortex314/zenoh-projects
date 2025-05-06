#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <nvs_flash.h>
#include <optional>
#include <string>
#include <vector>
#include <actor.h>
#include <wifi_actor.h>
#include <zenoh_actor.h>
#include <sys_actor.h>
#include <led_actor.h>
#include <ota_actor.h>
#include <log.h>

#include <esp_wifi.h>
#include <esp_coexist.h>
#include <esp_event.h>

#define DEVICE_NAME "esp3"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

// threads can be run separately or share a thread
// Pinning all on CPU0 to avoid Bluetooth crash in rwbt.c line 360.
WifiActor wifi_actor("wifi", 9000, 40, 5);
ZenohActor zenoh_actor("zenoh", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
OtaActor ota_actor("ota", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 23, Cpu::CPU0);

Log logger;

void zenoh_publish(const char *topic, Option<PublishSerdes> &serdes);
void publish(const char *topic, const Serializable &serializable)
{
  Bytes buffer;
  CborSerializer ser(buffer);
  serializable.serialize(ser);
  zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
  // pulse led when we publish
  led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
}

template <typename U>
std::function<Option<U>(const PublishBytes)> cbor_to = [](const PublishBytes pub) -> Option<U>
{ return cbor_deserialize<U>(pub.payload); };

template <typename T>
std::function<void(const T &)> z_publish = [](const T &msg)
{ publish(SRC_DEVICE "zenoh", msg); };

/*
| WIFI | = connect/disconnect => | ZENOH | ( set up session )
| SYS | = system events => | ZENOH | ( publish )
| ZENOH | = zenoh events => | ZENOH | ( publish )
| ZENOH | = publish events => | LED | (pulse)
*/

extern "C" void app_main()
{

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  esp_wifi_set_ps(WIFI_PS_NONE);
  // esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);

  zenoh_actor.prefix("esp3"); // set the zenoh prefix to src/esp3 and destination subscriber dst/esp3/**

  // WIRING the actors together
  // WiFi connectivity starts and stops zenoh connection

  wifi_actor.on_event([](const WifiEvent &event)
                      {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Disconnect});
    } });

  // publish data from actors
  wifi_actor.on_event([&](const WifiEvent &event)
                      { if ( event.msg ){ publish(SRC_DEVICE "wifi", event.msg.value()); }; });
  sys_actor.on_event([&](SysEvent event)
                     { event.msg >> [](auto msg)
                       { publish(SRC_DEVICE "sys", msg); }; });
  zenoh_actor.on_event([&](ZenohEvent event)
                       { event.msg >> [](auto msg)
                         { publish(SRC_DEVICE "zenoh", msg); }; });
  ota_actor.on_event([&](OtaEvent event)
                     { event.msg >> [](auto msg)
                       { publish(SRC_DEVICE "ota", msg); }; });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
    if (event.publish)
    {
      if (event.publish->topic == DST_DEVICE "sys")
      {
        cbor_deserialize<SysMsg>(event.publish->payload) >> [&](SysMsg &msg)
        { sys_actor.tell(new SysCmd{.msg = msg}); };
      }
      else if (event.publish->topic == DST_DEVICE "wifi")
      {
        cbor_deserialize<WifiMsg>(event.publish->payload) >> [&](WifiMsg &msg)
        { wifi_actor.tell(new WifiCmd{.msg = msg}); };
      }
      else if (event.publish->topic == DST_DEVICE "zenoh")
      {
        cbor_deserialize<ZenohMsg>(event.publish->payload) >> [&](ZenohMsg &msg)
        { zenoh_actor.tell(new ZenohCmd{.msg = msg}); };
      }
      else if (event.publish->topic == DST_DEVICE "ota")
      {
        cbor_deserialize<OtaMsg>(event.publish->payload) >> [&](OtaMsg msg)
        { ota_actor.tell(new OtaCmd{.msg = std::move(msg)}); };
      }
      else
      {
        INFO("Received Zenoh unknown event");
      }
    } });

  // actor_thread.add_actor(camera_actor);
  actor_thread.add_actor(wifi_actor);
  actor_thread.add_actor(zenoh_actor);
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

// re-entrant function to publish a serializable object
//
void zenoh_publish(const char *topic, Option<PublishSerdes> &serdes)
{
  if (serdes)
  {
    Bytes buffer;
    CborSerializer ser(buffer);
    auto &serializable = serdes.value().payload;
    serializable.serialize(ser);

    if (serdes.value().topic)
    {
      topic = serdes.value().topic.value().c_str();
    };

    zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
    // pulse led when we publish
    led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
  }
}
