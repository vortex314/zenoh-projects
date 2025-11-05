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
    
    // Field indexes
        typedef enum {
        FLAG_INDEX = 1,
        IDENTIFIER_INDEX = 2,
        NAME_INDEX = 3,
        VALUES_INDEX = 4,
        F_INDEX = 5,
        D_INDEX = 6,
        DATA_INDEX = 7,
    } Field;
    static Result<Bytes> json_serialize(const Sample&);
    static Result<Sample*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const Sample&);
    static Result<Sample*> cbor_deserialize(const Bytes&);
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
    
    // Field indexes
        typedef enum {
        ZID_INDEX = 2,
        WHAT_AM_I_INDEX = 3,
        PEERS_INDEX = 4,
        PREFIX_INDEX = 5,
        ROUTERS_INDEX = 6,
        CONNECT_INDEX = 7,
        LISTEN_INDEX = 8,
    } Field;
    static Result<Bytes> json_serialize(const ZenohInfo&);
    static Result<ZenohInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const ZenohInfo&);
    static Result<ZenohInfo*> cbor_deserialize(const Bytes&);
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
    
    // Field indexes
        typedef enum {
        LEVEL_INDEX = 2,
        MESSAGE_INDEX = 3,
        ERROR_CODE_INDEX = 4,
        FILE_INDEX = 5,
        LINE_INDEX = 6,
        TIMESTAMP_INDEX = 7,
    } Field;
    static Result<Bytes> json_serialize(const LogInfo&);
    static Result<LogInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const LogInfo&);
    static Result<LogInfo*> cbor_deserialize(const Bytes&);
};

class SysCmd : public Msg {
    MSG(SysCmd);
    public:
    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    std::optional<std::string> console;
    
    // Field indexes
        typedef enum {
        SRC_INDEX = 2,
        SET_TIME_INDEX = 3,
        REBOOT_INDEX = 4,
        CONSOLE_INDEX = 5,
    } Field;
    static Result<Bytes> json_serialize(const SysCmd&);
    static Result<SysCmd*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const SysCmd&);
    static Result<SysCmd*> cbor_deserialize(const Bytes&);
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
    
    // Field indexes
        typedef enum {
        UTC_INDEX = 1,
        UPTIME_INDEX = 2,
        FREE_HEAP_INDEX = 3,
        FLASH_INDEX = 4,
        CPU_BOARD_INDEX = 5,
        BUILD_DATE_INDEX = 6,
    } Field;
    static Result<Bytes> json_serialize(const SysInfo&);
    static Result<SysInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const SysInfo&);
    static Result<SysInfo*> cbor_deserialize(const Bytes&);
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
    
    // Field indexes
        typedef enum {
        SSID_INDEX = 2,
        BSSID_INDEX = 3,
        RSSI_INDEX = 4,
        IP_INDEX = 5,
        MAC_INDEX = 6,
        CHANNEL_INDEX = 7,
        GATEWAY_INDEX = 8,
        NETMASK_INDEX = 9,
    } Field;
    static Result<Bytes> json_serialize(const WifiInfo&);
    static Result<WifiInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const WifiInfo&);
    static Result<WifiInfo*> cbor_deserialize(const Bytes&);
};

class MulticastInfo : public Msg {
    MSG(MulticastInfo);
    public:
    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;
    
    // Field indexes
        typedef enum {
        GROUP_INDEX = 2,
        PORT_INDEX = 3,
        MTU_INDEX = 4,
    } Field;
    static Result<Bytes> json_serialize(const MulticastInfo&);
    static Result<MulticastInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const MulticastInfo&);
    static Result<MulticastInfo*> cbor_deserialize(const Bytes&);
};

