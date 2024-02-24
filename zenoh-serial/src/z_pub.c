// #include <mbed.h>

#include <zenoh-pico.h>

#include <stdio.h>
#include <stdlib.h>

#define MODE "client"
#define CONNECT "serial//dev/ttyUSB1#baudrate=115200"
#define CONNECT2 "tcp/localhost:7447"


#define KEYEXPR "demo/example/zenoh-pico-pub"
#define VALUE "[MBedOS]{nucleo-F767ZI} Pub from Zenoh-Pico via Serial!"

int main(int argc, char **argv)
{

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = z_config_default();
    zp_config_insert(z_config_loan(&config), Z_CONFIG_MODE_KEY, z_string_make(MODE));
    zp_config_insert(z_config_loan(&config), Z_CONFIG_CONNECT_KEY, z_string_make(CONNECT2));

    // Open Zenoh session
    printf("Open session!\n");
    z_owned_session_t s = z_open(z_config_move(&config));
    if (!z_session_check(&s))
    {
        printf("Unable to open session!\n");
        return -1;
    }
    printf("OK\n");

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_session_loan(&s),NULL);
    zp_start_lease_task(z_session_loan(&s),NULL);

    printf("Declaring publisher for '%s'...", KEYEXPR);
    z_owned_publisher_t pub = z_declare_publisher(z_session_loan(&s), z_keyexpr(KEYEXPR), NULL);
    if (!z_publisher_check(&pub))
    {
        printf("Unable to declare publisher for key expression!\n");
        exit(-1);
    }
    printf("OK\n");

    char buf[7];
    for (int idx = 0; idx < 5000; ++idx)
    {
        //    z_sleep_s(1);
        sprintf(buf, "[%4d]", idx);
        printf("Putting Data ('%s': '%s')...\n", KEYEXPR, buf);
        z_publisher_put_options_t options = z_publisher_put_options_default();
        options.encoding.prefix = Z_ENCODING_PREFIX_TEXT_PLAIN;
        z_publisher_put(z_publisher_loan(&pub), (const uint8_t *)buf, strlen(buf), &options);
    }

    printf("\nPreparing to send burst of 1000 messages. Start in 2 seconds...\n");
    zp_sleep_s(2);

    for (int idx = 0; idx < 1000; ++idx)
    {
        sprintf(buf, "[%4d]", idx);
        printf(".");
        fflush(stdout);
        z_publisher_put_options_t options = z_publisher_put_options_default();
        options.encoding.prefix = Z_ENCODING_PREFIX_TEXT_PLAIN;
        z_publisher_put(z_publisher_loan(&pub), (const uint8_t *)buf, strlen(buf), &options);
    }
    printf("\n");

    printf("Closing Zenoh Session...");
    z_undeclare_publisher(z_publisher_move(&pub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_session_loan(&s));
    zp_stop_lease_task(z_session_loan(&s));

    z_close(z_session_move(&s));
    printf("OK!\n");

    return 0;
}