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
#include <motor_actor.h>
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

MotorActor motor_actor("motor", 9000, 40, 5);
Thread motor_thread("motor_thread", 9000, 40, 23, Cpu::CPU0);

Log logger;

void zenoh_publish(const char *topic, std::optional<PublishSerdes> &serdes);

template <typename T>
std::optional<T> deserialize(Bytes bytes)
{
  CborDeserializer des(bytes.data(), bytes.size());
  T obj;
  if (obj.deserialize(des).is_ok())
    return obj;
  return std::nullopt;
}

#define DEVICE_NAME "mtr1"

#define DST_DEVICE "dst/" DEVICE_NAME "/"

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

  zenoh_actor.prefix(DEVICE_NAME); // set the zenoh prefix to src/mtr1 and destination subscriber dst/mtr1/ **

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
                      { zenoh_publish("src/mtr1/wifi", event.serdes);
                      zenoh_publish("info/mtr1/wifi",event.prop_info); });
  sys_actor.on_event([&](SysEvent event)
                     { zenoh_publish("src/mtr1/sys", event.serdes); 
                     zenoh_publish("info/mtr1/sys",event.prop_info); });
  zenoh_actor.on_event([&](ZenohEvent event) // send to myself
                       { zenoh_publish("src/mtr1/zenoh", event.serdes); 
                       zenoh_publish("info/mtr1/zenoh",event.prop_info); });
  motor_actor.on_event([&](MotorEvent event)
                        { zenoh_publish("src/mtr1/motor", event.serdes); });
  ota_actor.on_event([&](OtaEvent event)
                        { zenoh_publish("src/mtr1/ota", event.serdes); });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
    if (event.publish) {
      PublishBytes pub = *event.publish;
      CborDeserializer des(pub.payload.data(), pub.payload.size());
      if (pub.topic == DST_DEVICE "sys" ) {
        auto msg = deserialize<SysMsg>(pub.payload);
        if ( msg )  sys_actor.tell(new SysCmd{.serdes = PublishSerdes { msg.value() }});  
      } else if ( pub.topic == "dst/mtr1/wifi") {
        auto msg = deserialize<WifiMsg>(pub.payload);
        if ( msg ) wifi_actor.tell(new WifiCmd{.serdes = PublishSerdes { msg.value() }});
      } else if ( pub.topic == "dst/mtr1/zenoh") {
        auto msg = deserialize<ZenohMsg>(pub.payload);
        if ( msg ) zenoh_actor.tell(new ZenohCmd{.serdes = PublishSerdes { msg.value() }});
      } else if ( pub.topic == "dst/mtr1/motor") {
        auto msg = deserialize<MotorMsg>(pub.payload);
        if ( msg ) motor_actor.tell(new MotorCmd{.msg = msg.value()});
      } else if ( pub.topic == "dst/mtr1/ota") {
        INFO("Received OTA message");
        auto msg = deserialize<OtaMsg>(pub.payload);
        if ( msg ) ota_actor.tell(new OtaCmd{.msg = msg.value()});
      }else {
        INFO("Received Zenoh unknown event");
      }
    } });

  // one thread to rule them all, in the hope to save some memory
  // wifi_actor.start();
  motor_thread.add_actor(motor_actor);
  motor_thread.start();
  //actor_thread.add_actor(motor_actor);
  actor_thread.add_actor(wifi_actor);
  actor_thread.add_actor(zenoh_actor);
  actor_thread.add_actor(sys_actor);
  actor_thread.add_actor(led_actor);
  actor_thread.add_actor(ota_actor);
  actor_thread.start();

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
//    assert(buffer.size() < 1024);

    zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
    // pulse led when we publish
    led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
  }
}
