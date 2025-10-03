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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (zid)  doc["zid"] = *zid;
        if (what_am_i)  doc["what_am_i"] = *what_am_i;
        {
            JsonArray arr = doc["peers"].to<JsonArray>();
        for (const auto& item : peers) {
            arr.add(item);
        }
        }
        if (prefix)  doc["prefix"] = *prefix;
        {
            JsonArray arr = doc["routers"].to<JsonArray>();
        for (const auto& item : routers) {
            arr.add(item);
        }
        }
        if (connect)  doc["connect"] = *connect;
        if (listen)  doc["listen"] = *listen;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["zid"].is<std::string>() )  zid = doc["zid"].as<std::string>();
        if (doc["what_am_i"].is<std::string>() )  what_am_i = doc["what_am_i"].as<std::string>();
        if (doc["peers"].is<JsonArray>()) {
            JsonArray arr = doc["peers"].as<JsonArray>();
            peers.clear();
            for (JsonVariant v : arr) {
                peers.push_back(v.as<std::string>());
            }
        }
        if (doc["prefix"].is<std::string>() )  prefix = doc["prefix"].as<std::string>();
        if (doc["routers"].is<JsonArray>()) {
            JsonArray arr = doc["routers"].as<JsonArray>();
            routers.clear();
            for (JsonVariant v : arr) {
                routers.push_back(v.as<std::string>());
            }
        }
        if (doc["connect"].is<std::string>() )  connect = doc["connect"].as<std::string>();
        if (doc["listen"].is<std::string>() )  listen = doc["listen"].as<std::string>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (level)  doc["level"] = *level;
        if (message)  doc["message"] = *message;
        if (error_code)  doc["error_code"] = *error_code;
        if (file)  doc["file"] = *file;
        if (line)  doc["line"] = *line;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["level"].is<LogLevel>() )  level = doc["level"].as<LogLevel>();
        if (doc["message"].is<std::string>() )  message = doc["message"].as<std::string>();
        if (doc["error_code"].is<int32_t>() )  error_code = doc["error_code"].as<int32_t>();
        if (doc["file"].is<std::string>() )  file = doc["file"].as<std::string>();
        if (doc["line"].is<int32_t>() )  line = doc["line"].as<int32_t>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        doc["src"] = src;
        if (set_time)  doc["set_time"] = *set_time;
        if (reboot)  doc["reboot"] = *reboot;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        
        if (doc["set_time"].is<uint64_t>() )  set_time = doc["set_time"].as<uint64_t>();
        if (doc["reboot"].is<bool>() )  reboot = doc["reboot"].as<bool>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (utc)  doc["utc"] = *utc;
        if (uptime)  doc["uptime"] = *uptime;
        if (free_heap)  doc["free_heap"] = *free_heap;
        if (flash)  doc["flash"] = *flash;
        if (cpu_board)  doc["cpu_board"] = *cpu_board;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["utc"].is<uint64_t>() )  utc = doc["utc"].as<uint64_t>();
        if (doc["uptime"].is<uint64_t>() )  uptime = doc["uptime"].as<uint64_t>();
        if (doc["free_heap"].is<uint64_t>() )  free_heap = doc["free_heap"].as<uint64_t>();
        if (doc["flash"].is<uint64_t>() )  flash = doc["flash"].as<uint64_t>();
        if (doc["cpu_board"].is<std::string>() )  cpu_board = doc["cpu_board"].as<std::string>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (ssid)  doc["ssid"] = *ssid;
        if (bssid)  doc["bssid"] = *bssid;
        if (rssi)  doc["rssi"] = *rssi;
        if (ip)  doc["ip"] = *ip;
        if (mac)  doc["mac"] = *mac;
        if (channel)  doc["channel"] = *channel;
        if (gateway)  doc["gateway"] = *gateway;
        if (netmask)  doc["netmask"] = *netmask;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["ssid"].is<std::string>() )  ssid = doc["ssid"].as<std::string>();
        if (doc["bssid"].is<std::string>() )  bssid = doc["bssid"].as<std::string>();
        if (doc["rssi"].is<int32_t>() )  rssi = doc["rssi"].as<int32_t>();
        if (doc["ip"].is<std::string>() )  ip = doc["ip"].as<std::string>();
        if (doc["mac"].is<std::string>() )  mac = doc["mac"].as<std::string>();
        if (doc["channel"].is<int32_t>() )  channel = doc["channel"].as<int32_t>();
        if (doc["gateway"].is<std::string>() )  gateway = doc["gateway"].as<std::string>();
        if (doc["netmask"].is<std::string>() )  netmask = doc["netmask"].as<std::string>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (group)  doc["group"] = *group;
        if (port)  doc["port"] = *port;
        if (mtu)  doc["mtu"] = *mtu;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["group"].is<std::string>() )  group = doc["group"].as<std::string>();
        if (doc["port"].is<int32_t>() )  port = doc["port"].as<int32_t>();
        if (doc["mtu"].is<uint32_t>() )  mtu = doc["mtu"].as<uint32_t>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (speed)  doc["speed"] = *speed;
        if (direction)  doc["direction"] = *direction;
        if (currentA)  doc["currentA"] = *currentA;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["speed"].is<int32_t>() )  speed = doc["speed"].as<int32_t>();
        if (doc["direction"].is<int32_t>() )  direction = doc["direction"].as<int32_t>();
        if (doc["currentA"].is<int32_t>() )  currentA = doc["currentA"].as<int32_t>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (src)  doc["src"] = *src;
        if (speed)  doc["speed"] = *speed;
        if (direction)  doc["direction"] = *direction;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["src"].is<std::string>() )  src = doc["src"].as<std::string>();
        if (doc["speed"].is<int32_t>() )  speed = doc["speed"].as<int32_t>();
        if (doc["direction"].is<int32_t>() )  direction = doc["direction"].as<int32_t>();
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
    

    Bytes serialize()  {
        JsonDocument doc;
        if (direction)  doc["direction"] = *direction;
        if (msg)  doc["msg"] = *msg;
        std::string str;
        serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    bool deserialize(const Bytes& bytes) {
        JsonDocument doc;
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            return false;
        };        
        if (doc["direction"].is<int32_t>() )  direction = doc["direction"].as<int32_t>();
        if (doc["msg"].is<std::string>() )  msg = doc["msg"].as<std::string>();
        return true;
    }

};

