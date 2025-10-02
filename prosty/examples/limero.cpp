#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <actor.h>
#include <result.h>


typedef std::vector<uint8_t> Bytes;


typedef enum {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    ALERT = 6,
} LogLevel;

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



class ZenohInfo : public Msg {
    public:
    static constexpr const char *id = "ZenohInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 33380;

    std::optional<std::string> zid;
    std::optional<std::string> what_am_i;
    std::vector<std::string> peers;
    std::optional<std::string> prefix;
    std::vector<std::string> routers;
    std::optional<std::string> connect;
    std::optional<std::string> listen;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["ZenohInfo"].to<JsonObject>();
        if (zid)  obj["zid"] = *zid;
        if (what_am_i)  obj["what_am_i"] = *what_am_i;
        {
            JsonArray arr = obj["peers"].to<JsonArray>();
        for (const auto& item : peers) {
            arr.add(item);
        }
        }
        if (prefix)  obj["prefix"] = *prefix;
        {
            JsonArray arr = obj["routers"].to<JsonArray>();
        for (const auto& item : routers) {
            arr.add(item);
        }
        }
        if (connect)  obj["connect"] = *connect;
        if (listen)  obj["listen"] = *listen;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["zid"].is<std::string>() )  zid = obj["zid"].as<std::string>();
        if (obj["what_am_i"].is<std::string>() )  what_am_i = obj["what_am_i"].as<std::string>();
        if (obj["peers"].is<JsonArray>()) {
            JsonArray arr = obj["peers"].as<JsonArray>();
            peers.clear();
            for (JsonVariant v : arr) {
                peers.push_back(v.as<std::string>());
            }
        }
        if (obj["prefix"].is<std::string>() )  prefix = obj["prefix"].as<std::string>();
        if (obj["routers"].is<JsonArray>()) {
            JsonArray arr = obj["routers"].as<JsonArray>();
            routers.clear();
            for (JsonVariant v : arr) {
                routers.push_back(v.as<std::string>());
            }
        }
        if (obj["connect"].is<std::string>() )  connect = obj["connect"].as<std::string>();
        if (obj["listen"].is<std::string>() )  listen = obj["listen"].as<std::string>();
        return true;
    }

};

class LogInfo : public Msg {
    public:
    static constexpr const char *id = "LogInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 34678;

    std::optional<LogLevel> level;
    std::optional<std::string> message;
    std::optional<int32_t> error_code;
    std::optional<std::string> file;
    std::optional<int32_t> line;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["LogInfo"].to<JsonObject>();
        if (level)  obj["level"] = *level;
        if (message)  obj["message"] = *message;
        if (error_code)  obj["error_code"] = *error_code;
        if (file)  obj["file"] = *file;
        if (line)  obj["line"] = *line;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["level"].is<LogLevel>() )  level = obj["level"].as<LogLevel>();
        if (obj["message"].is<std::string>() )  message = obj["message"].as<std::string>();
        if (obj["error_code"].is<int32_t>() )  error_code = obj["error_code"].as<int32_t>();
        if (obj["file"].is<std::string>() )  file = obj["file"].as<std::string>();
        if (obj["line"].is<int32_t>() )  line = obj["line"].as<int32_t>();
        return true;
    }

};

class SysCmd : public Msg {
    public:
    static constexpr const char *id = "SysCmd";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 51983;

    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["SysCmd"].to<JsonObject>();
        obj["src"] = src;
        if (set_time)  obj["set_time"] = *set_time;
        if (reboot)  obj["reboot"] = *reboot;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        
        if (obj["set_time"].is<uint64_t>() )  set_time = obj["set_time"].as<uint64_t>();
        if (obj["reboot"].is<bool>() )  reboot = obj["reboot"].as<bool>();
        return true;
    }

};

class SysInfo : public Msg {
    public:
    static constexpr const char *id = "SysInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 10347;

    std::optional<uint64_t> utc;
    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_heap;
    std::optional<uint64_t> flash;
    std::optional<std::string> cpu_board;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["SysInfo"].to<JsonObject>();
        if (utc)  obj["utc"] = *utc;
        if (uptime)  obj["uptime"] = *uptime;
        if (free_heap)  obj["free_heap"] = *free_heap;
        if (flash)  obj["flash"] = *flash;
        if (cpu_board)  obj["cpu_board"] = *cpu_board;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["utc"].is<uint64_t>() )  utc = obj["utc"].as<uint64_t>();
        if (obj["uptime"].is<uint64_t>() )  uptime = obj["uptime"].as<uint64_t>();
        if (obj["free_heap"].is<uint64_t>() )  free_heap = obj["free_heap"].as<uint64_t>();
        if (obj["flash"].is<uint64_t>() )  flash = obj["flash"].as<uint64_t>();
        if (obj["cpu_board"].is<std::string>() )  cpu_board = obj["cpu_board"].as<std::string>();
        return true;
    }

};

