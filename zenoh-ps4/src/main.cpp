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
#include <ps4_actor.h>
#include <led_actor.h>
#include <log.h>

WifiActor wifi_actor;
ZenohActor zenoh_actor;
SysActor sys_actor;
Ps4Actor ps4_actor;
LedActor led_actor;

Log logger;

struct MotorMsg : Serializable
{
  std::optional<float> pwm = std::nullopt;
  std::optional<bool> direction = std::nullopt;
  std::optional<float> speed = std::nullopt;
  std::optional<int32_t> distance = std::nullopt;
  std::optional<int32_t> angle = std::nullopt;

  Res serialize(Serializer &ser) override
  {
    ser.reset();
    ser.map_begin();
    ser.serialize(0, pwm);
    ser.serialize(1, direction);
    ser.serialize(2, speed);
    ser.serialize(3, distance);
    ser.serialize(4, angle);
    return ser.map_end();
  }
  Res deserialize(Deserializer &des) override
  {
    des.deserialize(pwm);
    des.deserialize(direction);
    des.deserialize(speed);
    des.deserialize(distance);
    des.deserialize(angle);
    return Res::Ok();
  }
};

// re-entrant function to publish a serializable object
void publish(ZenohActor &zenoh_actor, std::optional<PublishSerdes> &serdes)
{
  if (serdes)
  {
    Bytes buffer;
    CborSerializer ser(buffer);
    auto topic = serdes.value().topic;
    auto &serializable = serdes.value().payload;
    serializable.serialize(ser);
    if ( buffer.size() > 1024 || topic.size() > 1024 ) {
      ERROR("publish: buffer or topic too large {} {}", buffer.size(), topic.size());
      return;
    }
    zenoh_actor.tell(new ZenohCmd{.publish = PublishBytes{topic, buffer}});
    led_actor.tell(new LedCmd{.action = LED_PULSE, .duration = 10});
  }
}

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

  zenoh_actor.prefix("lm1");
  // WiFi connectivity starts and stops zenoh connection
  wifi_actor.on_event([](WifiEvent event)
                         {
    if (event.signal == WifiSignal::WIFI_CONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Connect});
    }
    if (event.signal == WifiSignal::WIFI_DISCONNECTED) {
      zenoh_actor.tell(new ZenohCmd{.action = ZenohAction::Disconnect});
    } });

  wifi_actor.on_event([](WifiEvent event)
                         { publish(zenoh_actor, event.serdes); });

  sys_actor.on_event([](SysEvent event)
                        { publish(zenoh_actor, event.serdes); });

  zenoh_actor.on_event([](ZenohEvent event) // send to myself
                          { publish(zenoh_actor, event.serialize); });
  ps4_actor.on_event([](Ps4Event event)
                        {
                          if ( event.serdes ) {
                            publish(zenoh_actor, event.serdes);
                          }
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

  zenoh_actor.on_event([&](ZenohEvent event)
                          {
    if (event.publish) {
      PublishBytes pub = event.publish.value();
      if (pub.topic == "sys") {
        SysMsg sys_msg;
        CborDeserializer des(pub.payload.data(), pub.payload.size());
        sys_msg.deserialize(des);
        sys_actor.tell(new SysCmd{.publish = std::move(pub)});
      } else if ( pub.topic == "wifi") {
        wifi_actor.tell(new WifiCmd{.publish = std::move(pub)});
      } else if ( pub.topic == "zenoh") {
        zenoh_actor.tell(new ZenohCmd{.publish = std::move(pub)});
      } else {
        INFO("Received Zenoh unknown event");
      }
    } });


  wifi_actor.start();
  zenoh_actor.start();
  sys_actor.start();
  ps4_actor.start();
  led_actor.start();

  char buf[256];
  for (int idx = 0; 1; ++idx)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    MotorMsg motor_msg;
    motor_msg.pwm = idx;
    motor_msg.direction = 1;
    motor_msg.speed = 100;
    motor_msg.distance = 100;
    motor_msg.angle = 90;
    PublishSerdes pub{.topic = "motor/pwm", .payload = motor_msg};
    std::optional<PublishSerdes> opt = pub;
    publish(zenoh_actor, opt);

    INFO(" free heap size: %lu biggest block : %lu ", esp_get_free_heap_size(),heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
  }
}
