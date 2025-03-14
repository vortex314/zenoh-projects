#include <zenoh-pico.h>
#include <sys.h>
// put function declarations here:

Log logger;


#define MODE "peer"
#define LOCATOR "serial/UART_1#baudrate=115200"
z_owned_session_t zenoh_session;
z_owned_publisher_t pub;
const char *keyexpr = "demo/example/zenoh-pico-pub";
int MAX_COUNT = 2147483647; // max int value by default
const char *value = "Pub from Pico!";
char *buf = (char *)malloc(256);

#define LOOP_FOREVER \
  while (1)          \
  {                  \
    ;                \
  }

#define Z_CHECK(expr)                                   \
  {                                                     \
    z_result_t res = (expr);                            \
    if (res != Z_OK)                                    \
    {                                                   \
      INFO("Error: " #expr " rc: %d \n", res); \
      LOOP_FOREVER;                                     \
    }                                                   \
  }

void setup()
{
  INFO("\nZenoh-Pico Publisher Example");
  z_sleep_ms(1000);

  z_result_t res;
  z_owned_config_t config;
  z_config_default(&config);
  zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, MODE);
  zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, LOCATOR);

  INFO("Opening Zenoh Session...");
  z_sleep_ms(1000);

  Z_CHECK(z_open(&zenoh_session, z_config_move(&config), NULL));
  z_sleep_ms(1000);
  INFO("Done!\n");

  z_owned_publisher_t pub;
  z_view_keyexpr_t ke;
  res = z_view_keyexpr_from_str(&ke, keyexpr);
  if (res != Z_OK)
  {
    INFO("%s is not a valid key expression\n", keyexpr);
    LOOP_FOREVER;
  }
  res = z_declare_publisher(z_session_loan(&zenoh_session), &pub, z_view_keyexpr_loan(&ke), NULL);
  if (res != Z_OK)
  {
    INFO("Unable to declare publisher for key expression!\n");
    LOOP_FOREVER;
  }
}

void loop()
{

  z_clock_t now = z_clock_now();
  for (int idx = 0; idx < MAX_COUNT;)
  {
    if (z_clock_elapsed_ms(&now) > 1000)
    {
      snprintf(buf, 256, "[%4d] %s", idx, value);
      INFO("Putting Data ('%s': '%s')...\n", keyexpr, buf);

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
    printfln(" Hello world ");
    delay(1000);
    */
}

int main() {
  setup();
  while(1){
    loop();
  }
}
