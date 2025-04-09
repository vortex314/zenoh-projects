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
// threads can be run separately or share a thread
// Pinning all on CPU0 to avoid Bluetooth crash in rwbt.c line 360.
WifiActor wifi_actor("wifi", 9000, 40, 5);
ZenohActor zenoh_actor("zenoh", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
OtaActor ota_actor("ota", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 23, Cpu::CPU0);

Log logger;

void zenoh_publish(const char *topic, std::optional<PublishSerdes> &serdes);

template <typename T, typename U>
std::optional<U> map(std::optional<T> t, std::function<U(T)> f)
{
  if (t)
  {
    return f(t);
  }
  else
  {
    return std::nullopt;
  }
}

template <typename T, typename F>
void operator>>(std::optional<T> t, F f)
{
  if (t)
  {
    f(t.value());
  }
}

template <typename T, typename F>
auto operator>>=(const std::optional<T> &opt, F &&func)
    -> std::optional<std::invoke_result_t<F, T>>
{
  // If the optional has a value, apply the function to it and wrap the result in an optional
  if (opt)
  {
    return std::invoke(std::forward<F>(func), *opt);
  }
  // Otherwise, return an empty optional
  return std::nullopt;
}

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
                      { zenoh_publish("src/esp3/wifi", event.serdes);
                      zenoh_publish("info/esp3/wifi",event.prop_info); });
  sys_actor.on_event([&](SysEvent event)
                     { zenoh_publish("src/esp3/sys", event.serdes); 
                     zenoh_publish("info/esp3/sys",event.prop_info); });
  zenoh_actor.on_event([&](ZenohEvent event) // send to myself
                       { zenoh_publish("src/esp3/zenoh", event.serdes); 
                       zenoh_publish("info/esp3/zenoh",event.prop_info); });
  ota_actor.on_event([&](OtaEvent event)
                     { zenoh_publish("src/esp3/ota", event.serdes); });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
    if (event.publish) {
      if (event.publish->topic == "dst/esp3/sys") {
        auto msg = cbor_deserialize<SysMsg>(event.publish->payload);
        if ( msg )  sys_actor.tell(new SysCmd{.serdes = PublishSerdes ( msg.value() )});  
      } 
      else if ( event.publish->topic == "dst/esp3/wifi") {
        auto msg = cbor_deserialize<WifiMsg>(event.publish->payload);
        if ( msg ) wifi_actor.tell(new WifiCmd{.serdes = PublishSerdes ( msg.value() )});
      } 
      else if ( event.publish->topic == "dst/esp3/zenoh") {
        auto msg = cbor_deserialize<ZenohMsg>(event.publish->payload);
        if ( msg ) zenoh_actor.tell(new ZenohCmd{.serdes = PublishSerdes ( msg.value() )});
      } 
      else if ( event.publish->topic == "dst/esp3/ota") {
        cbor_deserialize<OtaMsg>(event.publish->payload) >> [&](OtaMsg msg) {ota_actor.tell(new OtaCmd{.msg = std::move(msg)});};
      } 
      else {
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
void zenoh_publish(const char *topic, std::optional<PublishSerdes> &serdes)
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
