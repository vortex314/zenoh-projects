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
    emit(WifiEvent{.serdes = PublishSerdes {"wifi",wifi_msg }});
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
