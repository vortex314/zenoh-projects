#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <strings.h>
#include <wifi_actor.h>
#include <esp_mac.h>

#ifndef WIFI_PASS
#error "WIFI_PASS not defined"
#endif

#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif

static int s_retry_count = 0;
#define STRINGIFY(X) #X
#define S(X) STRINGIFY(X)
#define ESP_MAXIMUM_RETRY 5

WifiActor::WifiActor() : Actor<WifiEvent, WifiCmd>(4000, "wifi", 5, 10)
{
  INFO("Starting WiFi actor sizeof(WifiCmd ) : %d ", sizeof(WifiCmd));
  add_timer(Timer::Repetitive(1, 1000));
  wifi_ssid = "Merckx2";
  wifi_password = S(WIFI_PASS);
}

void WifiActor::on_start()
{
  if (net_init().is_err())
  {
    INFO("Failed to init net");
    return;
  }
  while (true)
  {
    auto r = scan();
    if (r.is_ok())
    {
      INFO("Scanned SSID: %s", wifi_ssid.c_str());
      break;
    }
    else
    {
      INFO("Failed to scan: %s", r.msg().c_str());
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  wifi_ssid = S(WIFI_SSID);
  wifi_password = S(WIFI_PASS);
  if (wifi_init_sta().is_err())
  {
    INFO("Failed to init wifi");
    return;
  }
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
    if (_wifi_connected)
    {
      wifi_msg.fill(esp_netif);
      emit(WifiEvent{.serdes = PublishSerdes{"wifi", wifi_msg}});
    }
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
    INFO("WiFi STA started");
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT &&
           event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    INFO("WiFi STA disconnected");
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
    INFO("WiFi STA got IP address");
    actor->emit(WifiEvent{WifiSignal::WIFI_CONNECTED});
    actor->_wifi_connected = true;
    s_retry_count = 0;
  }
}

Res WifiActor::net_init(void)
{
  CHECK(esp_netif_init());
  CHECK(esp_event_loop_create_default());
  esp_netif = esp_netif_create_default_wifi_sta();
  assert(esp_netif);
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  CHECK(esp_wifi_init(&config));

  CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &handler_any_id));
  CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, &handler_got_ip));
  CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  CHECK(esp_wifi_start());
  return Res::Ok();
}