class HoverboardInfo : public Msg {
    MSG(HoverboardInfo);
    public:
    std::optional<CtrlMod> ctrl_mod;
    std::optional<CtrlTyp> ctrl_typ;
    std::optional<int32_t> cur_mot_max;
    std::optional<int32_t> rpm_mot_max;
    std::optional<int32_t> fi_weak_ena;
    std::optional<int32_t> fi_weak_hi;
    std::optional<int32_t> fi_weak_lo;
    std::optional<int32_t> fi_weak_max;
    std::optional<int32_t> phase_adv_max_deg;
    std::optional<int32_t> input1_raw;
    std::optional<InTyp> input1_typ;
    std::optional<int32_t> input1_min;
    std::optional<int32_t> input1_mid;
    std::optional<int32_t> input1_max;
    std::optional<int32_t> input1_cmd;
    std::optional<int32_t> input2_raw;
    std::optional<InTyp> input2_typ;
    std::optional<int32_t> input2_min;
    std::optional<int32_t> input2_mid;
    std::optional<int32_t> input2_max;
    std::optional<int32_t> input2_cmd;
    std::optional<int32_t> aux_input1_raw;
    std::optional<InTyp> aux_input1_typ;
    std::optional<int32_t> aux_input1_min;
    std::optional<int32_t> aux_input1_mid;
    std::optional<int32_t> aux_input1_max;
    std::optional<int32_t> aux_input1_cmd;
    std::optional<int32_t> aux_input2_raw;
    std::optional<InTyp> aux_input2_typ;
    std::optional<int32_t> aux_input2_min;
    std::optional<int32_t> aux_input2_mid;
    std::optional<int32_t> aux_input2_max;
    std::optional<int32_t> aux_input2_cmd;
    std::optional<int32_t> dc_curr;
    std::optional<int32_t> rdc_curr;
    std::optional<int32_t> ldc_curr;
    std::optional<int32_t> cmdl;
    std::optional<int32_t> cmdr;
    std::optional<int32_t> spd_avg;
    std::optional<int32_t> spdl;
    std::optional<int32_t> spdr;
    std::optional<int32_t> filter_rate;
    std::optional<int32_t> spd_coef;
    std::optional<int32_t> str_coef;
    std::optional<int32_t> batv;
    std::optional<int32_t> temp;
    
    // Field indexes
        typedef enum {
        CTRL_MOD_INDEX = 0,
        CTRL_TYP_INDEX = 1,
        CUR_MOT_MAX_INDEX = 2,
        RPM_MOT_MAX_INDEX = 3,
        FI_WEAK_ENA_INDEX = 4,
        FI_WEAK_HI_INDEX = 5,
        FI_WEAK_LO_INDEX = 6,
        FI_WEAK_MAX_INDEX = 7,
        PHASE_ADV_MAX_DEG_INDEX = 8,
        INPUT1_RAW_INDEX = 9,
        INPUT1_TYP_INDEX = 10,
        INPUT1_MIN_INDEX = 11,
        INPUT1_MID_INDEX = 12,
        INPUT1_MAX_INDEX = 13,
        INPUT1_CMD_INDEX = 14,
        INPUT2_RAW_INDEX = 15,
        INPUT2_TYP_INDEX = 16,
        INPUT2_MIN_INDEX = 17,
        INPUT2_MID_INDEX = 18,
        INPUT2_MAX_INDEX = 19,
        INPUT2_CMD_INDEX = 20,
        AUX_INPUT1_RAW_INDEX = 21,
        AUX_INPUT1_TYP_INDEX = 22,
        AUX_INPUT1_MIN_INDEX = 23,
        AUX_INPUT1_MID_INDEX = 24,
        AUX_INPUT1_MAX_INDEX = 25,
        AUX_INPUT1_CMD_INDEX = 26,
        AUX_INPUT2_RAW_INDEX = 27,
        AUX_INPUT2_TYP_INDEX = 28,
        AUX_INPUT2_MIN_INDEX = 29,
        AUX_INPUT2_MID_INDEX = 30,
        AUX_INPUT2_MAX_INDEX = 31,
        AUX_INPUT2_CMD_INDEX = 32,
        DC_CURR_INDEX = 33,
        RDC_CURR_INDEX = 34,
        LDC_CURR_INDEX = 35,
        CMDL_INDEX = 36,
        CMDR_INDEX = 37,
        SPD_AVG_INDEX = 38,
        SPDL_INDEX = 39,
        SPDR_INDEX = 40,
        FILTER_RATE_INDEX = 41,
        SPD_COEF_INDEX = 42,
        STR_COEF_INDEX = 43,
        BATV_INDEX = 44,
        TEMP_INDEX = 45,
    } Field;
    static Result<Bytes> json_serialize(const HoverboardInfo&);
    static Result<HoverboardInfo*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const HoverboardInfo&);
    static Result<HoverboardInfo*> cbor_deserialize(const Bytes&);
};

class HoverboardCmd : public Msg {
    MSG(HoverboardCmd);
    public:
    std::optional<int32_t> speed;
    std::optional<int32_t> steer;
    
