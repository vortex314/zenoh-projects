#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <msg.h>
#include <serdes.h>



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

typedef enum {
    VOLTAGE = 1,
    SPEED = 2,
    TORQUE = 3,
} CtrlMod;

typedef enum {
    COMMUTATION = 0,
    SINUSOIDAL = 1,
    FOC = 2,
} CtrlTyp;

typedef enum {
    DISABLED = 0,
    NORMAL_POT = 1,
    MIDDLE_RESTING_POT = 2,
    AUTO_DETECT = 3,
} InTyp;

class Sample : public Msg {
    MSG(Sample);
    public:
    std::optional<bool> flag;
    std::optional<int32_t> identifier;
    std::optional<std::string> name;
    std::vector<float> values;
    std::optional<float> f;
    std::optional<double> d;
    std::optional<Bytes> data;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (flag)doc["flag"] = *flag;
        if (identifier)doc["identifier"] = *identifier;
        if (name)doc["name"] = *name;
        {
                    JsonArray arr = doc["values"].to<JsonArray>();
                    for (const auto& item : values) {
                        arr.add(item);
                    }
                }
        if (f)doc["f"] = *f;
        if (d)doc["d"] = *d;
        if (data)
                        doc["data"] = base64_encode(*data);
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    Sample* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Sample* msg = new Sample();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["flag"].is<bool>() )  
                        msg->flag = doc["flag"].as<bool>();
        if (doc["identifier"].is<int32_t>() )  
                        msg->identifier = doc["identifier"].as<int32_t>();
        if (doc["name"].is<std::string>() )  
                        msg->name = doc["name"].as<std::string>();
        if (doc["values"].is<JsonArray>()) {
                    JsonArray arr = doc["values"].as<JsonArray>();
                    msg->values.clear();
                    for (JsonVariant v : arr) {
                        msg->values.push_back(v.as<float>());
                    }
                }
        if (doc["f"].is<float>() )  
                        msg->f = doc["f"].as<float>();
        if (doc["d"].is<double>() )  
                        msg->d = doc["d"].as<double>();
        if (doc["data"].is<std::string>() )  
                        msg->data = base64_decode(doc["data"].as<std::string>());
        return msg;
    }

};

class ZenohInfo : public Msg {
    MSG(ZenohInfo);
    public:
    std::optional<std::string> zid;
    std::optional<std::string> what_am_i;
    std::vector<std::string> peers;
    std::optional<std::string> prefix;
    std::vector<std::string> routers;
    std::optional<std::string> connect;
    std::optional<std::string> listen;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (zid)doc["zid"] = *zid;
        if (what_am_i)doc["what_am_i"] = *what_am_i;
        {
                    JsonArray arr = doc["peers"].to<JsonArray>();
                    for (const auto& item : peers) {
                        arr.add(item);
                    }
                }
        if (prefix)doc["prefix"] = *prefix;
        {
                    JsonArray arr = doc["routers"].to<JsonArray>();
                    for (const auto& item : routers) {
                        arr.add(item);
                    }
                }
        if (connect)doc["connect"] = *connect;
        if (listen)doc["listen"] = *listen;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    ZenohInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        ZenohInfo* msg = new ZenohInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["zid"].is<std::string>() )  
                        msg->zid = doc["zid"].as<std::string>();
        if (doc["what_am_i"].is<std::string>() )  
                        msg->what_am_i = doc["what_am_i"].as<std::string>();
        if (doc["peers"].is<JsonArray>()) {
                    JsonArray arr = doc["peers"].as<JsonArray>();
                    msg->peers.clear();
                    for (JsonVariant v : arr) {
                        msg->peers.push_back(v.as<std::string>());
                    }
                }
        if (doc["prefix"].is<std::string>() )  
                        msg->prefix = doc["prefix"].as<std::string>();
        if (doc["routers"].is<JsonArray>()) {
                    JsonArray arr = doc["routers"].as<JsonArray>();
                    msg->routers.clear();
                    for (JsonVariant v : arr) {
                        msg->routers.push_back(v.as<std::string>());
                    }
                }
        if (doc["connect"].is<std::string>() )  
                        msg->connect = doc["connect"].as<std::string>();
        if (doc["listen"].is<std::string>() )  
                        msg->listen = doc["listen"].as<std::string>();
        return msg;
    }

};