class WifiInfo : public Msg {
    public:
    static constexpr const char *id = "WifiInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 15363;

    std::optional<std::string> ssid;
    std::optional<std::string> bssid;
    std::optional<int32_t> rssi;
    std::optional<std::string> ip;
    std::optional<std::string> mac;
    std::optional<int32_t> channel;
    std::optional<std::string> gateway;
    std::optional<std::string> netmask;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["WifiInfo"].to<JsonObject>();
        if (ssid)  obj["ssid"] = *ssid;
        if (bssid)  obj["bssid"] = *bssid;
        if (rssi)  obj["rssi"] = *rssi;
        if (ip)  obj["ip"] = *ip;
        if (mac)  obj["mac"] = *mac;
        if (channel)  obj["channel"] = *channel;
        if (gateway)  obj["gateway"] = *gateway;
        if (netmask)  obj["netmask"] = *netmask;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["ssid"].is<std::string>() )  ssid = obj["ssid"].as<std::string>();
        if (obj["bssid"].is<std::string>() )  bssid = obj["bssid"].as<std::string>();
        if (obj["rssi"].is<int32_t>() )  rssi = obj["rssi"].as<int32_t>();
        if (obj["ip"].is<std::string>() )  ip = obj["ip"].as<std::string>();
        if (obj["mac"].is<std::string>() )  mac = obj["mac"].as<std::string>();
        if (obj["channel"].is<int32_t>() )  channel = obj["channel"].as<int32_t>();
        if (obj["gateway"].is<std::string>() )  gateway = obj["gateway"].as<std::string>();
        if (obj["netmask"].is<std::string>() )  netmask = obj["netmask"].as<std::string>();
        return true;
    }

};

class MulticastInfo : public Msg {
    public:
    static constexpr const char *id = "MulticastInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 61310;

    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["MulticastInfo"].to<JsonObject>();
        if (group)  obj["group"] = *group;
        if (port)  obj["port"] = *port;
        if (mtu)  obj["mtu"] = *mtu;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["group"].is<std::string>() )  group = obj["group"].as<std::string>();
        if (obj["port"].is<int32_t>() )  port = obj["port"].as<int32_t>();
        if (obj["mtu"].is<uint32_t>() )  mtu = obj["mtu"].as<uint32_t>();
        return true;
    }

};

class HoverboardInfo : public Msg {
    public:
    static constexpr const char *id = "HoverboardInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 59150;

    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    std::optional<int32_t> currentA;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["HoverboardInfo"].to<JsonObject>();
        if (speed)  obj["speed"] = *speed;
        if (direction)  obj["direction"] = *direction;
        if (currentA)  obj["currentA"] = *currentA;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["speed"].is<int32_t>() )  speed = obj["speed"].as<int32_t>();
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        if (obj["currentA"].is<int32_t>() )  currentA = obj["currentA"].as<int32_t>();
        return true;
    }

};

class HoverboardCmd : public Msg {
    public:
    static constexpr const char *id = "HoverboardCmd";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 58218;

    std::optional<std::string> src;
    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["HoverboardCmd"].to<JsonObject>();
        if (src)  obj["src"] = *src;
        if (speed)  obj["speed"] = *speed;
        if (direction)  obj["direction"] = *direction;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["src"].is<std::string>() )  src = obj["src"].as<std::string>();
        if (obj["speed"].is<int32_t>() )  speed = obj["speed"].as<int32_t>();
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        return true;
    }

};

class LpsInfo : public Msg {
    public:
    static constexpr const char *id = "LpsInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 24957;

    std::optional<int32_t> direction;
    std::optional<std::string> msg;
    

    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc["LpsInfo"].to<JsonObject>();
        if (direction)  obj["direction"] = *direction;
        if (msg)  obj["msg"] = *msg;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument obj;
        auto err = deserializeJson(obj,bytes);
        if ( err != DeserializationError::Ok || obj.is<JsonObject>() == false ) {
            return false;
        };        
        if (obj["direction"].is<int32_t>() )  direction = obj["direction"].as<int32_t>();
        if (obj["msg"].is<std::string>() )  msg = obj["msg"].as<std::string>();
        return true;
    }

};

