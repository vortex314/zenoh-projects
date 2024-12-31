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

class InfoTopic : public Serializable<InfoTopic> {
public:
  std::optional<std::string> name;
  std::optional<std::string> desc;
  InfoTopic(){};

  Result<Void> serialize(Serializer &ser) {
    TEST_R(Void, ser.map_begin(), "Failed to encode map");
    TEST_R(Void, ser.serialize(0, name), "Failed to encode name");
    TEST_R(Void, ser.serialize(1, desc), "Failed to encode desc");
    TEST_R(Void, ser.map_end(), "Failed to encode map");
    return Result<Void>::Ok(true);
  }

  Result<InfoTopic> deserialize(Deserializer &des) {
    InfoTopic topic = InfoTopic();
    TEST_R(InfoTopic, des.map_begin(), "Failed to decode map");
    TEST_R(InfoTopic, des.deserialize_string(), "Failed to decode name");
    TEST_R(InfoTopic, des.deserialize_string(), "Failed to decode desc");
    TEST_R(InfoTopic, des.map_end(), "Failed to decode map");
    return Result<InfoTopic>::Ok(topic);
  }
};

typedef enum PropMode {
  READ = 0,
  WRITE = 1,
  READ_WRITE = 2,
} PropMode;
typedef enum PropType {
  UINT = 0,
  INT = 1,
  STR = 2,
  BYTES = 3,
  FLOAT = 4,
  OBJECT = 5,
  ARRAY = 6,
} PropType;

class InfoProp : public Serializable<InfoProp> {
public:
  std::optional<uint32_t> id;
  std::optional<std::string> name;
  std::optional<std::string> desc;
  std::optional<PropType> type;
  std::optional<PropMode> mode;

  Result<Void> serialize(Serializer &ser) {
    TEST_R(Void, ser.map_begin(), "Failed to encode map");
    TEST_R(Void, ser.serialize(0, id), "Failed to encode id");
    TEST_R(Void, ser.serialize(1, name), "Failed to encode name");
    TEST_R(Void, ser.serialize(2, desc), "Failed to encode desc");
    TEST_R(Void, ser.serialize(3, type), "Failed to encode type");
    TEST_R(Void, ser.serialize(4, mode), "Failed to encode mode");
    TEST_R(Void, ser.map_end(), "Failed to encode map");
    return Result<Void>::Ok(true);
  }

  Result<InfoProp> deserialize(Deserializer &des) {
    InfoProp prop = InfoProp();
    TEST_R(InfoProp, des.map_begin(), "Failed to decode map");
    TEST_R(InfoProp, des.deserialize_uint32(), "Failed to decode id");
    TEST_R(InfoProp, des.deserialize_string(), "Failed to decode name");
    TEST_R(InfoProp, des.deserialize_string(), "Failed to decode desc");
    TEST_R(InfoProp, des.deserialize_uint32(), "Failed to decode type");
    TEST_R(InfoProp, des.deserialize_uint32(), "Failed to decode mode");
    TEST_R(InfoProp, des.map_end(), "Failed to decode map");
    return Result<InfoProp>::Ok(prop);
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

  Result<Void> serialize(Serializer &ser);
  Result<Msg> deserialize(Deserializer &des);
};

#define DST_KEY 0
#define SRC_KEY 1
#define INFO_PROP_KEY 6
#define INFO_TOPIC_KEY 7
#define PUBLISH_KEY 4

Result<Void> Msg::serialize(Serializer &ser) {
  TEST_R(Void, ser.map_begin(), "Failed to encode map");
  TEST_R(Void, ser.serialize(DST_KEY, dst), "Failed to encode dst");
  TEST_R(Void, ser.serialize(SRC_KEY, src), "Failed to encode src");
  if (info_prop.has_value()) {
    TEST_R(Void, ser.serialize(INFO_PROP_KEY), "Failed to encode info_prop");
    TEST_R(Void, info_prop.value().serialize(ser),
           "Failed to encode info_prop");
  }
  if (info_topic.has_value()) {
    TEST_R(Void, ser.serialize(INFO_TOPIC_KEY), "Failed to encode info_topic");
    TEST_R(Void, info_topic.value().serialize(ser),
           "Failed to encode info_topic");
  }
  if (publish.has_value()) {
    TEST_R(Void, ser.serialize(PUBLISH_KEY), "Failed to encode publish");
    TEST_R(Void, ser.serialize(publish.value()), "Failed to encode publish");
  }
  TEST_R(Void, ser.map_end(), "Failed to encode map");
  return Result<Void>::Ok(true);
}

  CborSerializer ser(1024);


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
  Msg msg;
  msg.dst = 1;
  msg.src = 2;
  InfoProp prop;
  prop.id = 3;
  prop.name = "prop";
  prop.desc = "prop desc";
  prop.type = PropType::UINT;
  prop.mode = PropMode::READ;
  msg.info_prop = prop;
  InfoTopic topic;
  topic.name = "topic";
  topic.desc = "topic desc";
  msg.info_topic = topic;
  Bytes b;
  auto r = msg.serialize(ser);
  if (r.is_err()) {

    printf("Failed to serialize message %d : %s \n", r.rc(), r.msg().c_str());
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    return;
  };
  auto buffer = ser.get_bytes().value();
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
  z_owned_session_t s;
  z_result_t res = z_open(&s, z_move(config), NULL);
  if (res < 0) {
    printf("Unable to open session! %d \n", res);
    exit(-1);
  }
  printf("Zenoh session opened \n");

  // Start the receive and the session lease loop for zenoh-pico
  zp_start_read_task(z_loan_mut(s), NULL);
  zp_start_lease_task(z_loan_mut(s), NULL);

  printf("Declaring publisher for '%s'...", KEYEXPR);
  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  z_view_keyexpr_from_str_unchecked(&ke, KEYEXPR);
  if (z_declare_publisher(z_loan(s), &pub, z_loan(ke), NULL) < 0) {
    printf("Unable to declare publisher for key expression!\n");
    exit(-1);
  }
  printf("OK\n");

  char buf[256];
  z_owned_bytes_t payload;
  for (int idx = 0; 1; ++idx) {
    vTaskDelay(5 / portTICK_PERIOD_MS);
    sprintf(buf, "[%6d] heap : %d ,%s  ", idx, esp_get_free_heap_size(), VALUE);
    // z_bytes_copy_from_str(&payload, buf);
    printf("Publishing '%d'...\n", buffer.size());
    z_bytes_copy_from_buf(&payload, buffer.data(), buffer.size());
    z_publisher_put(z_loan(pub), z_move(payload), NULL);
  }

  printf("Closing Zenoh Session...");
  z_drop(z_move(pub));

  z_drop(z_move(s));
  printf("OK!\n");
}
#else
void app_main() {
  printf("ERROR: Zenoh pico was compiled without Z_FEATURE_PUBLICATION but "
         "this example requires it.\n");
}
#endif