    // Field indexes
        typedef enum {
        SPEED_INDEX = 0,
        STEER_INDEX = 1,
    } Field;
    static Result<Bytes> json_serialize(const HoverboardCmd&);
    static Result<HoverboardCmd*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const HoverboardCmd&);
    static Result<HoverboardCmd*> cbor_deserialize(const Bytes&);
};



Result<Bytes> Sample::json_serialize(const Sample& msg)  {
        JsonDocument doc;
        if (msg.flag)doc["flag"] = *msg.flag;
        if (msg.identifier)doc["identifier"] = *msg.identifier;
        if (msg.name)doc["name"] = *msg.name;
        if (msg.values.size()) {
                    JsonArray arr = doc["values"].to<JsonArray>();
                    for (const auto& item : msg.values) {
                        arr.add(item);
                    }
                }
        if (msg.f)doc["f"] = *msg.f;
        if (msg.d)doc["d"] = *msg.d;
        if (msg.data)
                        doc["data"] = base64_encode(*msg.data);
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<Sample*> Sample::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Sample* msg = new Sample();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<Sample*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<Sample*>::Ok(msg);
    }


Result<Bytes> ZenohInfo::json_serialize(const ZenohInfo& msg)  {
        JsonDocument doc;
        if (msg.zid)doc["zid"] = *msg.zid;
        if (msg.what_am_i)doc["what_am_i"] = *msg.what_am_i;
        if (msg.peers.size()) {
                    JsonArray arr = doc["peers"].to<JsonArray>();
                    for (const auto& item : msg.peers) {
                        arr.add(item);
                    }
                }
        if (msg.prefix)doc["prefix"] = *msg.prefix;
        if (msg.routers.size()) {
                    JsonArray arr = doc["routers"].to<JsonArray>();
                    for (const auto& item : msg.routers) {
                        arr.add(item);
                    }
                }
        if (msg.connect)doc["connect"] = *msg.connect;
        if (msg.listen)doc["listen"] = *msg.listen;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<ZenohInfo*> ZenohInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        ZenohInfo* msg = new ZenohInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<ZenohInfo*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<ZenohInfo*>::Ok(msg);
    }


Result<Bytes> LogInfo::json_serialize(const LogInfo& msg)  {
        JsonDocument doc;
        if (msg.level)doc["level"] = *msg.level;
        if (msg.message)doc["message"] = *msg.message;
        if (msg.error_code)doc["error_code"] = *msg.error_code;
        if (msg.file)doc["file"] = *msg.file;
        if (msg.line)doc["line"] = *msg.line;
        if (msg.timestamp)doc["timestamp"] = *msg.timestamp;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LogInfo*> LogInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LogInfo* msg = new LogInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LogInfo*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<LogInfo*>::Ok(msg);
    }


Result<Bytes> SysCmd::json_serialize(const SysCmd& msg)  {
        JsonDocument doc;
        doc["src"] = msg.src;
        if (msg.set_time)doc["set_time"] = *msg.set_time;
        if (msg.reboot)doc["reboot"] = *msg.reboot;
        if (msg.console)doc["console"] = *msg.console;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<SysCmd*> SysCmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        SysCmd* msg = new SysCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<SysCmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["src"].is<std::string>() )
                    msg->src = doc["src"].as<std::string>();
        if (doc["set_time"].is<uint64_t>() )  
                        msg->set_time = doc["set_time"].as<uint64_t>();
        if (doc["reboot"].is<bool>() )  
                        msg->reboot = doc["reboot"].as<bool>();
        if (doc["console"].is<std::string>() )  
                        msg->console = doc["console"].as<std::string>();
        return Result<SysCmd*>::Ok(msg);
    }


Result<Bytes> SysInfo::json_serialize(const SysInfo& msg)  {
        JsonDocument doc;
        if (msg.utc)doc["utc"] = *msg.utc;
        if (msg.uptime)doc["uptime"] = *msg.uptime;
        if (msg.free_heap)doc["free_heap"] = *msg.free_heap;
        if (msg.flash)doc["flash"] = *msg.flash;
        if (msg.cpu_board)doc["cpu_board"] = *msg.cpu_board;
        if (msg.build_date)doc["build_date"] = *msg.build_date;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<SysInfo*> SysInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        SysInfo* msg = new SysInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<SysInfo*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<SysInfo*>::Ok(msg);
    }