class LogInfo : public Msg {
    MSG(LogInfo);
    public:
    std::optional<LogLevel> level;
    std::optional<std::string> message;
    std::optional<int32_t> error_code;
    std::optional<std::string> file;
    std::optional<int32_t> line;
    std::optional<uint64_t> timestamp;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (level)doc["level"] = *level;
        if (message)doc["message"] = *message;
        if (error_code)doc["error_code"] = *error_code;
        if (file)doc["file"] = *file;
        if (line)doc["line"] = *line;
        if (timestamp)doc["timestamp"] = *timestamp;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    LogInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LogInfo* msg = new LogInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["level"].is<LogLevel>() )  
                        msg->level = doc["level"].as<LogLevel>();
        if (doc["message"].is<std::string>() )  
                        msg->message = doc["message"].as<std::string>();
        if (doc["error_code"].is<int32_t>() )  
                        msg->error_code = doc["error_code"].as<int32_t>();
        if (doc["file"].is<std::string>() )  
                        msg->file = doc["file"].as<std::string>();
        if (doc["line"].is<int32_t>() )  
                        msg->line = doc["line"].as<int32_t>();
        if (doc["timestamp"].is<uint64_t>() )  
                        msg->timestamp = doc["timestamp"].as<uint64_t>();
        return msg;
    }

};

class SysCmd : public Msg {
    MSG(SysCmd);
    public:
    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    std::optional<std::string> console;
    

    Bytes serialize() const {
        JsonDocument doc;
        doc["src"] = src;
        if (set_time)doc["set_time"] = *set_time;
        if (reboot)doc["reboot"] = *reboot;
        if (console)doc["console"] = *console;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    SysCmd* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        SysCmd* msg = new SysCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["src"].is<std::string>() )
                    msg->src = doc["src"].as<std::string>();
        if (doc["set_time"].is<uint64_t>() )  
                        msg->set_time = doc["set_time"].as<uint64_t>();
        if (doc["reboot"].is<bool>() )  
                        msg->reboot = doc["reboot"].as<bool>();
        if (doc["console"].is<std::string>() )  
                        msg->console = doc["console"].as<std::string>();
        return msg;
    }

};

class SysInfo : public Msg {
    MSG(SysInfo);
    public:
    std::optional<uint64_t> utc;
    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_heap;
    std::optional<uint64_t> flash;
    std::optional<std::string> cpu_board;
    std::optional<std::string> build_date;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (utc)doc["utc"] = *utc;
        if (uptime)doc["uptime"] = *uptime;
        if (free_heap)doc["free_heap"] = *free_heap;
        if (flash)doc["flash"] = *flash;
        if (cpu_board)doc["cpu_board"] = *cpu_board;
        if (build_date)doc["build_date"] = *build_date;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    SysInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        SysInfo* msg = new SysInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["utc"].is<uint64_t>() )  
                        msg->utc = doc["utc"].as<uint64_t>();
        if (doc["uptime"].is<uint64_t>() )  
                        msg->uptime = doc["uptime"].as<uint64_t>();
        if (doc["free_heap"].is<uint64_t>() )  
                        msg->free_heap = doc["free_heap"].as<uint64_t>();
        if (doc["flash"].is<uint64_t>() )  
                        msg->flash = doc["flash"].as<uint64_t>();
        if (doc["cpu_board"].is<std::string>() )  
                        msg->cpu_board = doc["cpu_board"].as<std::string>();
        if (doc["build_date"].is<std::string>() )  
                        msg->build_date = doc["build_date"].as<std::string>();
        return msg;
    }

};

