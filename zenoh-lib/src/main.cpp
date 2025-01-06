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
#include <wifi_actor.h>
#include <zenoh-pico.h>
#include <zenoh_actor.h>
#include <sys_actor.h>
#include <log.h>

Bytes buffer(1024);
WifiActor wifi_actor;
ZenohActor zenoh_actor;
SysActor sys_actor;
TaskHandle_t task_handle_zenoh;
TaskHandle_t task_handle_wifi;
CborSerializer ser(1024);
Log logger;

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

  INFO("Starting WiFi and Zenoh actors");

  zenoh_actor.prefix("lm1");
  // device name src/<device_name>/<object> -> props messages

  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.add_handler([](WifiEvent event)
                         {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Disconnect});
    }
    if ( event.info) {
      ser.reset();
      ser.serialize(event.info.value());
      ser.get_bytes(buffer);
      zenoh_actor.tell(new ZenohCmd{.publish = PublishMsg{"wifi/info", std::move(buffer)}});
    } });

  zenoh_actor.add_handler([&](ZenohEvent event)
                          {
    if (event.publish) {
      PublishMsg pub = event.publish.value();
      INFO("Received binary: %s", pub.topic.c_str());
      if (pub.topic == "sys") {
        sys_actor.tell(new SysCmd{.publish = std::move(pub)});
      } else if ( pub.topic == "wifi") {
        wifi_actor.tell(new WifiCmd{.publish = std::move(pub)});
      } else {
        INFO("Received Zenoh unknown event");
      }
    } });

  wifi_actor.start();
  zenoh_actor.start();
  sys_actor.start();

  char buf[256];
  for (int idx = 0; 1; ++idx)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(buf, "Hello World %d ===========================================================================================", idx);
    zenoh_actor.tell(new ZenohCmd{.publish =
                                      PublishMsg{"motor/pwm", Bytes{buf, buf + strlen(buf)}}});
    INFO(" free heap size: %lu", esp_get_free_heap_size());
  }
}
