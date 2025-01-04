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
#include <nanocbor/nanocbor.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zenoh-pico.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

#define CLIENT_OR_PEER 0 // 0: Client mode; 1: Peer mode
#if CLIENT_OR_PEER == 0
#define MODE "client"
#define CONNECT "" // If empty, it will scout
#elif CLIENT_OR_PEER == 1
#define MODE "peer"
#define CONNECT "udp/224.0.0.123:7447#iface=lo" // If empty, it will scout
#else
#error "Unknown Zenoh operation mode. Check CLIENT_OR_PEER value."
#endif

#define KEYEXPR "demo/example/zenoh-pico-pub"
#define VALUE "[ESPIDF]{ESP32} Publication from Zenoh-Pico!"

#include <wifi_actor.h>
#include <zenoh_actor.h>

Bytes buffer(1024);
WifiActor wifi_actor;
ZenohActor zenoh_actor;

extern "C" void app_main() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  zenoh_actor.prefix("lm1");
 // Set WiFi in STA mode and trigger attachment
  wifi_actor.handlers.push_back([&](WifiEvent event) {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.cmds.send(ZenohCmd{.action = ZenohAction::Connect});
    }
    if ( event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.cmds.send(ZenohCmd{.action = ZenohAction::Disconnect});
    }
  });

  TaskHandle_t task_handle;
  xTaskCreate(
      [](void *arg) {
        auto self = static_cast<WifiActor *>(arg);
        self->run();
      },
      "wifi_actor_task", 10000, &wifi_actor, 5, &task_handle);
  xTaskCreate(
      [](void *arg) {
        auto self = static_cast<ZenohActor *>(arg);
        self->run();
      },
      "zenoh_actor_task", 10000, &zenoh_actor, 5, &task_handle);

  char buf[256];
  for (int idx = 0; 1; ++idx) {
    vPortYield();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    sprintf(buf, "Hello World %d", idx);
    zenoh_actor.cmds.send(ZenohCmd{.publish_binary = ZenohBinary{"motor/pwm", Bytes{buf, buf + strlen(buf)}}});
    if ( idx%100 ==0) printf(" free heap size: %d\n", esp_get_free_heap_size());
  }

}