class WifiInfo : public Msg {
    MSG(WifiInfo);
    public:
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
        if (ssid)doc["ssid"] = *ssid;
        if (bssid)doc["bssid"] = *bssid;
        if (rssi)doc["rssi"] = *rssi;
        if (ip)doc["ip"] = *ip;
        if (mac)doc["mac"] = *mac;
        if (channel)doc["channel"] = *channel;
        if (gateway)doc["gateway"] = *gateway;
        if (netmask)doc["netmask"] = *netmask;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    WifiInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        WifiInfo* msg = new WifiInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["ssid"].is<std::string>() )  
                        msg->ssid = doc["ssid"].as<std::string>();
        if (doc["bssid"].is<std::string>() )  
                        msg->bssid = doc["bssid"].as<std::string>();
        if (doc["rssi"].is<int32_t>() )  
                        msg->rssi = doc["rssi"].as<int32_t>();
        if (doc["ip"].is<std::string>() )  
                        msg->ip = doc["ip"].as<std::string>();
        if (doc["mac"].is<std::string>() )  
                        msg->mac = doc["mac"].as<std::string>();
        if (doc["channel"].is<int32_t>() )  
                        msg->channel = doc["channel"].as<int32_t>();
        if (doc["gateway"].is<std::string>() )  
                        msg->gateway = doc["gateway"].as<std::string>();
        if (doc["netmask"].is<std::string>() )  
                        msg->netmask = doc["netmask"].as<std::string>();
        return msg;
    }

};

class MulticastInfo : public Msg {
    MSG(MulticastInfo);
    public:
    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (group)doc["group"] = *group;
        if (port)doc["port"] = *port;
        if (mtu)doc["mtu"] = *mtu;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    MulticastInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        MulticastInfo* msg = new MulticastInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["group"].is<std::string>() )  
                        msg->group = doc["group"].as<std::string>();
        if (doc["port"].is<int32_t>() )  
                        msg->port = doc["port"].as<int32_t>();
        if (doc["mtu"].is<uint32_t>() )  
                        msg->mtu = doc["mtu"].as<uint32_t>();
        return msg;
    }

};

