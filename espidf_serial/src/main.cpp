

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <zenoh-pico.h>
#include <zenoh-pico/config.h>

z_owned_session_t s;
z_owned_publisher_t pub;
const char *keyexpr = "demo/example/zenoh-pico-pub";
const char *value = "Hello, Zenoh-Pico!";
#define MODE "client"
#define CONNECT0 "serial/1.3#baudrate=115200"
#define CONNECT2 "serial/17.16#baudrate=115200" // UART2

#define CONNECT1 "serial/UART_2#baudrate=115200"
#define PUB_KEY "src/esp32/sys/message"
#define SUB_KEY "dst/esp32/sys/**"
#define VALUE "[ESPIDF]{ESP32}"

void data_handler(const z_sample_t *sample, void *arg) {
  z_owned_str_t keystr = z_keyexpr_to_string(sample->keyexpr);
  std::string val((const char *)sample->payload.start, sample->payload.len);

  printf(" >>>>>>>>>> [Subscription listener] Received (%s, %s)\n",
         z_str_loan(&keystr), val.c_str());

  /* Serial.print(" >> [Subscription listener] Received (");
   Serial.print(z_str_loan(&keystr));
   Serial.print(", ");
   Serial.print(val.c_str());
   Serial.println(")");*/

  z_str_drop(z_str_move(&keystr));
}

void nvs_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

extern "C" void app_main() {
  uint32_t sessions_ok = 0;
  uint32_t sessions_fail = 0;
  uint32_t publish_ok = 0;
  int erc = 0;
  nvs_init();
  while (true) {
    printf(" >>> Starting Zenoh-Pico Publisher...\n ");
    while (true) {
      uint32_t heap_before = esp_get_free_heap_size();
      printf("[%u]Opening Zenoh Session ... session_ok= %u sessions_fail=%u "
             "publish_ok=%u  \n",
             heap_before, sessions_ok, sessions_fail, publish_ok);
      z_owned_config_t config = z_config_default();
      zp_config_insert(z_config_loan(&config), Z_CONFIG_MODE_KEY,
                       z_string_make(MODE));
      zp_config_insert(z_config_loan(&config), Z_CONFIG_CONNECT_KEY,
                       z_string_make(CONNECT1));

      z_owned_session_t s;
      s = z_open(z_config_move(&config));
      if (!z_session_check(&s)) {
        printf("Unable to open session!\n");
        break;
      }
      printf("session open \n");

      // Start the receive and the session lease loop for zenoh-pico
      erc = zp_start_read_task(z_loan(s), NULL);
      if (erc != 0) {
        printf("Unable to start read task!\n");
        z_close(z_move(s));
        break;
      }

      erc = zp_start_lease_task(z_loan(s), NULL);
      if (erc != 0) {
        zp_stop_read_task(z_loan(s));
        printf("Unable to start lease task!\n");
        z_close(z_move(s));
        break;
      }

      uint32_t heap_size = esp_get_free_heap_size();

      z_owned_publisher_t pub =
          z_declare_publisher(z_loan(s), z_keyexpr(PUB_KEY), NULL);
      if (!z_check(pub)) {
        printf("Unable to declare publisher for key expression!\n");
        zp_stop_read_task(z_loan(s));
        zp_stop_lease_task(z_loan(s));
        break;
      }
      printf("publisher declared\n");

      printf("Declaring Subscriber on '%s'...\n", SUB_KEY);
      z_owned_closure_sample_t callback = z_closure(data_handler);
      z_owned_subscriber_t sub = z_declare_subscriber(
          z_loan(s), z_keyexpr(SUB_KEY), z_move(callback), NULL);
      if (!z_check(sub)) {
        printf("Unable to declare subscriber.\n");
        zp_stop_read_task(z_loan(s));
        zp_stop_lease_task(z_loan(s));
        break;
      }

      char buf[256];
      for (int idx = 0; idx < 100; ++idx) {
        sprintf(buf, "[%7d] heap=%u sessions_ok=%d sessions_fail=%d %s", idx,
                heap_size, sessions_ok, sessions_fail, VALUE);
        //       printf("Putting Data ('%s': '%s')...\n", KEYEXPR, buf);
        //      z_publisher_put_options_t options =
        // z_publisher_put_options_default();
        //        options.encoding.prefix = Z_ENCODING_PREFIX_TEXT_PLAIN;
        erc = z_publisher_put(z_loan(pub), (const uint8_t *)buf, strlen(buf),
                              NULL);
        if (erc != 0) {
          printf("Unable to publish data!\n");
          break;
        };
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        publish_ok += 1;
      }

      printf("Closing sessions_ok= %u sessions_fail=%u "
             "publish_ok=%u  \n",
             sessions_ok, sessions_fail, publish_ok);
      z_undeclare_publisher(z_move(pub));
      z_undeclare_subscriber(z_move(sub));

      vTaskDelay(100 / portTICK_PERIOD_MS); // wait flush ?
      erc = zp_stop_lease_task(z_loan(s));
      if (erc != 0) {
        printf("Unable to stop lease task!\n");
      }

      erc = zp_stop_read_task(z_loan(s));
      if (erc != 0) {
        printf("Unable to stop read task!\n");
      }

      z_close(z_move(s));
      vTaskDelay(100 / portTICK_PERIOD_MS); // wait flush ?
      sessions_ok++;
      uint32_t heap_after = esp_get_free_heap_size();
      printf("Heap before: %u, Heap after: %u delta %d \n", heap_before,
             heap_after, heap_before - heap_after);
    }
    sessions_fail++;
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
