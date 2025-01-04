#ifndef MSG_WIFI_H
#define MSG_WIFI_H

#include <serdes.h>
#include <string>
#include <vector>
#include <optional>
#include <esp_wifi.h>
#include <esp_event.h>

#include <msg_info.h>

  const static InfoProp info_props_wifi[11] = {
    InfoProp(0, "mac_address", "MAC Address", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(1, "ip_address", "IP Address", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(2, "gateway", "Gateway", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(3, "netmask", "Netmask", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(4, "dns", "DNS", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(5, "ssid", "SSID", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(6, "channel", "Channel", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(7, "rssi", "RSSI", PropType::PROP_SINT, PropMode::PROP_READ),
    InfoProp(8, "encryption", "Encryption", PropType::PROP_STR, PropMode::PROP_READ),
    InfoProp(9, "wifi_mode", "Wifi Mode", PropType::PROP_UINT, PropMode::PROP_READ),
    InfoProp(10, "ap_scan", "AP Scan", PropType::PROP_ARRAY, PropMode::PROP_READ),
  };

struct WifiMsg : public Serializable {
  // WIFI & ethernet
  std::optional<std::string> mac_address;
  std::optional<std::string> ip_address;
  std::optional<std::string> gateway;
  std::optional<std::string> netmask;
  std::optional<std::string> dns;
  std::optional<std::string> ssid;
  std::optional<uint8_t> channel; // dynamic
  std::optional<int8_t> rssi;
  std::optional<std::string> encryption;
  std::optional<uint8_t> wifi_mode;
  std::optional<std::string> ap_scan;

  Res serialize(Serializer &ser) {
    int idx = 0;
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
  Res deserialize(Deserializer &des) {
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

  Res fill() {
    // get IP address and publish
    tcpip_adapter_ip_info_t ip_info;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    ip_address = ip4addr_ntoa(&ip_info.ip);
    gateway = ip4addr_ntoa(&ip_info.gw);
    netmask = ip4addr_ntoa(&ip_info.netmask);
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
    switch (ap_info.authmode) {
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
    return Res::Ok();
  }



  const InfoProp* info(int idx) {
    if ( idx >= sizeof(info_props_wifi)/sizeof(InfoProp)) return nullptr;
    return &info_props_wifi[idx];
  }
};

#endif // MSG_WIFI_H