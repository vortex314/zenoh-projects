#include <zenoh-pico.h>
#include <sys.h>
#include <Serial.h>
// put function declarations here:

Log logger;

#define MODE "peer"
#define LOCATOR "serial/UART_1#baudrate=115200"
z_owned_session_t *zenoh_session;
z_owned_publisher_t pub;
const char *keyexpr = "src/stm32/zenoh-pico";
int MAX_COUNT = 2147483647; // max int value by default
const char *value = "Pub from STM32 !";
char *buf = (char *)malloc(256);

#define LOOP_FOREVER \
  while (1)          \
  {                  \
    ;                \
  }

#define Z_CHECK(expr)                          \
  {                                            \
    z_result_t res = (expr);                   \
    if (res != Z_OK)                           \
    {                                          \
      INFO("Error: " #expr " rc: %d \n", res); \
      LOOP_FOREVER;                            \
    }                                          \
  }

void setup()
{

  INFO("Zenoh-Pico Publisher Example " __DATE__ " " __TIME__);

  z_result_t res;
  while (true)
  {
    z_owned_config_t *config = new z_owned_config_t;
    zenoh_session = new z_owned_session_t;

    z_config_default(config);
    zp_config_insert(z_config_loan_mut(config), Z_CONFIG_MODE_KEY, MODE);
    zp_config_insert(z_config_loan_mut(config), Z_CONFIG_CONNECT_KEY, LOCATOR);

    INFO("Opening Zenoh Session...");

    if (z_open(zenoh_session, z_config_move(config), NULL) == 0)
    {
      break;
    }
    else
    {
      INFO("Failed to open session, retrying...");
      z_sleep_ms(10000);
    }
  }
  INFO(" Zenoh Session Opened!");
  z_sleep_ms(100);
  INFO("Done!");

  z_view_keyexpr_t ke;
  res = z_view_keyexpr_from_str(&ke, keyexpr);
  if (res != Z_OK)
  {
    INFO("%s is not a valid key expression\n", keyexpr);
    LOOP_FOREVER;
  }
  res = z_declare_publisher(z_session_loan(zenoh_session), &pub, z_view_keyexpr_loan(&ke), NULL);
  if (res != Z_OK)
  {
    INFO("Unable to declare publisher for key expression!\n");
    LOOP_FOREVER;
  }
  INFO("Publisher declared for key expression: %s\n", keyexpr);
}

void loop()
{
  static int idx = 0;
  INFO("Looping...");
  delay(1000);

  snprintf(buf, 256, "[%4d] %s", idx++, value);
  INFO("Putting Data ('%s': '%s')...\n", keyexpr, buf);

  // Create payload
  z_owned_bytes_t payload;
  z_bytes_copy_from_str(&payload, buf);

  z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&payload), NULL);
  zp_read(z_session_loan(zenoh_session), NULL);
  zp_send_keep_alive(z_session_loan(zenoh_session), NULL);
  zp_send_join(z_session_loan(zenoh_session), NULL);

  /*
    z_publisher_drop(z_publisher_move(&pub));
    z_session_drop(z_session_move(&s));
    free(buf);
    printfln(" Hello world ");
    delay(1000);
    */
}

int main()
{
  sys_init();
  Serial1.begin(921600);
  Serial0.begin(115200);
  setup();
  while (1)
  {
    loop();
  }
  return 0;
}
