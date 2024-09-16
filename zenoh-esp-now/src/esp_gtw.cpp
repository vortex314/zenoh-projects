#include "esp_gtw.h"

static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t CONFIG_ESPNOW_PMK[16] = {0};
const int CONFIG_ESPNOW_CHANNEL = 1;
const int CONFIG_ESPNOW_SEND_COUNT = 100;
const int CONFIG_ESPNOW_SEND_DELAY = 1000;
const int CONFIG_ESPNOW_SEND_LEN = 100;
const int CONFIG_ESPNOW_WAKE_INTERVAL = 0;
const int CONFIG_ESPNOW_WAKE_WINDOW = 0;
const bool CONFIG_ESPNOW_ENABLE_LONG_RANGE = false;
const wifi_interface_t ESPNOW_WIFI_IF = WIFI_IF_STA;

const char* TAG = "gtw_espnow";

static esp_err_t gtw_espnow_init(void);
static esp_err_t gtw_wifi_init(void);

Log logger(10);

Result<Void> EspGtw::init() {
    /*if (gtw_wifi_init() != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to initialize WiFi");
    }*/
    if (gtw_espnow_init() != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to initialize ESP-NOW");
    }
    return {};
}
Result<Void> EspGtw::set_callback_receive(void (*func)(const uint8_t* mac_addr,
                                                       const uint8_t* data,
                                                       int len)) {
    if (esp_now_register_recv_cb(func) != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to set receive callback");
    }
    return Result<Void>::Ok(Void());
}
Result<Void> EspGtw::set_pmk() {
    if (esp_now_set_pmk((uint8_t*)CONFIG_ESPNOW_PMK) != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to set PMK");
    }
    return {};
}
/*
    add broadcast address to possible peers, no arg ??? broadcast address ia added elsewhere
*/
Result<Void> EspGtw::add_peer() {
    esp_now_peer_info_t* peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        return Result<Void>::Err(ENOMEM, "Failed to allocate memory for peer");
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = 0;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    if (esp_now_add_peer(peer) != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to add peer");
    }
    free(peer);
    return {};
}
Result<Void> EspGtw::send(const uint8_t* data, int len) {
    if (esp_now_send(broadcast_mac, data, len) != ESP_OK) {
        return Result<Void>::Err(EINVAL, "Failed to send data");
    }
    return {};
}
Result<Void> EspGtw::deinit() {
    esp_now_deinit();
    return {};
}

static void gtw_espnow_send_cb(const uint8_t* mac_addr, esp_now_send_status_t status) {}

static void gtw_espnow_recv_cb(const uint8_t* mac_addr, const uint8_t* data, int len) {}

static esp_err_t gtw_espnow_init(void) {
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(gtw_espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(gtw_espnow_recv_cb));
#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK(esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW));
    ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL));
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t*)CONFIG_ESPNOW_PMK));

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t* peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = 0;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);
    return ESP_OK;
}

void gtw_espnow_deinit() {
    esp_now_deinit();
}

static esp_err_t gtw_wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(
        ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif
    return ESP_OK;
}