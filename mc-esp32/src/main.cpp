#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "cJSON.h"

#define BZERO(x) memset(&(x), 0, sizeof(x))
#define T_ESP(VAL)                                                                       \
  {                                                                                      \
    auto r = (VAL);                                                                      \
    if (r != ESP_OK)                                                                     \
    {                                                                                    \
      printf("[%d] %s:%d %s = %s ", r, __FILE__, __LINE__, #VAL, esp_err_to_name(r)); \
      return r;                                                                          \
    }                                                                                    \
  }

static const char *TAG = "UDP_MULTICAST";

// Multicast configuration
#define MULTICAST_IP "225.0.0.1"
#define MULTICAST_PORT 6502
#define MAX_UDP_PACKET_SIZE 1024

static int sock = -1;

// Function to create multicast socket
static int create_multicast_socket()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create socket: errno %d : %s", errno, strerror(errno));
        return -1;
    }

    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind the socket to any address
    struct sockaddr_in saddr;
    BZERO(saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MULTICAST_PORT);
    saddr.sin_addr = (struct in_addr){
        .s_addr = htonl(INADDR_ANY) // Bind to all interfaces
    };

   T_ESP(bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)));

    // Get the actual network interface IP address
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get network interface");
        close(sock);
        return -1;
    }
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get IP info");
        close(sock);
        return -1;
    }
    // Join multicast group
    struct ip_mreq mreq ;
    BZERO(mreq);
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
    mreq.imr_interface.s_addr = ip_info.ip.addr; // Use the actual interface IP address
    // mreq.imr_interface.s_addr = htonl(INADDR_ANY); // Use INADDR_ANY to join on all interfaces

    int rc = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (rc < 0)
    {
        ESP_LOGE(TAG, "Failed to join multicast group: errno %d : %s", rc, strerror(errno));
        close(sock);
        return -1;
    }

    // Set multicast interface
    struct in_addr if_addr;
    if_addr.s_addr = htonl(INADDR_ANY);
    T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_addr, sizeof(if_addr)));
    // Enable loopback so we receive our own packets (for testing)
    int loop = 1;
    T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)));

    // Set multicast TTL (time to live)
    int ttl = 32;
    T_ESP(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)));


    return sock;
}

// Function to send multicast JSON message
static void send_multicast_json(int sock, const char *json_str)
{
    struct sockaddr_in dest_addr ;
    BZERO(dest_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(MULTICAST_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);


    int err = sendto(sock, json_str, strlen(json_str), 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    else
    {
        ESP_LOGI(TAG, "Message sent: %s", json_str);
    }
}

// Function to receive multicast messages
static void receive_multicast_messages(void *pvParameters)
{
    char rx_buffer[MAX_UDP_PACKET_SIZE];
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    while (1)
    {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);

        if (len < 0)
        {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }
        else
        {
            rx_buffer[len] = '\0'; // Null-terminate the received data
            printf("Received %s\n", rx_buffer);

            // Parse JSON
            cJSON *json = cJSON_Parse(rx_buffer);
            if (json == NULL)
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    ESP_LOGE(TAG, "JSON parse error before: %s", error_ptr);
                }
            }
            else
            {
                // Print the received JSON
                char *json_str = cJSON_Print(json);
                ESP_LOGI(TAG, "Received JSON from %s:%d: %s",
                         inet_ntoa(source_addr.sin_addr), ntohs(source_addr.sin_port), json_str);
                free(json_str);

                // You can access JSON fields here
                // Example:
                // cJSON *field = cJSON_GetObjectItemCaseSensitive(json, "field_name");
                // if (cJSON_IsString(field) && (field->valuestring != NULL)) {
                //     printf("field_name: %s\n", field->valuestring);
                // }

                cJSON_Delete(json);
            }
        }
    }

    vTaskDelete(NULL);
}

// Function to create a sample JSON message
static char *create_sample_json()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device", "ESP32");
    cJSON_AddStringToObject(root, "type", "multicast_demo");
    cJSON_AddNumberToObject(root, "value", 42);
    cJSON_AddBoolToObject(root, "status", true);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}
/*
extern "C" void app_main()
{
    ESP_LOGI(TAG, "Starting UDP Multicast JSON Demo");

    // Initialize network stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create multicast socket
    sock = create_multicast_socket();
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create multicast socket");
        return;
    }

    // Create receive task
    xTaskCreate(receive_multicast_messages, "udp_rx", 4096, NULL, 5, NULL);

    // Main loop to send multicast messages periodically
    while (1)
    {
        char *json_str = create_sample_json();
        send_multicast_json(sock, json_str);
        free(json_str);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Send every 5 seconds
    }
}*/

// ...existing code...
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID      "Merckx2"
#define WIFI_PASS      "LievenMarletteEwoutRonald"
#define WIFI_MAX_RETRY 5

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}
// ...existing code...

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Starting UDP Multicast JSON Demo");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    wifi_init_sta();

    // Create multicast socket
    sock = create_multicast_socket();
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create multicast socket");
        return;
    }

    // Create receive task
    xTaskCreate(receive_multicast_messages, "udp_rx", 4096, NULL, 5, NULL);

    // Main loop to send multicast messages periodically
    while (1)
    {
        char *json_str = create_sample_json();
        send_multicast_json(sock, json_str);
        free(json_str);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Send every 5 seconds
    }
}
// ...existing code...