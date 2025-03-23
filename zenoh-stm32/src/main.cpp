#include <zenoh-pico.h>
#include <sys.h>
#include <Serial.h>
#include <malloc.h>
// put function declarations here:

Log logger;

#define MODE "client"
#define LOCATOR "serial/UART_1#baudrate=115200"
const char *pub_topic = "src/stm32/zenoh-pico";
const char *sub_topic = "dst/stm32/**";
const char *value = "Pub from STM32 !";
z_owned_session_t *zenoh_session;
z_owned_publisher_t publisher;
z_view_keyexpr_t pub_keyexpr;
z_view_keyexpr_t sub_keyexpr;
z_owned_closure_sample_t callback;
z_owned_subscriber_t subscriber;

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

uint32_t msg_nb = 0;

void data_handler(z_loaned_sample_t *sample, void *ctx)
{
  (void)(ctx);
  z_view_string_t keystr;
  z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
  z_owned_string_t value;
  z_bytes_to_string(z_sample_payload(sample), &value);
  printf(">> [Subscriber] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
         z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)), z_string_data(z_string_loan(&value)));
  z_string_drop(z_string_move(&value));
  msg_nb++;
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

  res = z_view_keyexpr_from_str(&pub_keyexpr, pub_topic);
  if (res != Z_OK)
  {
    INFO("%s is not a valid key expression\n", pub_topic);
    LOOP_FOREVER;
  }
  res = z_declare_publisher(z_session_loan(zenoh_session), &publisher, z_view_keyexpr_loan(&pub_keyexpr), NULL);
  if (res != Z_OK)
  {
    INFO("Unable to declare publisher for key expression!\n");
    LOOP_FOREVER;
  }
  INFO("Publisher declared for key expression: %s\n", pub_topic);

  if (z_closure_sample(&callback, data_handler, NULL, NULL))
  {
    ERROR("z_closure_sample fails");
    LOOP_FOREVER;
  }
  printf("Declaring Subscriber on '%s'...\n", sub_topic);

  if (z_view_keyexpr_from_str(&sub_keyexpr, sub_topic) < 0)
  {
    ERROR("%s is not a valid key expression", sub_keyexpr);
    LOOP_FOREVER;
  }
  if (z_declare_subscriber(z_session_loan(zenoh_session), &subscriber, z_view_keyexpr_loan(&sub_keyexpr), z_closure_sample_move(&callback), NULL) < 0)
  {
    ERROR("Unable to declare subscriber");
    LOOP_FOREVER;
  }
}

extern char _end;      // Start of heap
extern char _estack;   // End of heap
extern char *__brkval; // Current heap pointer

void print_heap_info()
{
  struct mallinfo mi = mallinfo();
  INFO("Free heap: %d bytes / %d bytes", mi.fordblks, mi.arena);
}

void loop()
{
  static int idx = 0;
  INFO("Looping...");
  print_heap_info();
  delay(1000);

  snprintf(buf, 256, "[%4d] %s", idx++, value);
  INFO("Putting Data ('%s': '%s')", pub_topic, buf);

  // Create payload
  z_owned_bytes_t payload;
  z_bytes_copy_from_str(&payload, buf);

  // z_publisher_put(z_publisher_loan(&pub), , NULL);
  z_put_options_t *put_opts = new z_put_options_t;
  bzero(put_opts, sizeof(z_put_options_t));
  put_opts->congestion_control = Z_CONGESTION_CONTROL_BLOCK;

  z_put(z_session_loan(zenoh_session), z_view_keyexpr_loan(&pub_keyexpr), z_bytes_move(&payload), put_opts);
  zp_read(z_session_loan(zenoh_session), NULL);
  zp_send_keep_alive(z_session_loan(zenoh_session), NULL);
  // zp_send_join(z_session_loan(zenoh_session), NULL);
  z_free(put_opts);
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
  if (sys_init().is_err())
    panic_handler("sys_init failed");
  Serial2.begin(921600);
  Serial1.begin(115200);
  uint32_t count = 0;
  while (Serial1.available())
  {
    Serial1.read();
    count++;
  }
  INFO("Read %d bytes from Serial1", count);
  setup();
  while (1)
  {
    loop();
  }
  return 0;
}
