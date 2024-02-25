

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
#define KEYEXPR "demo/example/zenoh-pico-pub"
#define VALUE "[ESPIDF]{ESP32} Publication from Zenoh-Pico!"

void data_handler(const z_sample_t *sample, void *arg) {
  z_owned_str_t keystr = z_keyexpr_to_string(sample->keyexpr);
  std::string val((const char *)sample->payload.start, sample->payload.len);

  printf(" >> [Subscription listener] Received (%s, %s)\n",
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
  nvs_init();
  while (true) {
    printf(" >>> Starting Zenoh-Pico Publisher...\n ");
    while (true) {
      printf("Opening Zenoh Session...  \n");
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
      printf("OK\n");

      // Start the receive and the session lease loop for zenoh-pico
      int erc = zp_start_read_task(z_loan(s), NULL);
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

      printf("Declaring publisher for '%s'...", KEYEXPR);
      z_owned_publisher_t pub =
          z_declare_publisher(z_loan(s), z_keyexpr(KEYEXPR), NULL);
      if (!z_check(pub)) {
        printf("Unable to declare publisher for key expression!\n");
        zp_stop_read_task(z_loan(s));
        zp_stop_lease_task(z_loan(s));
        break;
      }
      printf("OK\n");

      char buf[256];
      for (int idx = 0; 1; ++idx) {
        sleep(1);
        sprintf(buf, "[%4d] %s", idx, VALUE);
        printf("Putting Data ('%s': '%s')...\n", KEYEXPR, buf);
        z_publisher_put_options_t options = z_publisher_put_options_default();
        options.encoding.prefix = Z_ENCODING_PREFIX_TEXT_PLAIN;
        z_publisher_put(z_loan(pub), (const uint8_t *)buf, strlen(buf), NULL);
      }

      printf("Closing Zenoh Session...");
      z_undeclare_publisher(z_move(pub));
      zp_stop_read_task(z_loan(s));
      zp_stop_lease_task(z_loan(s));
      z_close(z_move(s));
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
