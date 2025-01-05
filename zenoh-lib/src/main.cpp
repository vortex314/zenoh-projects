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

Bytes buffer(1024);
WifiActor wifi_actor;
ZenohActor zenoh_actor;
TaskHandle_t task_handle_zenoh;
TaskHandle_t task_handle_wifi;

extern "C" void app_main() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  zenoh_actor.prefix("lm1");
  // device name src/<device_name>/<object> -> props messages

  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.handlers.push_back([&](WifiEvent event) {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.cmds.send(ZenohCmd{.action = ZenohAction::Connect});
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.cmds.send(ZenohCmd{.action = ZenohAction::Disconnect});
    }
    if ( event.info) {
      printf("Received wifi info\n");
      zenoh_actor.cmds.send(ZenohCmd{.publish_serialized = ZenohSerial{"wifi/info", event.info.value()}});
    }
    if ( event.msg) {
      printf("Received wifi msg\n");
      zenoh_actor.cmds.send(ZenohCmd{.publish_serialized = ZenohSerial{"wifi/msg", event.msg.value()}});
    }
  });

  zenoh_actor.handlers.push_back([&](ZenohEvent event) {
    if (event.binary) {
      ZenohBinary bin = event.binary.value();
      printf("Received binary: %s\n", bin.topic.c_str());
    } else {
      printf("Received Zenoh unknown event\n");
    }
  });

  xTaskCreate(
      [](void *arg) {
        auto self = static_cast<WifiActor *>(arg);
        self->run();
      },
      "wifi_actor_task", 10000, &wifi_actor, 5, &task_handle_wifi);
  xTaskCreate(
      [](void *arg) {
        auto self = static_cast<ZenohActor *>(arg);
        self->run();
      },
      "zenoh_actor_task", 10000, &zenoh_actor, 5, &task_handle_zenoh);

  char buf[256];
  for (int idx = 0; 1; ++idx) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(buf, "Hello World %d", idx);
    zenoh_actor.cmds.send(
        ZenohCmd{.publish_binary =
                     ZenohBinary{"motor/pwm", Bytes{buf, buf + strlen(buf)}}});
    printf(" free heap size: %d\n", esp_get_free_heap_size());
  }
}
