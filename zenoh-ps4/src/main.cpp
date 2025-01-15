//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <functional>
#include <nanocbor/nanocbor.h>
#include <nvs_flash.h>
#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <actor.h>
#include <wifi_actor.h>
#include <zenoh-pico.h>
#include <zenoh_actor.h>
#include <sys_actor.h>
#include <ps4_actor.h>
#include <led_actor.h>
#include <log.h>

WifiActor wifi_actor;
ZenohActor zenoh_actor;
SysActor sys_actor;
Ps4Actor ps4_actor;
LedActor led_actor;
Thread actors("actors", 7000, 20);
Thread bluetooth("bluetooth", 7000, 20);
Log logger;

// re-entrant function to publish a serializable object
void publish(ZenohActor &zenoh_actor, const char *topic, std::optional<PublishSerdes> &serdes)
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

| WIFI | = connect/disconnect => | ZENOH | ( publish )
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

  zenoh_actor.prefix("lm1"); // set the zenoh prefix to src/lm1 and destination dst/lm1

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
                      { publish(zenoh_actor, "wifi", event.serdes); });
  sys_actor.on_event([&](SysEvent event)
                     { publish(zenoh_actor, "sys", event.serdes); });
  zenoh_actor.on_event([&](ZenohEvent event) // send to myself
                       { publish(zenoh_actor, "zenoh", event.serdes); });
  ps4_actor.on_event([&](Ps4Event event)
                     { publish(zenoh_actor, "ps4", event.serdes); });

  // log connection events
  ps4_actor.on_event([](Ps4Event event)
                     {
                          if (event.blue_event) {
                             switch (event.blue_event.value()) {
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
                            } } });

  // send commands to actors coming from zenoh
  // deserialize and send to the right actor

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
  actors.add_actor(wifi_actor);
  actors.add_actor(zenoh_actor);
  actors.add_actor(sys_actor);
  actors.add_actor(led_actor);
  actors.start();
  
  bluetooth.add_actor(ps4_actor);
  bluetooth.start();

  // log heap size, monitoring thread in main, we could exit also
  while (true)
  {
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    INFO(" free heap size: %lu biggest block : %lu ", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
  }
}