Result<Bytes> WifiInfo::json_serialize(const WifiInfo& msg)  {
        JsonDocument doc;
        if (msg.ssid)doc["ssid"] = *msg.ssid;
        if (msg.bssid)doc["bssid"] = *msg.bssid;
        if (msg.rssi)doc["rssi"] = *msg.rssi;
        if (msg.ip)doc["ip"] = *msg.ip;
        if (msg.mac)doc["mac"] = *msg.mac;
        if (msg.channel)doc["channel"] = *msg.channel;
        if (msg.gateway)doc["gateway"] = *msg.gateway;
        if (msg.netmask)doc["netmask"] = *msg.netmask;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<WifiInfo*> WifiInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        WifiInfo* msg = new WifiInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<WifiInfo*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<WifiInfo*>::Ok(msg);
    }


Result<Bytes> MulticastInfo::json_serialize(const MulticastInfo& msg)  {
        JsonDocument doc;
        if (msg.group)doc["group"] = *msg.group;
        if (msg.port)doc["port"] = *msg.port;
        if (msg.mtu)doc["mtu"] = *msg.mtu;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<MulticastInfo*> MulticastInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        MulticastInfo* msg = new MulticastInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<MulticastInfo*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["group"].is<std::string>() )  
                        msg->group = doc["group"].as<std::string>();
        if (doc["port"].is<int32_t>() )  
                        msg->port = doc["port"].as<int32_t>();
        if (doc["mtu"].is<uint32_t>() )  
                        msg->mtu = doc["mtu"].as<uint32_t>();
        return Result<MulticastInfo*>::Ok(msg);
    }


