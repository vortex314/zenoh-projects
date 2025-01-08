#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <strings.h>
#include <wifi_actor.h>
#include <esp_mac.h>

static int s_retry_count = 0;
#define STRINGIFY(X) #X
#define S(X) STRINGIFY(X)
#define ESP_MAXIMUM_RETRY 5

WifiActor::WifiActor() : Actor<WifiEvent, WifiCmd>(4096, "wifi", 5, 10)
{
  INFO("Starting WiFi actor sizeof(WifiCmd ) : %d ", sizeof(WifiCmd));
  add_timer(Timer::Repetitive(1, 1000));
}

void WifiActor::on_start()
{
  wifi_init_sta();
}

void WifiActor::on_cmd(WifiCmd &cmd)
{
  if (cmd.stop_actor)
  {
    stop();
  }
}

void WifiActor::on_timer(int timer_id)
{
  switch (timer_id)
  {
  case 1:
  {
    wifi_msg.fill(esp_netif);
    emit(WifiEvent{.serdes = PublishSerdes{"wifi", wifi_msg}});
    break;
  }
  default:
    INFO("Unknown timer expired wifi");
  }
}

WifiActor::~WifiActor()
{
  INFO("Stopping WiFi actor");
}

void WifiActor::event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
  WifiActor *actor = (WifiActor *)arg;
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    actor->emit(WifiEvent{WifiSignal::WIFI_DISCONNECTED});
    actor->_wifi_connected = false;
    if (s_retry_count < ESP_MAXIMUM_RETRY)
    {
      esp_wifi_connect();
      s_retry_count++;
    }
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    actor->emit(WifiEvent{WifiSignal::WIFI_CONNECTED});
    actor->_wifi_connected = true;
    s_retry_count = 0;
  }
}

void WifiActor::wifi_init_sta(void)
{

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif = esp_netif_create_default_wifi_sta();

  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &handler_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, &handler_got_ip));

  wifi_config_t wifi_config;
  bzero(&wifi_config, sizeof(wifi_config));
  strcpy((char *)wifi_config.sta.ssid, "Merckx2");
  strcpy((char *)wifi_config.sta.password, S(WIFI_PASS));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