class HoverboardInfo : public Msg {
    MSG(HoverboardInfo);
    public:
    std::optional<CtrlMod> ctrl_mod;
    std::optional<CtrlTyp> ctrl_typ;
    std::optional<uint32_t> cur_mot_max;
    std::optional<uint32_t> rpm_mot_max;
    std::optional<uint32_t> fi_weak_ena;
    std::optional<uint32_t> fi_weak_hi;
    std::optional<uint32_t> fi_weak_lo;
    std::optional<uint32_t> fi_weak_max;
    std::optional<uint32_t> phase_adv_max_deg;
    std::optional<uint32_t> input1_raw;
    std::optional<InTyp> input1_typ;
    std::optional<uint32_t> input1_min;
    std::optional<uint32_t> input1_mid;
    std::optional<uint32_t> input1_max;
    std::optional<uint32_t> input1_cmd;
    std::optional<uint32_t> input2_raw;
    std::optional<InTyp> input2_typ;
    std::optional<uint32_t> input2_min;
    std::optional<uint32_t> input2_mid;
    std::optional<uint32_t> input2_max;
    std::optional<uint32_t> input2_cmd;
    std::optional<uint32_t> aux_input1_raw;
    std::optional<InTyp> aux_input1_typ;
    std::optional<uint32_t> aux_input1_min;
    std::optional<uint32_t> aux_input1_mid;
    std::optional<uint32_t> aux_input1_max;
    std::optional<uint32_t> aux_input1_cmd;
    std::optional<uint32_t> aux_input2_raw;
    std::optional<InTyp> aux_input2_typ;
    std::optional<uint32_t> aux_input2_min;
    std::optional<uint32_t> aux_input2_mid;
    std::optional<uint32_t> aux_input2_max;
    std::optional<uint32_t> aux_input2_cmd;
    std::optional<uint32_t> dc_curr;
    std::optional<uint32_t> rdc_curr;
    std::optional<uint32_t> ldc_curr;
    std::optional<uint32_t> cmdl;
    std::optional<uint32_t> cmdr;
    std::optional<uint32_t> spd_avg;
    std::optional<uint32_t> spdl;
    std::optional<uint32_t> spdr;
    std::optional<uint32_t> filter_rate;
    std::optional<uint32_t> spd_coef;
    std::optional<uint32_t> str_coef;
    std::optional<uint32_t> batv;
    std::optional<uint32_t> temp;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (ctrl_mod)doc["ctrl_mod"] = *ctrl_mod;
        if (ctrl_typ)doc["ctrl_typ"] = *ctrl_typ;
        if (cur_mot_max)doc["cur_mot_max"] = *cur_mot_max;
        if (rpm_mot_max)doc["rpm_mot_max"] = *rpm_mot_max;
        if (fi_weak_ena)doc["fi_weak_ena"] = *fi_weak_ena;
        if (fi_weak_hi)doc["fi_weak_hi"] = *fi_weak_hi;
        if (fi_weak_lo)doc["fi_weak_lo"] = *fi_weak_lo;
        if (fi_weak_max)doc["fi_weak_max"] = *fi_weak_max;
        if (phase_adv_max_deg)doc["phase_adv_max_deg"] = *phase_adv_max_deg;
        if (input1_raw)doc["input1_raw"] = *input1_raw;
        if (input1_typ)doc["input1_typ"] = *input1_typ;
        if (input1_min)doc["input1_min"] = *input1_min;
        if (input1_mid)doc["input1_mid"] = *input1_mid;
        if (input1_max)doc["input1_max"] = *input1_max;
        if (input1_cmd)doc["input1_cmd"] = *input1_cmd;
        if (input2_raw)doc["input2_raw"] = *input2_raw;
        if (input2_typ)doc["input2_typ"] = *input2_typ;
        if (input2_min)doc["input2_min"] = *input2_min;
        if (input2_mid)doc["input2_mid"] = *input2_mid;
        if (input2_max)doc["input2_max"] = *input2_max;
        if (input2_cmd)doc["input2_cmd"] = *input2_cmd;
        if (aux_input1_raw)doc["aux_input1_raw"] = *aux_input1_raw;
        if (aux_input1_typ)doc["aux_input1_typ"] = *aux_input1_typ;
        if (aux_input1_min)doc["aux_input1_min"] = *aux_input1_min;
        if (aux_input1_mid)doc["aux_input1_mid"] = *aux_input1_mid;
        if (aux_input1_max)doc["aux_input1_max"] = *aux_input1_max;
        if (aux_input1_cmd)doc["aux_input1_cmd"] = *aux_input1_cmd;
        if (aux_input2_raw)doc["aux_input2_raw"] = *aux_input2_raw;
        if (aux_input2_typ)doc["aux_input2_typ"] = *aux_input2_typ;
        if (aux_input2_min)doc["aux_input2_min"] = *aux_input2_min;
        if (aux_input2_mid)doc["aux_input2_mid"] = *aux_input2_mid;
        if (aux_input2_max)doc["aux_input2_max"] = *aux_input2_max;
        if (aux_input2_cmd)doc["aux_input2_cmd"] = *aux_input2_cmd;
        if (dc_curr)doc["dc_curr"] = *dc_curr;
        if (rdc_curr)doc["rdc_curr"] = *rdc_curr;
        if (ldc_curr)doc["ldc_curr"] = *ldc_curr;
        if (cmdl)doc["cmdl"] = *cmdl;
        if (cmdr)doc["cmdr"] = *cmdr;
        if (spd_avg)doc["spd_avg"] = *spd_avg;
        if (spdl)doc["spdl"] = *spdl;
        if (spdr)doc["spdr"] = *spdr;
        if (filter_rate)doc["filter_rate"] = *filter_rate;
        if (spd_coef)doc["spd_coef"] = *spd_coef;
        if (str_coef)doc["str_coef"] = *str_coef;
        if (batv)doc["batv"] = *batv;
        if (temp)doc["temp"] = *temp;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    static HoverboardInfo* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardInfo* msg = new HoverboardInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["ctrl_mod"].is<CtrlMod>() )  
                        msg->ctrl_mod = doc["ctrl_mod"].as<CtrlMod>();
        if (doc["ctrl_typ"].is<CtrlTyp>() )  
                        msg->ctrl_typ = doc["ctrl_typ"].as<CtrlTyp>();
        if (doc["cur_mot_max"].is<uint32_t>() )  
                        msg->cur_mot_max = doc["cur_mot_max"].as<uint32_t>();
        if (doc["rpm_mot_max"].is<uint32_t>() )  
                        msg->rpm_mot_max = doc["rpm_mot_max"].as<uint32_t>();
        if (doc["fi_weak_ena"].is<uint32_t>() )  
                        msg->fi_weak_ena = doc["fi_weak_ena"].as<uint32_t>();
        if (doc["fi_weak_hi"].is<uint32_t>() )  
                        msg->fi_weak_hi = doc["fi_weak_hi"].as<uint32_t>();
        if (doc["fi_weak_lo"].is<uint32_t>() )  
                        msg->fi_weak_lo = doc["fi_weak_lo"].as<uint32_t>();
        if (doc["fi_weak_max"].is<uint32_t>() )  
                        msg->fi_weak_max = doc["fi_weak_max"].as<uint32_t>();
        if (doc["phase_adv_max_deg"].is<uint32_t>() )  
                        msg->phase_adv_max_deg = doc["phase_adv_max_deg"].as<uint32_t>();
        if (doc["input1_raw"].is<uint32_t>() )  
                        msg->input1_raw = doc["input1_raw"].as<uint32_t>();
        if (doc["input1_typ"].is<InTyp>() )  
                        msg->input1_typ = doc["input1_typ"].as<InTyp>();
        if (doc["input1_min"].is<uint32_t>() )  
                        msg->input1_min = doc["input1_min"].as<uint32_t>();
        if (doc["input1_mid"].is<uint32_t>() )  
                        msg->input1_mid = doc["input1_mid"].as<uint32_t>();
        if (doc["input1_max"].is<uint32_t>() )  
                        msg->input1_max = doc["input1_max"].as<uint32_t>();
        if (doc["input1_cmd"].is<uint32_t>() )  
                        msg->input1_cmd = doc["input1_cmd"].as<uint32_t>();
        if (doc["input2_raw"].is<uint32_t>() )  
                        msg->input2_raw = doc["input2_raw"].as<uint32_t>();
        if (doc["input2_typ"].is<InTyp>() )  
                        msg->input2_typ = doc["input2_typ"].as<InTyp>();
        if (doc["input2_min"].is<uint32_t>() )  
                        msg->input2_min = doc["input2_min"].as<uint32_t>();
        if (doc["input2_mid"].is<uint32_t>() )  
                        msg->input2_mid = doc["input2_mid"].as<uint32_t>();
        if (doc["input2_max"].is<uint32_t>() )  
                        msg->input2_max = doc["input2_max"].as<uint32_t>();
        if (doc["input2_cmd"].is<uint32_t>() )  
                        msg->input2_cmd = doc["input2_cmd"].as<uint32_t>();
        if (doc["aux_input1_raw"].is<uint32_t>() )  
                        msg->aux_input1_raw = doc["aux_input1_raw"].as<uint32_t>();
        if (doc["aux_input1_typ"].is<InTyp>() )  
                        msg->aux_input1_typ = doc["aux_input1_typ"].as<InTyp>();
        if (doc["aux_input1_min"].is<uint32_t>() )  
                        msg->aux_input1_min = doc["aux_input1_min"].as<uint32_t>();
        if (doc["aux_input1_mid"].is<uint32_t>() )  
                        msg->aux_input1_mid = doc["aux_input1_mid"].as<uint32_t>();
        if (doc["aux_input1_max"].is<uint32_t>() )  
                        msg->aux_input1_max = doc["aux_input1_max"].as<uint32_t>();
        if (doc["aux_input1_cmd"].is<uint32_t>() )  
                        msg->aux_input1_cmd = doc["aux_input1_cmd"].as<uint32_t>();
        if (doc["aux_input2_raw"].is<uint32_t>() )  
                        msg->aux_input2_raw = doc["aux_input2_raw"].as<uint32_t>();
        if (doc["aux_input2_typ"].is<InTyp>() )  
                        msg->aux_input2_typ = doc["aux_input2_typ"].as<InTyp>();
        if (doc["aux_input2_min"].is<uint32_t>() )  
                        msg->aux_input2_min = doc["aux_input2_min"].as<uint32_t>();
        if (doc["aux_input2_mid"].is<uint32_t>() )  
                        msg->aux_input2_mid = doc["aux_input2_mid"].as<uint32_t>();
        if (doc["aux_input2_max"].is<uint32_t>() )  
                        msg->aux_input2_max = doc["aux_input2_max"].as<uint32_t>();
        if (doc["aux_input2_cmd"].is<uint32_t>() )  
                        msg->aux_input2_cmd = doc["aux_input2_cmd"].as<uint32_t>();
        if (doc["dc_curr"].is<uint32_t>() )  
                        msg->dc_curr = doc["dc_curr"].as<uint32_t>();
        if (doc["rdc_curr"].is<uint32_t>() )  
                        msg->rdc_curr = doc["rdc_curr"].as<uint32_t>();
        if (doc["ldc_curr"].is<uint32_t>() )  
                        msg->ldc_curr = doc["ldc_curr"].as<uint32_t>();
        if (doc["cmdl"].is<uint32_t>() )  
                        msg->cmdl = doc["cmdl"].as<uint32_t>();
        if (doc["cmdr"].is<uint32_t>() )  
                        msg->cmdr = doc["cmdr"].as<uint32_t>();
        if (doc["spd_avg"].is<uint32_t>() )  
                        msg->spd_avg = doc["spd_avg"].as<uint32_t>();
        if (doc["spdl"].is<uint32_t>() )  
                        msg->spdl = doc["spdl"].as<uint32_t>();
        if (doc["spdr"].is<uint32_t>() )  
                        msg->spdr = doc["spdr"].as<uint32_t>();
        if (doc["filter_rate"].is<uint32_t>() )  
                        msg->filter_rate = doc["filter_rate"].as<uint32_t>();
        if (doc["spd_coef"].is<uint32_t>() )  
                        msg->spd_coef = doc["spd_coef"].as<uint32_t>();
        if (doc["str_coef"].is<uint32_t>() )  
                        msg->str_coef = doc["str_coef"].as<uint32_t>();
        if (doc["batv"].is<uint32_t>() )  
                        msg->batv = doc["batv"].as<uint32_t>();
        if (doc["temp"].is<uint32_t>() )  
                        msg->temp = doc["temp"].as<uint32_t>();
        return msg;
    }

};

class HoverboardCmd : public Msg {
    MSG(HoverboardCmd);
    public:
    std::optional<int32_t> speed;
    std::optional<int32_t> steer;
    

    Bytes serialize() const {
        JsonDocument doc;
        if (speed)doc["speed"] = *speed;
        if (steer)doc["steer"] = *steer;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Bytes(str.begin(),str.end());
    }

    HoverboardCmd* deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardCmd* msg = new HoverboardCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return NULL ;
        };        
        if (doc["speed"].is<int32_t>() )  
                        msg->speed = doc["speed"].as<int32_t>();
        if (doc["steer"].is<int32_t>() )  
                        msg->steer = doc["steer"].as<int32_t>();
        return msg;
    }

};

