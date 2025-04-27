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
Thread wifi_thread("wifi_thread", 9000, 40, 23, Cpu::CPU1);
ZenohActor zenoh_actor("zenoh", 9000, 40, 5);
SysActor sys_actor("sys", 9000, 40, 5);
LedActor led_actor("led", 9000, 40, 5);
OtaActor ota_actor("ota", 9000, 40, 5);
Thread actor_thread("actors", 9000, 40, 23, Cpu::CPU0);

MotorActor motor_actor("motor", 9000, 40, 5);
Thread motor_thread("motor_thread", 9000, 40, 23, Cpu::CPU0);

Log logger;

void publish(const char *topic, const Serializable &serializable)
{
  Bytes buffer;
  CborSerializer ser(buffer);
  serializable.serialize(ser);
  zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
  // pulse led when we publish
  led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
}

#define DEVICE_NAME "mtr1"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

/*
| WIFI | = connect/disconnect => | ZENOH | ( set up session )
| SYS | = system events => | ZENOH | ( publish )
| ZENOH | = zenoh events => | ZENOH | ( publish )
| ZENOH | = publish events => | LED | (pulse)
*/

/**
 * @brief Main application entry point for initializing and managing the system.
 *
 * This function initializes the Non-Volatile Storage (NVS), configures WiFi power-saving mode,
 * and sets up the Zenoh actor prefix. It wires together various actors (WiFi, system, Zenoh, motor, OTA)
 * to handle events and publish data. It also processes incoming Zenoh commands and dispatches them
 * to the appropriate actors. The function starts threads for managing actors and continuously logs
 * the free heap size and largest memory block available.
 *
 * Key functionalities:
 * - Initializes NVS and handles errors related to storage.
 * - Configures WiFi power-saving mode.
 * - Sets up event handling for WiFi, system, Zenoh, motor, and OTA actors.
 * - Publishes actor events to Zenoh topics.
 * - Processes incoming Zenoh commands and routes them to the appropriate actors.
 * - Starts threads for managing actors and their events.
 * - Logs memory usage periodically.
 *
 * @note This function runs indefinitely in a loop to monitor system memory.
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
      INFO("WIFI_CONNECTED => ZENOH_CONNECT");
      auto ok = zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
      INFO("WIFI_CONNECTED => ZENOH_CONNECT %s", ok ? "true" : "false");
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      auto ok = zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Disconnect});
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
  motor_actor.on_event([&](MotorEvent event)
                       { event.msg >> [](auto msg)
                         { publish(SRC_DEVICE "motor", msg); }; });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
    if (event.publish) {
      if (event.publish->topic == DST_DEVICE "sys") {
        cbor_deserialize<SysMsg>(event.publish->payload) >>
        [](auto msg){sys_actor.tell(new SysCmd{.msg = msg});};
      } 
      else if ( event.publish->topic == DST_DEVICE "wifi") {
        cbor_deserialize<WifiMsg>(event.publish->payload) >>
        [](auto msg){wifi_actor.tell(new WifiCmd{.msg = msg});};
      } 
      else if ( event.publish->topic == DST_DEVICE "zenoh") {
        cbor_deserialize<ZenohMsg>(event.publish->payload) >>
        [](auto msg){zenoh_actor.tell(new ZenohCmd{.msg = msg});};
      } 
      else if ( event.publish->topic == DST_DEVICE "ota") {
        cbor_deserialize<OtaMsg>(event.publish->payload) >>
        [](auto msg){ota_actor.tell(new OtaCmd{.msg = msg});};
      } 
      else if ( event.publish->topic == DST_DEVICE "motor") {
        cbor_deserialize<MotorMsg>(event.publish->payload) >>
        [](auto msg){motor_actor.tell(new MotorCmd{.msg = msg});};
      } 
      else {
        INFO("Received Zenoh unknown event");
      }
    } });

  // one thread to rule them all, in the hope to save some memory
  // wifi_actor.start();
  motor_thread.add_actor(motor_actor);
  motor_thread.start();
  // actor_thread.add_actor(motor_actor);
  wifi_thread.add_actor(wifi_actor); // wifi blocking
  wifi_thread.start();

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
