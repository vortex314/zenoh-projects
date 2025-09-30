#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <actor.h>


typedef std::vector<uint8_t> Bytes;


typedef enum {
    SYS_CMD = 1,
    SYS_INFO = 2,
    WIFI_INFO = 3,
    MOTOR_INFO = 4,
    MOTOR_CMD = 5,
} MessageType;

typedef enum {
    OFF = 0,
    ON = 1,
} Toggle;



class SysCmd : public Msg {
    public:
    static constexpr const char *id = "SysCmd";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 51983;

    std::string dst;
    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["SysCmd"].to<JsonObject>();
        obj["dst"] = dst;
        obj["src"] = src;
        if (set_time)  obj["set_time"] = *set_time;
        if (reboot)  obj["reboot"] = *reboot;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        
        if (obj["set_time"].is<uint64_t>() )  set_time = obj["set_time"].as<uint64_t>();
        if (obj["reboot"].is<bool>() )  reboot = obj["reboot"].as<bool>();
        }

};

class SysInfo : public Msg {
    public:
    static constexpr const char *id = "SysInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 10347;

    std::string src;
    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_heap;
    std::optional<uint64_t> flash;
    std::optional<std::string> cpu_board;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["SysInfo"].to<JsonObject>();
        obj["src"] = src;
        if (uptime)  obj["uptime"] = *uptime;
        if (free_heap)  obj["free_heap"] = *free_heap;
        if (flash)  obj["flash"] = *flash;
        if (cpu_board)  obj["cpu_board"] = *cpu_board;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        if (obj["uptime"].is<uint64_t>() )  uptime = obj["uptime"].as<uint64_t>();
        if (obj["free_heap"].is<uint64_t>() )  free_heap = obj["free_heap"].as<uint64_t>();
        if (obj["flash"].is<uint64_t>() )  flash = obj["flash"].as<uint64_t>();
        if (obj["cpu_board"].is<std::string>() )  cpu_board = obj["cpu_board"].as<std::string>();
        }

};

class WifiInfo : public Msg {
    public:
    static constexpr const char *id = "WifiInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 15363;

    std::string src;
    std::optional<std::string> ssid;
    std::optional<std::string> bssid;
    std::optional<int32_t> rssi;
    std::optional<std::string> ip;
    std::optional<std::string> mac;
    std::optional<int32_t> channel;
    std::optional<std::string> gateway;
    std::optional<std::string> netmask;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["WifiInfo"].to<JsonObject>();
        obj["src"] = src;
        if (ssid)  obj["ssid"] = *ssid;
        if (bssid)  obj["bssid"] = *bssid;
        if (rssi)  obj["rssi"] = *rssi;
        if (ip)  obj["ip"] = *ip;
        if (mac)  obj["mac"] = *mac;
        if (channel)  obj["channel"] = *channel;
        if (gateway)  obj["gateway"] = *gateway;
        if (netmask)  obj["netmask"] = *netmask;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        if (obj["ssid"].is<std::string>() )  ssid = obj["ssid"].as<std::string>();
        if (obj["bssid"].is<std::string>() )  bssid = obj["bssid"].as<std::string>();
        if (obj["rssi"].is<int32_t>() )  rssi = obj["rssi"].as<int32_t>();
        if (obj["ip"].is<std::string>() )  ip = obj["ip"].as<std::string>();
        if (obj["mac"].is<std::string>() )  mac = obj["mac"].as<std::string>();
        if (obj["channel"].is<int32_t>() )  channel = obj["channel"].as<int32_t>();
        if (obj["gateway"].is<std::string>() )  gateway = obj["gateway"].as<std::string>();
        if (obj["netmask"].is<std::string>() )  netmask = obj["netmask"].as<std::string>();
        }

};

class MulticastInfo : public Msg {
    public:
    static constexpr const char *id = "MulticastInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 61310;

    std::string src;
    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["MulticastInfo"].to<JsonObject>();
        obj["src"] = src;
        if (group)  obj["group"] = *group;
        if (port)  obj["port"] = *port;
        if (mtu)  obj["mtu"] = *mtu;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        if (obj["group"].is<std::string>() )  group = obj["group"].as<std::string>();
        if (obj["port"].is<int32_t>() )  port = obj["port"].as<int32_t>();
        if (obj["mtu"].is<uint32_t>() )  mtu = obj["mtu"].as<uint32_t>();
        }

};

class HoverboardInfo : public Msg {
    public:
    static constexpr const char *id = "HoverboardInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 59150;

    std::string src;
    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    std::optional<int32_t> currentA;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["HoverboardInfo"].to<JsonObject>();
        obj["src"] = src;
        if (speed)  obj["speed"] = *speed;
        if (direction)  obj["direction"] = *direction;
        if (currentA)  obj["currentA"] = *currentA;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        if (obj["speed"].is<int32_t>() )  speed = obj["speed"].as<int32_t>();
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        if (obj["currentA"].is<int32_t>() )  currentA = obj["currentA"].as<int32_t>();
        }

};

class HoverboardCmd : public Msg {
    public:
    static constexpr const char *id = "HoverboardCmd";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 58218;

    std::string dst;
    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["HoverboardCmd"].to<JsonObject>();
        obj["dst"] = dst;
        if (speed)  obj["speed"] = *speed;
        if (direction)  obj["direction"] = *direction;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        
        if (obj["speed"].is<int32_t>() )  speed = obj["speed"].as<int32_t>();
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        }

};

class LpsInfo : public Msg {
    public:
    static constexpr const char *id = "LpsInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 24957;

    std::optional<int32_t> direction;
    std::optional<std::string> msg;
    

    JsonDocument serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["LpsInfo"].to<JsonObject>();
        if (direction)  obj["direction"] = *direction;
        if (msg)  obj["msg"] = *msg;
        return doc;
    }

    void deserialize(const JsonObject& obj) {
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        if (obj["msg"].is<std::string>() )  msg = obj["msg"].as<std::string>();
        }

};

