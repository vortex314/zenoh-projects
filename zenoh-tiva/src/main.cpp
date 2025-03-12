//#include <Arduino.h>
#include <zenoh-pico.h>

// put function declarations here:

#define MODE "peer"
#define LOCATOR "serial/UART_1"
z_owned_session_t zenoh_session;
z_owned_publisher_t pub;
const char *keyexpr = "demo/example/zenoh-pico-pub";
int MAX_COUNT = 2147483647;  // max int value by default
const char *value = "Pub from Pico!";
char *buf = (char *)malloc(256);


#define LOOP_FOREVER  while (1) {;}

void setup() {
  Serial.begin(9600);

  z_owned_config_t config;
  z_config_default(&config);
  zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, MODE);
  if (strcmp(LOCATOR, "") != 0) {
      if (strcmp(MODE, "client") == 0) {
          zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, LOCATOR);
      } else {
          zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, LOCATOR);
      }
  }
  Serial.print("Opening Zenoh Session...");
  if (z_open(&zenoh_session, z_config_move(&config), NULL) < 0) {
      Serial.println("Unable to open session!");
     LOOP_FOREVER;
  }
  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  if (z_view_keyexpr_from_str(&ke, keyexpr) < 0) {
      Serial.printf("%s is not a valid key expression\n", keyexpr);
      LOOP_FOREVER;
  }
  if (z_declare_publisher(z_session_loan(&zenoh_session), &pub, z_view_keyexpr_loan(&ke), NULL) < 0) {
      Serial.printf("Unable to declare publisher for key expression!\n");
      LOOP_FOREVER;
  }
}

void loop() {

  z_clock_t now = z_clock_now();
  for (int idx = 0; idx < MAX_COUNT;) {
      if (z_clock_elapsed_ms(&now) > 1000) {
          snprintf(buf, 256, "[%4d] %s", idx, value);
          Serial.printf("Putting Data ('%s': '%s')...\n", keyexpr, buf);

          // Create payload
          z_owned_bytes_t payload;
          z_bytes_copy_from_str(&payload, buf);

          z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL);
          ++idx;

          now = z_clock_now();
      }

      zp_read(z_session_loan(&zenoh_session), NULL);
      zp_send_keep_alive(z_session_loan(&zenoh_session), NULL);
      zp_send_join(z_session_loan(&zenoh_session), NULL);
  }
/*
  z_publisher_drop(z_publisher_move(&pub));
  z_session_drop(z_session_move(&s));
  free(buf);
  Serial.println(" Hello world ");
  delay(1000);
  */
}

