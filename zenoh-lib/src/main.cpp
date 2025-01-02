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

#if Z_FEATURE_PUBLICATION == 1
#define STRINGIFY(X) #X
#define S(X) STRINGIFY(X)
#define ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0

static bool s_is_wifi_connected = false;
static EventGroupHandle_t s_event_group_handler;
static int s_retry_count = 0;

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

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_count < ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_count++;
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(s_event_group_handler, WIFI_CONNECTED_BIT);
    s_retry_count = 0;
  }
}

void wifi_init_sta(void) {
  s_event_group_handler = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));

  esp_event_handler_instance_t handler_any_id;
  esp_event_handler_instance_t handler_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &handler_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &handler_got_ip));

  wifi_config_t wifi_config;
  bzero(&wifi_config, sizeof(wifi_config));
  strcpy((char *)wifi_config.sta.ssid, "Merckx4");
  strcpy((char *)wifi_config.sta.password, S(WIFI_PASS));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits =
      xEventGroupWaitBits(s_event_group_handler, WIFI_CONNECTED_BIT, pdFALSE,
                          pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    s_is_wifi_connected = true;
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      IP_EVENT, IP_EVENT_STA_GOT_IP, handler_got_ip));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      WIFI_EVENT, ESP_EVENT_ANY_ID, handler_any_id));
  vEventGroupDelete(s_event_group_handler);
}

#include <serdes.h>

// using namespace std;
#define TOPIC_NAME_KEY 0
#define TOPIC_DESC_KEY 1

class InfoTopic {
public:
  std::optional<std::string> name;
  std::optional<std::string> desc;
  InfoTopic(){};

  Res serialize(Serializer &ser) {
    RET_ERR(ser.map_begin(), "Failed to encode map");
    RET_ERR(ser.serialize(0, name), "Failed to encode name");
    RET_ERR(ser.serialize(1, desc), "Failed to encode desc");
    RET_ERR(ser.map_end(), "Failed to encode map");
    return Res::Ok();
  }

  static Res deserialize(Deserializer &des, InfoTopic &info_topic) {
    des.iterate_map([&info_topic](Deserializer &des, uint32_t key) {
      switch (key) {
      case TOPIC_NAME_KEY: {
        RET_ERR(des.deserialize(info_topic.name), "Failed to decode string");
        break;
      }
      case TOPIC_DESC_KEY: {
        RET_ERR(des.deserialize(info_topic.desc), "Failed to decode string");
        break;
      }
      default:
        // todo skip item
        return Res::Ok(); // ignore unknown keys
      }
    });
    return Res::Ok();
  }
};

typedef enum PropMode {
  PROP_READ = 0,
  PROP_WRITE = 1,
  PROP_READ_WRITE = 2,
} PropMode;
typedef enum PropType {
  PROP_UINT = 0,
  PROP_INT = 1,
  PROP_STR = 2,
  BYTES = 3,
  PROP_FLOAT = 4,
  PROP_OBJECT = 5,
  PROP_ARRAY = 6,
} PropType;

#define PROP_ID_KEY 0
#define PROP_NAME_KEY 1
#define PROP_DESC_KEY 2
#define PROP_TYPE_KEY 3
#define PROP_MODE_KEY 4

class InfoProp {
public:
  std::optional<uint32_t> id;
  std::optional<std::string> name;
  std::optional<std::string> desc;
  std::optional<PropType> type;
  std::optional<PropMode> mode;

  Res serialize(Serializer &ser) {
    RET_ERR(ser.map_begin(), "Failed to encode map");
    RET_ERR(ser.serialize(PROP_ID_KEY, id), "Failed to encode id");
    RET_ERR(ser.serialize(PROP_NAME_KEY, name), "Failed to encode name");
    RET_ERR(ser.serialize(PROP_DESC_KEY, desc), "Failed to encode desc");
    RET_ERR(ser.serialize(PROP_TYPE_KEY, (std::optional<uint32_t>)type),
            "Failed to encode type");
    RET_ERR(ser.serialize(PROP_MODE_KEY, (std::optional<uint32_t>)mode),
            "Failed to encode mode");
    RET_ERR(ser.map_end(), "Failed to encode map");
    return Res::Ok();
  }

  static Res deserialize(Deserializer &des, InfoProp &prop) {
    des.iterate_map([&prop](Deserializer &des, uint32_t key) {
      switch (key) {
      case PROP_ID_KEY: {
        RET_ERR(des.deserialize(prop.id), "Failed to decode uint32_t");
        break;
      }
      case PROP_NAME_KEY: {
        RET_ERR(des.deserialize(prop.name), "Failed to decode string");
        break;
      }
      case PROP_DESC_KEY: {
        RET_ERR(des.deserialize(prop.desc), "Failed to decode string");
        break;
      }
      case PROP_TYPE_KEY: {
        uint32_t type;
        RET_ERR(des.deserialize(type), "Failed to decode PropType");
        prop.type = (PropType)type;
        break;
      }
      case PROP_MODE_KEY: {
        uint32_t mode;
        RET_ERR(des.deserialize(mode), "Failed to decode PropMode");
        prop.mode = (PropMode)mode;
        break;
      }

      default:
        printf("Unknown key %d \n", key);
        return Res::Err(0, "Unknown key");
      }
    });
    return Res::Ok();
  }
};

class Msg {
public:
  std::optional<uint32_t> dst;         // 0
  std::optional<uint32_t> src;         // 1
  std::optional<InfoProp> info_prop;   // 6
  std::optional<InfoTopic> info_topic; // 7
  std::optional<Bytes> publish;        // 4

  Msg(){};

  Res serialize(Serializer &ser);
  static Res deserialize(Deserializer &des, Msg &msg);
};

