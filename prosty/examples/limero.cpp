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

    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (set_time.has_value()) {
            obj["set_time"] = set_time.value();
        }
        if (reboot.has_value()) {
            obj["reboot"] = reboot.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["set_time"].is<uint64_t>() ) {
            set_time = obj["set_time"].as<uint64_t>();
        }
        if (obj["reboot"].is<bool>() ) {
            reboot = obj["reboot"].as<bool>();
        }
        
    }

};

class SysInfo : public Msg {
    public:
    static constexpr const char *id = "SysInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 10347;

    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_memory;
    std::vector<uint64_t> total_memory;
    std::optional<Toggle> power_on;
    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (uptime.has_value()) {
            obj["uptime"] = uptime.value();
        }
        if (free_memory.has_value()) {
            obj["free_memory"] = free_memory.value();
        }
        JsonArray arr = obj["total_memory"].as<JsonArray>();
        for (const auto& item : total_memory) {
            arr.add(item);
        }
        if (power_on.has_value()) {
            obj["power_on"] = power_on.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["uptime"].is<uint64_t>() ) {
            uptime = obj["uptime"].as<uint64_t>();
        }
        if (obj["free_memory"].is<uint64_t>() ) {
            free_memory = obj["free_memory"].as<uint64_t>();
        }
        if (obj["total_memory"].is<JsonArray>()) {
            JsonArray arr = obj["total_memory"].as<JsonArray>();
            total_memory.clear();
            for (JsonVariant v : arr) {
                total_memory.push_back(v.as<uint64_t>());
            }
        }
        if (obj["power_on"].is<Toggle>() ) {
            power_on = obj["power_on"].as<Toggle>();
        }
        
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
    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (ssid.has_value()) {
            obj["ssid"] = ssid.value();
        }
        if (bssid.has_value()) {
            obj["bssid"] = bssid.value();
        }
        if (rssi.has_value()) {
            obj["rssi"] = rssi.value();
        }
        if (ip.has_value()) {
            obj["ip"] = ip.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["ssid"].is<std::string>() ) {
            ssid = obj["ssid"].as<std::string>();
        }
        if (obj["bssid"].is<std::string>() ) {
            bssid = obj["bssid"].as<std::string>();
        }
        if (obj["rssi"].is<int32_t>() ) {
            rssi = obj["rssi"].as<int32_t>();
        }
        if (obj["ip"].is<std::string>() ) {
            ip = obj["ip"].as<std::string>();
        }
        
    }

};

class MotorInfo : public Msg {
    public:
    static constexpr const char *id = "MotorInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 62329;

    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (speed.has_value()) {
            obj["speed"] = speed.value();
        }
        if (direction.has_value()) {
            obj["direction"] = direction.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["speed"].is<int32_t>() ) {
            speed = obj["speed"].as<int32_t>();
        }
        if (obj["direction"].is<int32_t>() ) {
            direction = obj["direction"].as<int32_t>();
        }
        
    }

};

class MotorCmd : public Msg {
    public:
    static constexpr const char *id = "MotorCmd";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 32797;

    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    Bytes serialize() const {
        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        if (speed.has_value()) {
            obj["speed"] = speed.value();
        }
        if (direction.has_value()) {
            obj["direction"] = direction.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["speed"].is<int32_t>() ) {
            speed = obj["speed"].as<int32_t>();
        }
        if (obj["direction"].is<int32_t>() ) {
            direction = obj["direction"].as<int32_t>();
        }
        
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
        JsonObject obj = doc.to<JsonObject>();
        if (direction.has_value()) {
            obj["direction"] = direction.value();
        }
        if (msg.has_value()) {
            obj["msg"] = msg.value();
        }
        
        std::string str;
        serializeJson(doc, str);
        return {str.begin(), str.end()};
    }

    void deserialize(const Bytes& data) {
        JsonDocument doc;
        deserializeJson(doc, data);
        JsonObject obj = doc.as<JsonObject>();
        if (obj["direction"].is<int32_t>() ) {
            direction = obj["direction"].as<int32_t>();
        }
        if (obj["msg"].is<std::string>() ) {
            msg = obj["msg"].as<std::string>();
        }
        
    }

};