Res WifiActor::wifi_init_sta(void)
{
  wifi_config_t wifi_config;
  bzero(&wifi_config, sizeof(wifi_config));
  strcpy((char *)wifi_config.sta.ssid, wifi_ssid.c_str());
  strcpy((char *)wifi_config.sta.password, wifi_password.c_str());

  
  DEBUG("Setting WiFi configuration SSID '%s' PSWD '%s'", wifi_config.sta.ssid,
       wifi_config.sta.password);
  // CHECK(esp_wifi_stop());
  CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  // CHECK(esp_wifi_start());
  INFO("Connecting to WiFi network...");
  return Res::Ok();
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
  ser.begin_map();
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
  ser.serialize(idx++, ap_scan);
  return ser.end_map();
}
Res WifiMsg::deserialize(Deserializer &des)
{
  return Res::Err(-1, "Not implemented");
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

static const char *authmode_to_str(int authmode)
{
  switch (authmode)
  {
  case WIFI_AUTH_OPEN:
    return ("Authmode WIFI_AUTH_OPEN");
    break;
  case WIFI_AUTH_OWE:
    return ("Authmode WIFI_AUTH_OWE");
    break;
  case WIFI_AUTH_WEP:
    return ("Authmode WIFI_AUTH_WEP");
    break;
  case WIFI_AUTH_WPA_PSK:
    return ("Authmode WIFI_AUTH_WPA_PSK");
    break;
  case WIFI_AUTH_WPA2_PSK:
    return ("Authmode WIFI_AUTH_WPA2_PSK");
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:
    return ("Authmode WIFI_AUTH_WPA_WPA2_PSK");
    break;
  case WIFI_AUTH_ENTERPRISE:
    return ("Authmode WIFI_AUTH_ENTERPRISE");
    break;
  case WIFI_AUTH_WPA3_PSK:
    return ("Authmode WIFI_AUTH_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return ("Authmode WIFI_AUTH_WPA2_WPA3_PSK");
    break;
  case WIFI_AUTH_WPA3_ENT_192:
    return ("Authmode WIFI_AUTH_WPA3_ENT_192");
    break;
  default:
    return ("Authmode WIFI_AUTH_UNKNOWN");
    break;
  }
}

static const char *cipher_type_to_str(int pairwise_cipher)
{
  switch (pairwise_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_AES_CMAC128:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_AES_CMAC128");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    return ("Pairwise Cipher WIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }
}

static const char *group_cipher_to_str(int group_cipher)
{
  switch (group_cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    return ("Group Cipher WIFI_(sta_netif);CIPHER_TYPE_NONE");
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    return ("Group Cipher WIFI_CIPHER_TYPE_WEP40");
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    return ("Group Cipher WIFI_CIPHER_TYPE_WEP104");
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    return ("Group Cipher WIFI_CIPHER_TYPE_TKIP");
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    return ("Group Cipher WIFI_CIPHER_TYPE_CCMP");
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    return ("Group Cipher WIFI_CIPHER_TYPE_TKIP_CCMP");
    break;
  case WIFI_CIPHER_TYPE_SMS4:
    return ("Group Cipher WIFI_CIPHER_TYPE_SMS4");
    break;
  case WIFI_CIPHER_TYPE_GCMP:
    return ("Group Cipher WIFI_CIPHER_TYPE_GCMP");
    break;
  case WIFI_CIPHER_TYPE_GCMP256:
    return ("Group Cipher WIFI_CIPHER_TYPE_GCMP256");
    break;
  default:
    return ("Group Cipher WIFI_CIPHER_TYPE_UNKNOWN");
    break;
  }
}

#ifdef USE_CHANNEL_BITMAPassert
static void array_2_channel_bitmap(const uint8_t channel_list[], const uint8_t channel_list_size, wifi_scan_config_t *scan_config)
{

  for (uint8_t i = 0; i < channel_list_size; i++)
  {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    uint8_t channel = channel_list[i];
    scan_config->channel_bitmap.ghz_2_channels |= (1 << channel);
  }
}
#endif /*USE_CHANNEL_BITMAP*/

/* Initialize Wi-Fi as sta and set scan method */
Res WifiActor::scan()
{

  uint16_t number = DEFAULT_SCAN_LIST_SIZE;
  wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
  uint16_t ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));

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
  CHECK(esp_wifi_scan_start(NULL, true));
#endif /*USE_CHANNEL_BITMAP*/

  INFO("Max AP number ap_info can hold = %u", number);
  CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  INFO("Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
  int8_t rssi_highest = -128;
  int ap_index = 0;
  for (int i = 0; i < number; i++)
  {
    INFO("Channel %d SSID '%s' RSSI %d %s %s %",
         ap_info[i].primary,
         ap_info[i].ssid,
         ap_info[i].rssi, authmode_to_str(ap_info[i].authmode),
         ap_info[i].authmode != WIFI_AUTH_WEP ? "" : cipher_type_to_str(ap_info[i].pairwise_cipher),
         ap_info[i].authmode != WIFI_AUTH_WEP ? "" : group_cipher_to_str(ap_info[i].group_cipher),
         group_cipher_to_str(ap_info[i].group_cipher));

    std::string ssid = std::string((const char *)ap_info[i].ssid);
    if (ap_info[i].rssi > rssi_highest && ssid.rfind("Merckx", 0) == 0)
    {
      rssi_highest = ap_info[i].rssi;
      ap_index = i;
    }
  }
  INFO("Highest RSSI = %d, AP index = %d", rssi_highest, ap_index);
  wifi_ssid = std::string((const char *)ap_info[ap_index].ssid);
  return Res::Ok();
}