Result<Bytes> HoverboardInfo::json_serialize(const HoverboardInfo& msg)  {
        JsonDocument doc;
        if (msg.ctrl_mod)doc["ctrl_mod"] = *msg.ctrl_mod;
        if (msg.ctrl_typ)doc["ctrl_typ"] = *msg.ctrl_typ;
        if (msg.cur_mot_max)doc["cur_mot_max"] = *msg.cur_mot_max;
        if (msg.rpm_mot_max)doc["rpm_mot_max"] = *msg.rpm_mot_max;
        if (msg.fi_weak_ena)doc["fi_weak_ena"] = *msg.fi_weak_ena;
        if (msg.fi_weak_hi)doc["fi_weak_hi"] = *msg.fi_weak_hi;
        if (msg.fi_weak_lo)doc["fi_weak_lo"] = *msg.fi_weak_lo;
        if (msg.fi_weak_max)doc["fi_weak_max"] = *msg.fi_weak_max;
        if (msg.phase_adv_max_deg)doc["phase_adv_max_deg"] = *msg.phase_adv_max_deg;
        if (msg.input1_raw)doc["input1_raw"] = *msg.input1_raw;
        if (msg.input1_typ)doc["input1_typ"] = *msg.input1_typ;
        if (msg.input1_min)doc["input1_min"] = *msg.input1_min;
        if (msg.input1_mid)doc["input1_mid"] = *msg.input1_mid;
        if (msg.input1_max)doc["input1_max"] = *msg.input1_max;
        if (msg.input1_cmd)doc["input1_cmd"] = *msg.input1_cmd;
        if (msg.input2_raw)doc["input2_raw"] = *msg.input2_raw;
        if (msg.input2_typ)doc["input2_typ"] = *msg.input2_typ;
        if (msg.input2_min)doc["input2_min"] = *msg.input2_min;
        if (msg.input2_mid)doc["input2_mid"] = *msg.input2_mid;
        if (msg.input2_max)doc["input2_max"] = *msg.input2_max;
        if (msg.input2_cmd)doc["input2_cmd"] = *msg.input2_cmd;
        if (msg.aux_input1_raw)doc["aux_input1_raw"] = *msg.aux_input1_raw;
        if (msg.aux_input1_typ)doc["aux_input1_typ"] = *msg.aux_input1_typ;
        if (msg.aux_input1_min)doc["aux_input1_min"] = *msg.aux_input1_min;
        if (msg.aux_input1_mid)doc["aux_input1_mid"] = *msg.aux_input1_mid;
        if (msg.aux_input1_max)doc["aux_input1_max"] = *msg.aux_input1_max;
        if (msg.aux_input1_cmd)doc["aux_input1_cmd"] = *msg.aux_input1_cmd;
        if (msg.aux_input2_raw)doc["aux_input2_raw"] = *msg.aux_input2_raw;
        if (msg.aux_input2_typ)doc["aux_input2_typ"] = *msg.aux_input2_typ;
        if (msg.aux_input2_min)doc["aux_input2_min"] = *msg.aux_input2_min;
        if (msg.aux_input2_mid)doc["aux_input2_mid"] = *msg.aux_input2_mid;
        if (msg.aux_input2_max)doc["aux_input2_max"] = *msg.aux_input2_max;
        if (msg.aux_input2_cmd)doc["aux_input2_cmd"] = *msg.aux_input2_cmd;
        if (msg.dc_curr)doc["dc_curr"] = *msg.dc_curr;
        if (msg.rdc_curr)doc["rdc_curr"] = *msg.rdc_curr;
        if (msg.ldc_curr)doc["ldc_curr"] = *msg.ldc_curr;
        if (msg.cmdl)doc["cmdl"] = *msg.cmdl;
        if (msg.cmdr)doc["cmdr"] = *msg.cmdr;
        if (msg.spd_avg)doc["spd_avg"] = *msg.spd_avg;
        if (msg.spdl)doc["spdl"] = *msg.spdl;
        if (msg.spdr)doc["spdr"] = *msg.spdr;
        if (msg.filter_rate)doc["filter_rate"] = *msg.filter_rate;
        if (msg.spd_coef)doc["spd_coef"] = *msg.spd_coef;
        if (msg.str_coef)doc["str_coef"] = *msg.str_coef;
        if (msg.batv)doc["batv"] = *msg.batv;
        if (msg.temp)doc["temp"] = *msg.temp;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<HoverboardInfo*> HoverboardInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardInfo* msg = new HoverboardInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<HoverboardInfo*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["ctrl_mod"].is<CtrlMod>() )  
                        msg->ctrl_mod = doc["ctrl_mod"].as<CtrlMod>();
        if (doc["ctrl_typ"].is<CtrlTyp>() )  
                        msg->ctrl_typ = doc["ctrl_typ"].as<CtrlTyp>();
        if (doc["cur_mot_max"].is<int32_t>() )  
                        msg->cur_mot_max = doc["cur_mot_max"].as<int32_t>();
        if (doc["rpm_mot_max"].is<int32_t>() )  
                        msg->rpm_mot_max = doc["rpm_mot_max"].as<int32_t>();
        if (doc["fi_weak_ena"].is<int32_t>() )  
                        msg->fi_weak_ena = doc["fi_weak_ena"].as<int32_t>();
        if (doc["fi_weak_hi"].is<int32_t>() )  
                        msg->fi_weak_hi = doc["fi_weak_hi"].as<int32_t>();
        if (doc["fi_weak_lo"].is<int32_t>() )  
                        msg->fi_weak_lo = doc["fi_weak_lo"].as<int32_t>();
        if (doc["fi_weak_max"].is<int32_t>() )  
                        msg->fi_weak_max = doc["fi_weak_max"].as<int32_t>();
        if (doc["phase_adv_max_deg"].is<int32_t>() )  
                        msg->phase_adv_max_deg = doc["phase_adv_max_deg"].as<int32_t>();
        if (doc["input1_raw"].is<int32_t>() )  
                        msg->input1_raw = doc["input1_raw"].as<int32_t>();
        if (doc["input1_typ"].is<InTyp>() )  
                        msg->input1_typ = doc["input1_typ"].as<InTyp>();
        if (doc["input1_min"].is<int32_t>() )  
                        msg->input1_min = doc["input1_min"].as<int32_t>();
        if (doc["input1_mid"].is<int32_t>() )  
                        msg->input1_mid = doc["input1_mid"].as<int32_t>();
        if (doc["input1_max"].is<int32_t>() )  
                        msg->input1_max = doc["input1_max"].as<int32_t>();
        if (doc["input1_cmd"].is<int32_t>() )  
                        msg->input1_cmd = doc["input1_cmd"].as<int32_t>();
        if (doc["input2_raw"].is<int32_t>() )  
                        msg->input2_raw = doc["input2_raw"].as<int32_t>();
        if (doc["input2_typ"].is<InTyp>() )  
                        msg->input2_typ = doc["input2_typ"].as<InTyp>();
        if (doc["input2_min"].is<int32_t>() )  
                        msg->input2_min = doc["input2_min"].as<int32_t>();
        if (doc["input2_mid"].is<int32_t>() )  
                        msg->input2_mid = doc["input2_mid"].as<int32_t>();
        if (doc["input2_max"].is<int32_t>() )  
                        msg->input2_max = doc["input2_max"].as<int32_t>();
        if (doc["input2_cmd"].is<int32_t>() )  
                        msg->input2_cmd = doc["input2_cmd"].as<int32_t>();
        if (doc["aux_input1_raw"].is<int32_t>() )  
                        msg->aux_input1_raw = doc["aux_input1_raw"].as<int32_t>();
        if (doc["aux_input1_typ"].is<InTyp>() )  
                        msg->aux_input1_typ = doc["aux_input1_typ"].as<InTyp>();
        if (doc["aux_input1_min"].is<int32_t>() )  
                        msg->aux_input1_min = doc["aux_input1_min"].as<int32_t>();
        if (doc["aux_input1_mid"].is<int32_t>() )  
                        msg->aux_input1_mid = doc["aux_input1_mid"].as<int32_t>();
        if (doc["aux_input1_max"].is<int32_t>() )  
                        msg->aux_input1_max = doc["aux_input1_max"].as<int32_t>();
        if (doc["aux_input1_cmd"].is<int32_t>() )  
                        msg->aux_input1_cmd = doc["aux_input1_cmd"].as<int32_t>();
        if (doc["aux_input2_raw"].is<int32_t>() )  
                        msg->aux_input2_raw = doc["aux_input2_raw"].as<int32_t>();
        if (doc["aux_input2_typ"].is<InTyp>() )  
                        msg->aux_input2_typ = doc["aux_input2_typ"].as<InTyp>();
        if (doc["aux_input2_min"].is<int32_t>() )  
                        msg->aux_input2_min = doc["aux_input2_min"].as<int32_t>();
        if (doc["aux_input2_mid"].is<int32_t>() )  
                        msg->aux_input2_mid = doc["aux_input2_mid"].as<int32_t>();
        if (doc["aux_input2_max"].is<int32_t>() )  
                        msg->aux_input2_max = doc["aux_input2_max"].as<int32_t>();
        if (doc["aux_input2_cmd"].is<int32_t>() )  
                        msg->aux_input2_cmd = doc["aux_input2_cmd"].as<int32_t>();
        if (doc["dc_curr"].is<int32_t>() )  
                        msg->dc_curr = doc["dc_curr"].as<int32_t>();
        if (doc["rdc_curr"].is<int32_t>() )  
                        msg->rdc_curr = doc["rdc_curr"].as<int32_t>();
        if (doc["ldc_curr"].is<int32_t>() )  
                        msg->ldc_curr = doc["ldc_curr"].as<int32_t>();
        if (doc["cmdl"].is<int32_t>() )  
                        msg->cmdl = doc["cmdl"].as<int32_t>();
        if (doc["cmdr"].is<int32_t>() )  
                        msg->cmdr = doc["cmdr"].as<int32_t>();
        if (doc["spd_avg"].is<int32_t>() )  
                        msg->spd_avg = doc["spd_avg"].as<int32_t>();
        if (doc["spdl"].is<int32_t>() )  
                        msg->spdl = doc["spdl"].as<int32_t>();
        if (doc["spdr"].is<int32_t>() )  
                        msg->spdr = doc["spdr"].as<int32_t>();
        if (doc["filter_rate"].is<int32_t>() )  
                        msg->filter_rate = doc["filter_rate"].as<int32_t>();
        if (doc["spd_coef"].is<int32_t>() )  
                        msg->spd_coef = doc["spd_coef"].as<int32_t>();
        if (doc["str_coef"].is<int32_t>() )  
                        msg->str_coef = doc["str_coef"].as<int32_t>();
        if (doc["batv"].is<int32_t>() )  
                        msg->batv = doc["batv"].as<int32_t>();
        if (doc["temp"].is<int32_t>() )  
                        msg->temp = doc["temp"].as<int32_t>();
        return Result<HoverboardInfo*>::Ok(msg);
    }


Result<Bytes> HoverboardCmd::json_serialize(const HoverboardCmd& msg)  {
        JsonDocument doc;
        if (msg.speed)doc["speed"] = *msg.speed;
        if (msg.steer)doc["steer"] = *msg.steer;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<HoverboardCmd*> HoverboardCmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardCmd* msg = new HoverboardCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<HoverboardCmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["speed"].is<int32_t>() )  
                        msg->speed = doc["speed"].as<int32_t>();
        if (doc["steer"].is<int32_t>() )  
                        msg->steer = doc["steer"].as<int32_t>();
        return Result<HoverboardCmd*>::Ok(msg);
    }