InfoProp info_props_wifi[] = {
    InfoProp(0, "mac_address", "MAC Address", PropType::PROP_STR,
             PropMode::PROP_READ),
    InfoProp(1, "ip_address", "IP Address", PropType::PROP_STR,
             PropMode::PROP_READ),
    InfoProp(2, "gateway", "Gateway", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(3, "netmask", "Netmask", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(4, "dns", "DNS", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(5, "ssid", "SSID", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(6, "channel", "Channel", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(7, "rssi", "RSSI", PropType::PROP_SINT, PropMode::PROP_READ),
    InfoProp(8, "encryption", "Encryption", PropType::PROP_STR,
             PropMode::PROP_READ),
    InfoProp(9, "wifi_mode", "Wifi Mode", PropType::PROP_UINT,
             PropMode::PROP_READ),
    InfoProp(10, "ap_scan", "AP Scan", PropType::PROP_ARRAY,
             PropMode::PROP_READ),
};

Res WifiMsg::serialize(Serializer &ser)
{
  uint32_t idx = 0;
  ser.reset();
  ser.serialize(idx++, mac_address);
  ser.serialize(idx++, ip_address);
  ser.serialize(idx++, gateway);
  ser.serialize(idx++, netmask);
  ser.serialize(idx++, dns);
  ser.serialize(idx++, ssid);
  ser.serialize(idx++, channel);
  ser.serialize(idx++, rssi);
  ser.serialize(idx++, encryption);
  ser.serialize(idx++, wifi_mode);
  return ser.serialize(idx++, ap_scan);
}
Res WifiMsg::deserialize(Deserializer &des)
{
  des.deserialize(mac_address);
  des.deserialize(ip_address);
  des.deserialize(gateway);
  des.deserialize(netmask);
  des.deserialize(dns);
  des.deserialize(ssid);
  des.deserialize(channel);
  des.deserialize(rssi);
  des.deserialize(encryption);
  des.deserialize(wifi_mode);
  return des.deserialize(ap_scan);
}

Res WifiMsg::fill(esp_netif_t *esp_netif)
{
  // get IP address and publish
  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif, &ip_info);
  ip_address = ip4addr_to_str(&ip_info.ip);
  gateway = ip4addr_to_str(&ip_info.gw);
  netmask = ip4addr_to_str(&ip_info.netmask);
  // get MAC address
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
  mac_address = macStr;
  // get SSID
  wifi_config_t wifi_config;
  esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
  ssid = (char *)wifi_config.sta.ssid;
  // get channel
  wifi_ap_record_t ap_info;
  esp_wifi_sta_get_ap_info(&ap_info);
  channel = ap_info.primary;
  // get RSSI
  rssi = ap_info.rssi;
  // get encryption
  switch (ap_info.authmode)
  {
  case WIFI_AUTH_OPEN:
    encryption = "OPEN";
    break;
  case WIFI_AUTH_WEP:
    encryption = "WEP";
    break;
  case WIFI_AUTH_WPA_PSK:
    encryption = "WPA_PSK";
    break;
  case WIFI_AUTH_WPA2_PSK:
    encryption = "WPA2_PSK";
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:

    encryption = "WPA_WPA2_PSK";
    break;
  default:
    encryption = "UNKNOWN";
    break;
  }
  // get wifi mode
  wifi_mode = 0;
  ap_scan = "scan";
  return Res::Ok();
}

const InfoProp *WifiMsg::info(int idx)
{
  if (idx >= sizeof(info_props_wifi) / sizeof(InfoProp))
    return nullptr;
  return &info_props_wifi[idx];
}

/*

  static int idx_wifi = 0;
  const InfoProp *prop_wifi = wifi_msg.info(idx_wifi);
  if (prop_wifi == NULL) {
    idx_wifi = 0;
    prop_wifi = wifi_msg.info(idx_wifi);
  }
  publish_topic_value("info/wifi", *(InfoProp *)prop_wifi);
*/

std::string ip4addr_to_str(esp_ip4_addr_t *ip)
{
  char buf[16];
  uint8_t *ip_parts = (uint8_t *)ip;
  snprintf(buf, 16, "%u.%u.%u.%u",
           ip_parts[0], ip_parts[1],
           ip_parts[2], ip_parts[3]);
  return buf;
}

/* Scan Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
    This example shows how to scan for available set of APs.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "regex.h"

#define DEFAULT_SCAN_LIST_SIZE 10

#ifdef CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP
#define USE_CHANNEL_BITMAP 1
#define CHANNEL_LIST_SIZE 3
static uint8_t channel_list[CHANNEL_LIST_SIZE] = {1, 6, 11};
#endif /*CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP*/

static const char *TAG = "scan";

static void print_auth_mode(int authmode)
{
  switch (authmode)
  {
  case WIFI_AUTH_OPEN:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
    break;
  case WIFI_AUTH_OWE:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
    break;
  case WIFI_AUTH_WEP:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
    break;
  case WIFI_AUTH_WPA_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
    break;
  case WIFI_AUTH_WPA2_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
    break;
  case WIFI_AUTH_ENTERPRISE:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
    break;
  case WIFI_AUTH_WPA3_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA3_ENT_192:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
    break;
  default:
    ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
    break;
  }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
  switch (pairwise_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_AES_CMAC128:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }

  switch (group_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }
}

#ifdef USE_CHANNEL_BITMAP
static void array_2_channel_bitmap(const uint8_t channel_list[], const uint8_t channel_list_size, wifi_scan_config_t *scan_config)
{

  for (uint8_t i = 0; i < channel_list_size; i++)
  {
    uint8_t channel = channel_list[i];
    scan_config->channel_bitmap.ghz_2_channels |= (1 << channel);
  }
}
#endif /*USE_CHANNEL_BITMAP*/

/* Initialize Wi-Fi as sta and set scan method */
Result<std::string> WifiActor::scan()
{
  //    ESP_ERROR_CHECK(esp_netif_init());
  //    ESP_ERROR_CHECK(esp_event_loop_create_default());
  //    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  //     assert(sta_netif);

  //    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  //    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  uint16_t number = DEFAULT_SCAN_LIST_SIZE;
  wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
  uint16_t ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));

  //   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

#ifdef USE_CHANNEL_BITMAP
  wifi_scan_config_t *scan_config = (wifi_scan_config_t *)calloc(1, sizeof(wifi_scan_config_t));
  if (!scan_config)
  {
    ESP_LOGE(TAG, "Memory Allocation for scan config failed!");
    return;
  }
  array_2_channel_bitmap(channel_list, CHANNEL_LIST_SIZE, scan_config);
  esp_wifi_scan_start(scan_config, true);
  free(scan_config);

#else
  esp_wifi_scan_start(NULL, true);
#endif /*USE_CHANNEL_BITMAP*/

  ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
  int8_t rssi_highest = -128;
  int ap_index = 0;
  for (int i = 0; i < number; i++)
  {
    ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
    ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
    print_auth_mode(ap_info[i].authmode);
    if (ap_info[i].authmode != WIFI_AUTH_WEP)
    {
      print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
    }
    if (ap_info[i].rssi > rssi_highest)
    {
      rssi_highest = ap_info[i].rssi;
      ap_index = i;
    }
    ESP_LOGI(TAG, "Channel \t\t%d", ap_info[i].primary);
  }
  ESP_LOGI(TAG, "Highest RSSI = %d, AP index = %d", rssi_highest, ap_index);
  return Result<std::string>::Ok(std::string((const char *)ap_info[ap_index].ssid));
}