#define DST_KEY 0
#define SRC_KEY 1
#define INFO_PROP_KEY 6
#define INFO_TOPIC_KEY 7
#define PUBLISH_KEY 4

Res Msg::serialize(Serializer &ser) {
  RET_ERR(ser.map_begin(), "Failed to encode map");
  RET_ERR(ser.serialize(DST_KEY, dst), "Failed to encode dst");
  RET_ERR(ser.serialize(SRC_KEY, src), "Failed to encode src");
  RET_ERR(ser.map_end(), "Failed to encode map");
  return Res::Ok();
}

Res Msg::deserialize(Deserializer &des, Msg &msg) { return Res::Ok(); }

class System {
public:
  std::optional<uint32_t> free_heap_size;
  std::optional<int64_t> local_time;

  Res serialize(Serializer &ser) {
    free_heap_size = esp_get_free_heap_size();
    local_time = esp_timer_get_time();
    RET_ERR(ser.map_begin(), "Failed to encode map");
    RET_ERR(ser.serialize(0, free_heap_size),
            "Failed to encode free_heap_size");
    RET_ERR(ser.serialize(1, local_time), "Failed to encode local_time");
    RET_ERR(ser.map_end(), "Failed to encode map");
    return Res::Ok();
  }
};

Bytes buffer(1024);
CborSerializer ser(1024);
CborDeserializer des(1024);

extern "C" void app_main() {

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Set WiFi in STA mode and trigger attachment
  printf("========================================\n");

  InfoProp prop;
  prop.id = 3;
  prop.name = "prop";
  prop.desc = "prop desc";
  prop.type = PropType::PROP_UINT;
  prop.mode = PropMode::PROP_READ;
  InfoTopic topic;
  topic.name = "topic";
  topic.desc = "topic desc";
  Bytes buffer;
  prop.serialize(ser);
  auto r2 = ser.get_bytes(buffer);

  if (r2.is_err()) {
    printf("Failed to fill buffer %d : %s \n", r2.rc(), r2.msg().c_str());
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    return;
  }; /*
   auto r = msg.deserialize(des);
   if (r.is_err()) {
     printf("Failed to deserialize message %d : %s \n", r.rc(),
   r.msg().c_str()); vTaskDelay(10000 / portTICK_PERIOD_MS); } else {
     printf("Deserialized message \n");
     printf("DST: %d \n", r.value().dst.value());
     printf("SRC: %d \n", r.value().src.value());
     printf("INFO_PROP: \n");
     printf("ID: %d \n", r.value().info_prop.value().id.value());
     printf("NAME: %s \n", r.value().info_prop.value().name.value().c_str());
     printf("DESC: %s \n", r.value().info_prop.value().desc.value().c_str());
     printf("TYPE: %d \n", r.value().info_prop.value().type.value());
     printf("MODE: %d \n", r.value().info_prop.value().mode.value());
     printf("INFO_TOPIC: \n");
     printf("NAME: %s \n", r.value().info_topic.value().name.value().c_str());
     printf("DESC: %s \n", r.value().info_topic.value().desc.value().c_str());
   }*/

  printf("Connecting to WiFi...\n");
  wifi_init_sta();
  while (!s_is_wifi_connected) {
    printf(".");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  };
  printf("Wifi connected \n");

  // Initialize Zenoh Session and other parameters
  z_owned_config_t config;
  z_config_default(&config);
  zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE);
  if (strcmp(CONNECT, "") != 0) {
    //        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY,
    //        CONNECT);
    zp_config_insert(z_loan_mut(config), Z_CONFIG_LISTEN_KEY, CONNECT);
  }

  // Open Zenoh session
  printf("Opening Zenoh Session...");
  z_owned_session_t zenoh_session;
  z_result_t res = z_open(&zenoh_session, z_move(config), NULL);
  if (res < 0) {
    printf("Unable to open session! %d \n", res);
    exit(-1);
  }
  printf("Zenoh session opened \n");

  // Start the receive and the session lease loop for zenoh-pico
  zp_start_read_task(z_loan_mut(zenoh_session), NULL);
  zp_start_lease_task(z_loan_mut(zenoh_session), NULL);

  printf("Declaring publisher for '%s'...", KEYEXPR);
  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  z_view_keyexpr_from_str_unchecked(&ke, KEYEXPR);
  if (z_declare_publisher(z_loan(zenoh_session), &pub, z_loan(ke), NULL) < 0) {
    printf("Unable to declare publisher for key expression!\n");
    exit(-1);
  }
  printf("OK\n");
  z_owned_keyexpr_t keyexpr;
  z_keyexpr_from_str(&keyexpr, "info/lm1/motor");

  char buf[256];
  z_owned_bytes_t payload;
  for (int idx = 0; 1; ++idx) {
  //  vTaskDelay(5 / portTICK_PERIOD_MS);
  vPortYield();
    sprintf(buf, "[%6d] heap : %d ,%s  ", idx, esp_get_free_heap_size(), VALUE);
    // z_bytes_copy_from_str(&payload, buf);
    // printf("Publishing '%d'...\n", buffer.size());
    ser.reset();
    prop.id = idx;
    prop.serialize(ser);
    ser.get_bytes(buffer);
    z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size());
    z_publisher_put(z_loan(pub), z_move(payload), NULL);
    ser.get_bytes(buffer);
    z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size());
    z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL);
    z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size());
    z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL);
    z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size());
    z_put(z_loan(zenoh_session), z_loan(keyexpr), z_move(payload), NULL);
  }

  printf("Closing Zenoh Session...");
  z_drop(z_move(pub));

  z_drop(z_move(zenoh_session));
  printf("OK!\n");
}
#else
void app_main() {
  printf("ERROR: Zenoh pico was compiled without Z_FEATURE_PUBLICATION but "
         "this example requires it.\n");
}
#endif
