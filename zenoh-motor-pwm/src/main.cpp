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

#define DEVICE_NAME "mtr1"
#define DST_DEVICE "dst/" DEVICE_NAME "/"
#define SRC_DEVICE "src/" DEVICE_NAME "/"

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
  zenoh_actor.tell(new ZenohCmd{.publish_bytes = PublishBytes{topic, buffer}});
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
  ESP_ERROR_CHECK(nvs_init());
  zenoh_actor.prefix(DEVICE_NAME); // set the zenoh prefix to src/esp3 and destination subscriber dst/esp3/**
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
                      { event.publish.for_each([](auto msg)
                                               { publish(SRC_DEVICE "wifi", msg); }); });
  sys_actor.on_event([&](SysEvent event)
                     { event.publish >> [](auto msg)
                       { publish(SRC_DEVICE "sys", msg); }; });
  zenoh_actor.on_event([&](ZenohEvent event)
                       { event.publish >> [](auto msg)
                         { publish(SRC_DEVICE "zenoh", msg); }; });
  ota_actor.on_event([&](OtaEvent event)
                     { event.publish >> [](auto msg)
                       { publish(SRC_DEVICE "ota", msg); }; });
  motor_actor.on_event([&](MotorEvent event)
                       { event.publish >> [](auto msg)
                         { publish(SRC_DEVICE "motor", msg); }; });

  // send commands to actors coming from zenoh, deserialize and send to the right actor
  zenoh_actor.on_event([&](ZenohEvent event)
                       {
   event.publish_bytes
     .filter([&](auto pb){ return pb.topic == DST_DEVICE "sys" ;})
     .and_then([&](auto pb ){ return cbor_deserialize<SysMsg>(pb.payload);})
     .for_each([&](auto msg){ sys_actor.tell(new SysCmd{.publish = msg}); });
   event.publish_bytes
     .filter([&](auto pb){ return pb.topic == DST_DEVICE "wifi" ;})
     .and_then([&](auto pb ){ return cbor_deserialize<WifiMsg>(pb.payload);})
     .for_each([&](auto msg){ wifi_actor.tell(new WifiCmd{.publish = msg}); });
   event.publish_bytes
     .filter([&](auto pb){ return pb.topic == DST_DEVICE "zenoh" ;})
     .and_then([&](auto pb ){ return cbor_deserialize<ZenohMsg>(pb.payload);})
     .for_each([&](auto msg){ zenoh_actor.tell(new ZenohCmd{.publish = msg}); });
   event.publish_bytes
     .filter([&](auto pb){ return pb.topic == DST_DEVICE "ota" ;})
     .and_then([&](auto pb ){ return cbor_deserialize<OtaMsg>(pb.payload);})
     .for_each([&](auto msg){ ota_actor.tell(new OtaCmd{.publish = msg}); }); 
     event.publish_bytes
     .filter([&](auto pb){ return pb.topic == DST_DEVICE "ota" ;})
     .and_then([&](auto pb ){ return cbor_deserialize<MotorMsg>(pb.payload);})
     .for_each([&](auto msg){ motor_actor.tell(new MotorCmd{.publish = msg}); }); });

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
