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
#include <ps4_actor.h>
#include <led_actor.h>
#include <log.h>

#include <esp_wifi.h>
#include <esp_coexist.h>
#include <esp_event.h>
// threads can be run separately or share a thread
// Pinning all on CPU0 to avoid Bluetooth crash in rwbt.c line 360.
WifiActor wifi_actor("wifi", 9000, 40, 5);
ZenohActor zenoh_actor("zenoh", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 23, Cpu::CPU0);
// the btstack is a blocking task so it has its own thread
Ps4Actor ps4_actor("ps4", 9000, 40, 5);
Thread ps4_thread("bluetooth", 9000, 20, 5, Cpu::CPU0);

Log logger;

void zenoh_publish(const char *topic, std::optional<PublishSerdes> &serdes);
void log_ps4_event(Ps4Event event);

template <typename T>
std::optional<T> deserialize(Bytes bytes)
{
  CborDeserializer des(bytes.data(), bytes.size());
  T obj;
  if (obj.deserialize(des).is_ok())
    return obj;
  return std::nullopt;
}

/*
| WIFI | = connect/disconnect => | ZENOH | ( set up session )
| PS4 | = gamepad events => | ZENOH | ( publish )
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
  esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);

  zenoh_actor.prefix("lm1"); // set the zenoh prefix to src/lm1 and destination subscriber dst/lm1/ **

  // WIRING the actors together
  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.on_event([](WifiEvent event)
                      {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Disconnect});
    } });

  // publish data from actors
  wifi_actor.on_event([&](WifiEvent event)
                      { zenoh_publish("src/lm1/wifi", event.serdes);
                      zenoh_publish("info/lm1/wifi",event.prop_info); });
  sys_actor.on_event([&](SysEvent event)
                     { zenoh_publish("src/lm1/sys", event.serdes); 
                     zenoh_publish("info/lm1/sys",event.prop_info); });
  zenoh_actor.on_event([&](ZenohEvent event) // send to myself
                       { zenoh_publish("src/lm1/zenoh", event.serdes); 
                       zenoh_publish("info/lm1/zenoh",event.prop_info);
                       });
  ps4_actor.on_event([&](Ps4Event event)
                     { zenoh_publish("src/lm1/ps4", event.serdes); 
                     zenoh_publish("info/lm1/ps4",event.prop_info); });
  // log connection events
  ps4_actor.on_event([](Ps4Event event)
                     { log_ps4_event(event); });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
    if (event.publish) {
      PublishBytes pub = *event.publish;
      CborDeserializer des(pub.payload.data(), pub.payload.size());
      if (pub.topic == "sys") {
        auto msg = deserialize<SysMsg>(pub.payload);
        if ( msg )  sys_actor.tell(new SysCmd{.serdes = PublishSerdes { msg.value() }});  
      } else if ( pub.topic == "wifi") {
        auto msg = deserialize<WifiMsg>(pub.payload);
        if ( msg ) wifi_actor.tell(new WifiCmd{.serdes = PublishSerdes { msg.value() }});
      } else if ( pub.topic == "ps4") {
        auto msg = deserialize<Ps4Msg>(pub.payload);
        if ( msg ) ps4_actor.tell(new Ps4Cmd{.serdes = PublishSerdes { msg.value() }});
      } else if ( pub.topic == "zenoh") {
        auto msg = deserialize<ZenohMsg>(pub.payload);
        if ( msg ) zenoh_actor.tell(new ZenohCmd{.serdes = PublishSerdes { msg.value() }});
      } else {
        INFO("Received Zenoh unknown event");
      }
    } });

  // one thread to rule them all, in the hope to save some memory
  // wifi_actor.start();
  actor_thread.add_actor(wifi_actor);
  actor_thread.add_actor(zenoh_actor);
  actor_thread.add_actor(sys_actor);
  actor_thread.add_actor(led_actor);
  actor_thread.start();

  ps4_thread.add_actor(ps4_actor);
  ps4_thread.start();

  // log heap size, monitoring thread in main, we could exit also
  while (true)
  {
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    INFO(" free heap size: %lu biggest block : %lu ", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
  }
}

// re-entrant function to publish a serializable object
void zenoh_publish(const char *topic, std::optional<PublishSerdes> &serdes)
{
  if (serdes)
  {
    Bytes buffer;
    CborSerializer ser(buffer);
    auto &serializable = serdes.value().payload;
    serializable.serialize(ser);

    assert(buffer.size() > 0);
    assert(buffer.size() < 1024);

    zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
    // pulse led when we publish
    led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
  }
}

void log_ps4_event(Ps4Event event)
{
  if (event.blue_event)
  {
    switch (event.blue_event.value())
    {
    case DEVICE_DISCOVERED:
      INFO("DEVICE_DISCOVERED");
      break;
    case DEVICE_CONNECTED:
      INFO("DEVICE_CONNECTED");
      break;
    case DEVICE_DISCONNECTED:
      INFO("DEVICE_DISCONNECTED");
      break;
    case DEVICE_READY:
      INFO("DEVICE_READY");
      break;
    case CONTROLLER_DATA:
      // INFO("CONTROLLER_DATA");
      break;
    case OOB_EVENT:
      INFO("OOB_EVENT");
      break;
    }
  }
};