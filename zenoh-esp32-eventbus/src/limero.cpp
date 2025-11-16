#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <cbor.h>
#include <msg.h>
#include <serdes.h>

#include "limero.h"


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


Result<Bytes> Ps4Info::json_serialize(const Ps4Info& msg)  {
        JsonDocument doc;
        if (msg.button_left)doc["button_left"] = *msg.button_left;
        if (msg.button_right)doc["button_right"] = *msg.button_right;
        if (msg.button_up)doc["button_up"] = *msg.button_up;
        if (msg.button_down)doc["button_down"] = *msg.button_down;
        if (msg.button_square)doc["button_square"] = *msg.button_square;
        if (msg.button_cross)doc["button_cross"] = *msg.button_cross;
        if (msg.button_circle)doc["button_circle"] = *msg.button_circle;
        if (msg.button_triangle)doc["button_triangle"] = *msg.button_triangle;
        if (msg.button_left_sholder)doc["button_left_sholder"] = *msg.button_left_sholder;
        if (msg.button_right_sholder)doc["button_right_sholder"] = *msg.button_right_sholder;
        if (msg.button_left_trigger)doc["button_left_trigger"] = *msg.button_left_trigger;
        if (msg.button_right_trigger)doc["button_right_trigger"] = *msg.button_right_trigger;
        if (msg.button_left_joystick)doc["button_left_joystick"] = *msg.button_left_joystick;
        if (msg.button_right_joystick)doc["button_right_joystick"] = *msg.button_right_joystick;
        if (msg.button_share)doc["button_share"] = *msg.button_share;
        if (msg.axis_lx)doc["axis_lx"] = *msg.axis_lx;
        if (msg.axis_ly)doc["axis_ly"] = *msg.axis_ly;
        if (msg.axis_rx)doc["axis_rx"] = *msg.axis_rx;
        if (msg.axis_ry)doc["axis_ry"] = *msg.axis_ry;
        if (msg.gyro_x)doc["gyro_x"] = *msg.gyro_x;
        if (msg.gyro_y)doc["gyro_y"] = *msg.gyro_y;
        if (msg.gyro_z)doc["gyro_z"] = *msg.gyro_z;
        if (msg.accel_x)doc["accel_x"] = *msg.accel_x;
        if (msg.accel_y)doc["accel_y"] = *msg.accel_y;
        if (msg.accel_z)doc["accel_z"] = *msg.accel_z;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<Ps4Info*> Ps4Info::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Ps4Info* msg = new Ps4Info();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<Ps4Info*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["button_left"].is<bool>() )  
                        msg->button_left = doc["button_left"].as<bool>();
        if (doc["button_right"].is<bool>() )  
                        msg->button_right = doc["button_right"].as<bool>();
        if (doc["button_up"].is<bool>() )  
                        msg->button_up = doc["button_up"].as<bool>();
        if (doc["button_down"].is<bool>() )  
                        msg->button_down = doc["button_down"].as<bool>();
        if (doc["button_square"].is<bool>() )  
                        msg->button_square = doc["button_square"].as<bool>();
        if (doc["button_cross"].is<bool>() )  
                        msg->button_cross = doc["button_cross"].as<bool>();
        if (doc["button_circle"].is<bool>() )  
                        msg->button_circle = doc["button_circle"].as<bool>();
        if (doc["button_triangle"].is<bool>() )  
                        msg->button_triangle = doc["button_triangle"].as<bool>();
        if (doc["button_left_sholder"].is<bool>() )  
                        msg->button_left_sholder = doc["button_left_sholder"].as<bool>();
        if (doc["button_right_sholder"].is<bool>() )  
                        msg->button_right_sholder = doc["button_right_sholder"].as<bool>();
        if (doc["button_left_trigger"].is<bool>() )  
                        msg->button_left_trigger = doc["button_left_trigger"].as<bool>();
        if (doc["button_right_trigger"].is<bool>() )  
                        msg->button_right_trigger = doc["button_right_trigger"].as<bool>();
        if (doc["button_left_joystick"].is<bool>() )  
                        msg->button_left_joystick = doc["button_left_joystick"].as<bool>();
        if (doc["button_right_joystick"].is<bool>() )  
                        msg->button_right_joystick = doc["button_right_joystick"].as<bool>();
        if (doc["button_share"].is<bool>() )  
                        msg->button_share = doc["button_share"].as<bool>();
        if (doc["axis_lx"].is<int32_t>() )  
                        msg->axis_lx = doc["axis_lx"].as<int32_t>();
        if (doc["axis_ly"].is<int32_t>() )  
                        msg->axis_ly = doc["axis_ly"].as<int32_t>();
        if (doc["axis_rx"].is<int32_t>() )  
                        msg->axis_rx = doc["axis_rx"].as<int32_t>();
        if (doc["axis_ry"].is<int32_t>() )  
                        msg->axis_ry = doc["axis_ry"].as<int32_t>();
        if (doc["gyro_x"].is<int32_t>() )  
                        msg->gyro_x = doc["gyro_x"].as<int32_t>();
        if (doc["gyro_y"].is<int32_t>() )  
                        msg->gyro_y = doc["gyro_y"].as<int32_t>();
        if (doc["gyro_z"].is<int32_t>() )  
                        msg->gyro_z = doc["gyro_z"].as<int32_t>();
        if (doc["accel_x"].is<int32_t>() )  
                        msg->accel_x = doc["accel_x"].as<int32_t>();
        if (doc["accel_y"].is<int32_t>() )  
                        msg->accel_y = doc["accel_y"].as<int32_t>();
        if (doc["accel_z"].is<int32_t>() )  
                        msg->accel_z = doc["accel_z"].as<int32_t>();
        return Result<Ps4Info*>::Ok(msg);
    }


Result<Bytes> Ps4Cmd::json_serialize(const Ps4Cmd& msg)  {
        JsonDocument doc;
        if (msg.rumble)doc["rumble"] = *msg.rumble;
        if (msg.led_rgb)doc["led_rgb"] = *msg.led_rgb;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<Ps4Cmd*> Ps4Cmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Ps4Cmd* msg = new Ps4Cmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<Ps4Cmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["rumble"].is<int32_t>() )  
                        msg->rumble = doc["rumble"].as<int32_t>();
        if (doc["led_rgb"].is<int32_t>() )  
                        msg->led_rgb = doc["led_rgb"].as<int32_t>();
        return Result<Ps4Cmd*>::Ok(msg);
    }


Result<Bytes> CameraInfo::json_serialize(const CameraInfo& msg)  {
        JsonDocument doc;
        if (msg.width)doc["width"] = *msg.width;
        if (msg.height)doc["height"] = *msg.height;
        if (msg.format)doc["format"] = *msg.format;
        if (msg.data)
                        doc["data"] = base64_encode(*msg.data);
        if (msg.led)doc["led"] = *msg.led;
        if (msg.quality)doc["quality"] = *msg.quality;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<CameraInfo*> CameraInfo::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        CameraInfo* msg = new CameraInfo();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<CameraInfo*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["width"].is<int32_t>() )  
                        msg->width = doc["width"].as<int32_t>();
        if (doc["height"].is<int32_t>() )  
                        msg->height = doc["height"].as<int32_t>();
        if (doc["format"].is<std::string>() )  
                        msg->format = doc["format"].as<std::string>();
        if (doc["data"].is<std::string>() )  
                        msg->data = base64_decode(doc["data"].as<std::string>());
        if (doc["led"].is<bool>() )  
                        msg->led = doc["led"].as<bool>();
        if (doc["quality"].is<int32_t>() )  
                        msg->quality = doc["quality"].as<int32_t>();
        return Result<CameraInfo*>::Ok(msg);
    }


Result<Bytes> CameraCmd::json_serialize(const CameraCmd& msg)  {
        JsonDocument doc;
        if (msg.led)doc["led"] = *msg.led;
        if (msg.quality)doc["quality"] = *msg.quality;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<CameraCmd*> CameraCmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        CameraCmd* msg = new CameraCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<CameraCmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["led"].is<bool>() )  
                        msg->led = doc["led"].as<bool>();
        if (doc["quality"].is<int32_t>() )  
                        msg->quality = doc["quality"].as<int32_t>();
        return Result<CameraCmd*>::Ok(msg);
    }



// Helper macros for serialization and deserialization




// CBOR Serialization/Deserialization


Result<Bytes> Sample::cbor_serialize(const Sample& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.flag) {
            cbor_encode_int(&mapEncoder, Sample::Field::FLAG_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.flag.value());
            }
    if (msg.identifier) {
            cbor_encode_int(&mapEncoder, Sample::Field::IDENTIFIER_INDEX);
            cbor_encode_int(&mapEncoder, msg.identifier.value());
            }
    if (msg.name) {
            cbor_encode_int(&mapEncoder, Sample::Field::NAME_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.name.value().c_str());
            }
    {
            CborEncoder arrayEncoder;
            cbor_encode_int(&mapEncoder, Sample::Field::VALUES_INDEX );
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.values.size());
            for (const auto & item : msg.values) {
                cbor_encode_float(&arrayEncoder, item);
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
            }
    if (msg.f) {
            cbor_encode_int(&mapEncoder, Sample::Field::F_INDEX);
            cbor_encode_float(&mapEncoder, msg.f.value());
            }
    if (msg.d) {
            cbor_encode_int(&mapEncoder, Sample::Field::D_INDEX);
            cbor_encode_double(&mapEncoder, msg.d.value());
            }
    if (msg.data) {
            cbor_encode_int(&mapEncoder, Sample::Field::DATA_INDEX);
            cbor_encode_byte_string(&mapEncoder, msg.data.value().data(), msg.data.value().size());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<Sample*> Sample::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    Sample* msg = new Sample();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<Sample*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Sample*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Sample*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<Sample*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case Sample::Field::FLAG_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->flag = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Sample::Field::IDENTIFIER_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->identifier = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Sample::Field::NAME_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->name = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Sample::Field::VALUES_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    float v;
                    float f;
    cbor_value_get_float(&tmp, &f);
    v = f;
    cbor_value_advance(&tmp);

                    msg->values.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case Sample::Field::F_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->f = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Sample::Field::D_INDEX:{double d;
    cbor_value_get_double(&mapIt, &d);
    msg->d = d;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Sample::Field::DATA_INDEX:{{
    uint8_t tmpbuf[512];
    size_t tmplen = sizeof(tmpbuf);
    if (cbor_value_is_byte_string(&mapIt)) {
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, NULL);
        msg->data = Bytes(tmpbuf, tmpbuf + tmplen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<Sample*>::Ok(msg);
}

Result<Bytes> ZenohInfo::cbor_serialize(const ZenohInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.zid) {
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::ZID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.zid.value().c_str());
            }
    if (msg.what_am_i) {
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::WHAT_AM_I_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.what_am_i.value().c_str());
            }
    {
            CborEncoder arrayEncoder;
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::PEERS_INDEX );
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.peers.size());
            for (const auto & item : msg.peers) {
                cbor_encode_text_stringz(&arrayEncoder, item.c_str());
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
            }
    if (msg.prefix) {
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::PREFIX_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.prefix.value().c_str());
            }
    {
            CborEncoder arrayEncoder;
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::ROUTERS_INDEX );
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.routers.size());
            for (const auto & item : msg.routers) {
                cbor_encode_text_stringz(&arrayEncoder, item.c_str());
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
            }
    if (msg.connect) {
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::CONNECT_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.connect.value().c_str());
            }
    if (msg.listen) {
            cbor_encode_int(&mapEncoder, ZenohInfo::Field::LISTEN_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.listen.value().c_str());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<ZenohInfo*> ZenohInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    ZenohInfo* msg = new ZenohInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<ZenohInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<ZenohInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<ZenohInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<ZenohInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case ZenohInfo::Field::ZID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->zid = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohInfo::Field::WHAT_AM_I_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->what_am_i = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohInfo::Field::PEERS_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&tmp)) {
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&tmp);

                    msg->peers.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case ZenohInfo::Field::PREFIX_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->prefix = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohInfo::Field::ROUTERS_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&tmp)) {
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&tmp);

                    msg->routers.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case ZenohInfo::Field::CONNECT_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->connect = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohInfo::Field::LISTEN_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->listen = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<ZenohInfo*>::Ok(msg);
}

Result<Bytes> LogInfo::cbor_serialize(const LogInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.level) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::LEVEL_INDEX);
            cbor_encode_int(&mapEncoder, msg.level.value());
            }
    if (msg.message) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::MESSAGE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str());
            }
    if (msg.error_code) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::ERROR_CODE_INDEX);
            cbor_encode_int(&mapEncoder, msg.error_code.value());
            }
    if (msg.file) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::FILE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.file.value().c_str());
            }
    if (msg.line) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::LINE_INDEX);
            cbor_encode_int(&mapEncoder, msg.line.value());
            }
    if (msg.timestamp) {
            cbor_encode_int(&mapEncoder, LogInfo::Field::TIMESTAMP_INDEX);
            cbor_encode_int(&mapEncoder, msg.timestamp.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<LogInfo*> LogInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LogInfo* msg = new LogInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LogInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LogInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LogInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<LogInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LogInfo::Field::LEVEL_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->level = static_cast<LogLevel>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogInfo::Field::MESSAGE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->message = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogInfo::Field::ERROR_CODE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->error_code = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogInfo::Field::FILE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->file = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogInfo::Field::LINE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->line = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogInfo::Field::TIMESTAMP_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->timestamp = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<LogInfo*>::Ok(msg);
}

Result<Bytes> SysCmd::cbor_serialize(const SysCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    // field: src
            cbor_encode_int(&mapEncoder, SysCmd::Field::SRC_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.src.c_str());
    if (msg.set_time) {
            cbor_encode_int(&mapEncoder, SysCmd::Field::SET_TIME_INDEX);
            cbor_encode_int(&mapEncoder, msg.set_time.value());
            }
    if (msg.reboot) {
            cbor_encode_int(&mapEncoder, SysCmd::Field::REBOOT_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.reboot.value());
            }
    if (msg.console) {
            cbor_encode_int(&mapEncoder, SysCmd::Field::CONSOLE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.console.value().c_str());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<SysCmd*> SysCmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    SysCmd* msg = new SysCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<SysCmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<SysCmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<SysCmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<SysCmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case SysCmd::Field::SRC_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->src = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysCmd::Field::SET_TIME_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->set_time = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysCmd::Field::REBOOT_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->reboot = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysCmd::Field::CONSOLE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->console = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<SysCmd*>::Ok(msg);
}

Result<Bytes> SysInfo::cbor_serialize(const SysInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.utc) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::UTC_INDEX);
            cbor_encode_int(&mapEncoder, msg.utc.value());
            }
    if (msg.uptime) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::UPTIME_INDEX);
            cbor_encode_int(&mapEncoder, msg.uptime.value());
            }
    if (msg.free_heap) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::FREE_HEAP_INDEX);
            cbor_encode_int(&mapEncoder, msg.free_heap.value());
            }
    if (msg.flash) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::FLASH_INDEX);
            cbor_encode_int(&mapEncoder, msg.flash.value());
            }
    if (msg.cpu_board) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::CPU_BOARD_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.cpu_board.value().c_str());
            }
    if (msg.build_date) {
            cbor_encode_int(&mapEncoder, SysInfo::Field::BUILD_DATE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.build_date.value().c_str());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<SysInfo*> SysInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    SysInfo* msg = new SysInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<SysInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<SysInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<SysInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<SysInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case SysInfo::Field::UTC_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->utc = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysInfo::Field::UPTIME_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->uptime = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysInfo::Field::FREE_HEAP_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->free_heap = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysInfo::Field::FLASH_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->flash = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysInfo::Field::CPU_BOARD_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->cpu_board = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysInfo::Field::BUILD_DATE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->build_date = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<SysInfo*>::Ok(msg);
}

Result<Bytes> WifiInfo::cbor_serialize(const WifiInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.ssid) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::SSID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.ssid.value().c_str());
            }
    if (msg.bssid) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::BSSID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.bssid.value().c_str());
            }
    if (msg.rssi) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::RSSI_INDEX);
            cbor_encode_int(&mapEncoder, msg.rssi.value());
            }
    if (msg.ip) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::IP_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.ip.value().c_str());
            }
    if (msg.mac) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::MAC_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.mac.value().c_str());
            }
    if (msg.channel) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::CHANNEL_INDEX);
            cbor_encode_int(&mapEncoder, msg.channel.value());
            }
    if (msg.gateway) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::GATEWAY_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.gateway.value().c_str());
            }
    if (msg.netmask) {
            cbor_encode_int(&mapEncoder, WifiInfo::Field::NETMASK_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.netmask.value().c_str());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<WifiInfo*> WifiInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    WifiInfo* msg = new WifiInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<WifiInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<WifiInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<WifiInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<WifiInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case WifiInfo::Field::SSID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->ssid = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::BSSID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->bssid = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::RSSI_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rssi = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::IP_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->ip = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::MAC_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->mac = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::CHANNEL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->channel = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::GATEWAY_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->gateway = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiInfo::Field::NETMASK_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->netmask = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<WifiInfo*>::Ok(msg);
}

Result<Bytes> MulticastInfo::cbor_serialize(const MulticastInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.group) {
            cbor_encode_int(&mapEncoder, MulticastInfo::Field::GROUP_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.group.value().c_str());
            }
    if (msg.port) {
            cbor_encode_int(&mapEncoder, MulticastInfo::Field::PORT_INDEX);
            cbor_encode_int(&mapEncoder, msg.port.value());
            }
    if (msg.mtu) {
            cbor_encode_int(&mapEncoder, MulticastInfo::Field::MTU_INDEX);
            cbor_encode_int(&mapEncoder, msg.mtu.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<MulticastInfo*> MulticastInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    MulticastInfo* msg = new MulticastInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<MulticastInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<MulticastInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<MulticastInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<MulticastInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case MulticastInfo::Field::GROUP_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->group = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MulticastInfo::Field::PORT_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->port = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MulticastInfo::Field::MTU_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->mtu = v;
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<MulticastInfo*>::Ok(msg);
}

Result<Bytes> HoverboardInfo::cbor_serialize(const HoverboardInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.ctrl_mod) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CTRL_MOD_INDEX);
            cbor_encode_int(&mapEncoder, msg.ctrl_mod.value());
            }
    if (msg.ctrl_typ) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CTRL_TYP_INDEX);
            cbor_encode_int(&mapEncoder, msg.ctrl_typ.value());
            }
    if (msg.cur_mot_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CUR_MOT_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.cur_mot_max.value());
            }
    if (msg.rpm_mot_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::RPM_MOT_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.rpm_mot_max.value());
            }
    if (msg.fi_weak_ena) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_ENA_INDEX);
            cbor_encode_int(&mapEncoder, msg.fi_weak_ena.value());
            }
    if (msg.fi_weak_hi) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_HI_INDEX);
            cbor_encode_int(&mapEncoder, msg.fi_weak_hi.value());
            }
    if (msg.fi_weak_lo) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_LO_INDEX);
            cbor_encode_int(&mapEncoder, msg.fi_weak_lo.value());
            }
    if (msg.fi_weak_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.fi_weak_max.value());
            }
    if (msg.phase_adv_max_deg) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::PHASE_ADV_MAX_DEG_INDEX);
            cbor_encode_int(&mapEncoder, msg.phase_adv_max_deg.value());
            }
    if (msg.input1_raw) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_RAW_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_raw.value());
            }
    if (msg.input1_typ) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_TYP_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_typ.value());
            }
    if (msg.input1_min) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MIN_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_min.value());
            }
    if (msg.input1_mid) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MID_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_mid.value());
            }
    if (msg.input1_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_max.value());
            }
    if (msg.input1_cmd) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_CMD_INDEX);
            cbor_encode_int(&mapEncoder, msg.input1_cmd.value());
            }
    if (msg.input2_raw) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_RAW_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_raw.value());
            }
    if (msg.input2_typ) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_TYP_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_typ.value());
            }
    if (msg.input2_min) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MIN_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_min.value());
            }
    if (msg.input2_mid) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MID_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_mid.value());
            }
    if (msg.input2_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_max.value());
            }
    if (msg.input2_cmd) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_CMD_INDEX);
            cbor_encode_int(&mapEncoder, msg.input2_cmd.value());
            }
    if (msg.aux_input1_raw) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_RAW_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_raw.value());
            }
    if (msg.aux_input1_typ) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_TYP_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_typ.value());
            }
    if (msg.aux_input1_min) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MIN_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_min.value());
            }
    if (msg.aux_input1_mid) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MID_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_mid.value());
            }
    if (msg.aux_input1_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_max.value());
            }
    if (msg.aux_input1_cmd) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_CMD_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input1_cmd.value());
            }
    if (msg.aux_input2_raw) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_RAW_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_raw.value());
            }
    if (msg.aux_input2_typ) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_TYP_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_typ.value());
            }
    if (msg.aux_input2_min) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MIN_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_min.value());
            }
    if (msg.aux_input2_mid) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MID_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_mid.value());
            }
    if (msg.aux_input2_max) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MAX_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_max.value());
            }
    if (msg.aux_input2_cmd) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_CMD_INDEX);
            cbor_encode_int(&mapEncoder, msg.aux_input2_cmd.value());
            }
    if (msg.dc_curr) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::DC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, msg.dc_curr.value());
            }
    if (msg.rdc_curr) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::RDC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, msg.rdc_curr.value());
            }
    if (msg.ldc_curr) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::LDC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, msg.ldc_curr.value());
            }
    if (msg.cmdl) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CMDL_INDEX);
            cbor_encode_int(&mapEncoder, msg.cmdl.value());
            }
    if (msg.cmdr) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CMDR_INDEX);
            cbor_encode_int(&mapEncoder, msg.cmdr.value());
            }
    if (msg.spd_avg) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPD_AVG_INDEX);
            cbor_encode_int(&mapEncoder, msg.spd_avg.value());
            }
    if (msg.spdl) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPDL_INDEX);
            cbor_encode_int(&mapEncoder, msg.spdl.value());
            }
    if (msg.spdr) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPDR_INDEX);
            cbor_encode_int(&mapEncoder, msg.spdr.value());
            }
    if (msg.filter_rate) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FILTER_RATE_INDEX);
            cbor_encode_int(&mapEncoder, msg.filter_rate.value());
            }
    if (msg.spd_coef) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPD_COEF_INDEX);
            cbor_encode_int(&mapEncoder, msg.spd_coef.value());
            }
    if (msg.str_coef) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::STR_COEF_INDEX);
            cbor_encode_int(&mapEncoder, msg.str_coef.value());
            }
    if (msg.batv) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::BATV_INDEX);
            cbor_encode_int(&mapEncoder, msg.batv.value());
            }
    if (msg.temp) {
            cbor_encode_int(&mapEncoder, HoverboardInfo::Field::TEMP_INDEX);
            cbor_encode_int(&mapEncoder, msg.temp.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<HoverboardInfo*> HoverboardInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    HoverboardInfo* msg = new HoverboardInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<HoverboardInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<HoverboardInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case HoverboardInfo::Field::CTRL_MOD_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->ctrl_mod = static_cast<CtrlMod>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::CTRL_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->ctrl_typ = static_cast<CtrlTyp>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::CUR_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cur_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::RPM_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rpm_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::FI_WEAK_ENA_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_ena = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::FI_WEAK_HI_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_hi = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::FI_WEAK_LO_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_lo = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::FI_WEAK_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::PHASE_ADV_MAX_DEG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->phase_adv_max_deg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->input1_typ = static_cast<InTyp>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->input2_typ = static_cast<InTyp>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->aux_input1_typ = static_cast<InTyp>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->aux_input2_typ = static_cast<InTyp>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::AUX_INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::DC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->dc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::RDC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rdc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::LDC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ldc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::CMDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::CMDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::SPD_AVG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_avg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::SPDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::SPDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::FILTER_RATE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->filter_rate = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::SPD_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::STR_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->str_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::BATV_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->batv = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardInfo::Field::TEMP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->temp = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<HoverboardInfo*>::Ok(msg);
}

Result<Bytes> HoverboardCmd::cbor_serialize(const HoverboardCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.speed) {
            cbor_encode_int(&mapEncoder, HoverboardCmd::Field::SPEED_INDEX);
            cbor_encode_int(&mapEncoder, msg.speed.value());
            }
    if (msg.steer) {
            cbor_encode_int(&mapEncoder, HoverboardCmd::Field::STEER_INDEX);
            cbor_encode_int(&mapEncoder, msg.steer.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<HoverboardCmd*> HoverboardCmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    HoverboardCmd* msg = new HoverboardCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<HoverboardCmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardCmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardCmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<HoverboardCmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case HoverboardCmd::Field::SPEED_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->speed = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardCmd::Field::STEER_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->steer = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<HoverboardCmd*>::Ok(msg);
}

Result<Bytes> Ps4Info::cbor_serialize(const Ps4Info& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.button_left) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_LEFT_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_left.value());
            }
    if (msg.button_right) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_RIGHT_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_right.value());
            }
    if (msg.button_up) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_UP_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_up.value());
            }
    if (msg.button_down) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_DOWN_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_down.value());
            }
    if (msg.button_square) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_SQUARE_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_square.value());
            }
    if (msg.button_cross) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_CROSS_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_cross.value());
            }
    if (msg.button_circle) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_CIRCLE_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_circle.value());
            }
    if (msg.button_triangle) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_TRIANGLE_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_triangle.value());
            }
    if (msg.button_left_sholder) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_LEFT_SHOLDER_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_left_sholder.value());
            }
    if (msg.button_right_sholder) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_RIGHT_SHOLDER_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_right_sholder.value());
            }
    if (msg.button_left_trigger) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_LEFT_TRIGGER_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_left_trigger.value());
            }
    if (msg.button_right_trigger) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_RIGHT_TRIGGER_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_right_trigger.value());
            }
    if (msg.button_left_joystick) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_LEFT_JOYSTICK_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_left_joystick.value());
            }
    if (msg.button_right_joystick) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_RIGHT_JOYSTICK_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_right_joystick.value());
            }
    if (msg.button_share) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::BUTTON_SHARE_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.button_share.value());
            }
    if (msg.axis_lx) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::AXIS_LX_INDEX);
            cbor_encode_int(&mapEncoder, msg.axis_lx.value());
            }
    if (msg.axis_ly) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::AXIS_LY_INDEX);
            cbor_encode_int(&mapEncoder, msg.axis_ly.value());
            }
    if (msg.axis_rx) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::AXIS_RX_INDEX);
            cbor_encode_int(&mapEncoder, msg.axis_rx.value());
            }
    if (msg.axis_ry) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::AXIS_RY_INDEX);
            cbor_encode_int(&mapEncoder, msg.axis_ry.value());
            }
    if (msg.gyro_x) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::GYRO_X_INDEX);
            cbor_encode_int(&mapEncoder, msg.gyro_x.value());
            }
    if (msg.gyro_y) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::GYRO_Y_INDEX);
            cbor_encode_int(&mapEncoder, msg.gyro_y.value());
            }
    if (msg.gyro_z) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::GYRO_Z_INDEX);
            cbor_encode_int(&mapEncoder, msg.gyro_z.value());
            }
    if (msg.accel_x) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::ACCEL_X_INDEX);
            cbor_encode_int(&mapEncoder, msg.accel_x.value());
            }
    if (msg.accel_y) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::ACCEL_Y_INDEX);
            cbor_encode_int(&mapEncoder, msg.accel_y.value());
            }
    if (msg.accel_z) {
            cbor_encode_int(&mapEncoder, Ps4Info::Field::ACCEL_Z_INDEX);
            cbor_encode_int(&mapEncoder, msg.accel_z.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<Ps4Info*> Ps4Info::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    Ps4Info* msg = new Ps4Info();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<Ps4Info*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Ps4Info*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Ps4Info*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<Ps4Info*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case Ps4Info::Field::BUTTON_LEFT_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_RIGHT_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_UP_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_up = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_DOWN_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_down = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_SQUARE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_square = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_CROSS_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_cross = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_CIRCLE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_circle = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_TRIANGLE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_triangle = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_LEFT_SHOLDER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_sholder = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_RIGHT_SHOLDER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_sholder = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_LEFT_TRIGGER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_trigger = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_RIGHT_TRIGGER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_trigger = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_LEFT_JOYSTICK_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_joystick = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_RIGHT_JOYSTICK_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_joystick = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::BUTTON_SHARE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_share = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::AXIS_LX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_lx = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::AXIS_LY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_ly = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::AXIS_RX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_rx = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::AXIS_RY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_ry = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::GYRO_X_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_x = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::GYRO_Y_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_y = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::GYRO_Z_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_z = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::ACCEL_X_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_x = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::ACCEL_Y_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_y = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Info::Field::ACCEL_Z_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_z = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<Ps4Info*>::Ok(msg);
}

Result<Bytes> Ps4Cmd::cbor_serialize(const Ps4Cmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.rumble) {
            cbor_encode_int(&mapEncoder, Ps4Cmd::Field::RUMBLE_INDEX);
            cbor_encode_int(&mapEncoder, msg.rumble.value());
            }
    if (msg.led_rgb) {
            cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_RGB_INDEX);
            cbor_encode_int(&mapEncoder, msg.led_rgb.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<Ps4Cmd*> Ps4Cmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    Ps4Cmd* msg = new Ps4Cmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<Ps4Cmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Ps4Cmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Ps4Cmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<Ps4Cmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case Ps4Cmd::Field::RUMBLE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rumble = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_RGB_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_rgb = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<Ps4Cmd*>::Ok(msg);
}

Result<Bytes> CameraInfo::cbor_serialize(const CameraInfo& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.width) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::WIDTH_INDEX);
            cbor_encode_int(&mapEncoder, msg.width.value());
            }
    if (msg.height) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::HEIGHT_INDEX);
            cbor_encode_int(&mapEncoder, msg.height.value());
            }
    if (msg.format) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::FORMAT_INDEX);
            cbor_encode_text_stringz(&mapEncoder, msg.format.value().c_str());
            }
    if (msg.data) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::DATA_INDEX);
            cbor_encode_byte_string(&mapEncoder, msg.data.value().data(), msg.data.value().size());
            }
    if (msg.led) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::LED_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.led.value());
            }
    if (msg.quality) {
            cbor_encode_int(&mapEncoder, CameraInfo::Field::QUALITY_INDEX);
            cbor_encode_int(&mapEncoder, msg.quality.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<CameraInfo*> CameraInfo::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    CameraInfo* msg = new CameraInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<CameraInfo*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<CameraInfo*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<CameraInfo*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<CameraInfo*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case CameraInfo::Field::WIDTH_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->width = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraInfo::Field::HEIGHT_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->height = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraInfo::Field::FORMAT_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->format = std::string(valbuf, vallen - 1);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraInfo::Field::DATA_INDEX:{{
    uint8_t tmpbuf[512];
    size_t tmplen = sizeof(tmpbuf);
    if (cbor_value_is_byte_string(&mapIt)) {
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, NULL);
        msg->data = Bytes(tmpbuf, tmpbuf + tmplen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraInfo::Field::LED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->led = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraInfo::Field::QUALITY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->quality = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<CameraInfo*>::Ok(msg);
}

Result<Bytes> CameraCmd::cbor_serialize(const CameraCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.led) {
            cbor_encode_int(&mapEncoder, CameraCmd::Field::LED_INDEX);
            cbor_encode_boolean(&mapEncoder, msg.led.value());
            }
    if (msg.quality) {
            cbor_encode_int(&mapEncoder, CameraCmd::Field::QUALITY_INDEX);
            cbor_encode_int(&mapEncoder, msg.quality.value());
            }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

 Result<CameraCmd*> CameraCmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    CameraCmd* msg = new CameraCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<CameraCmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<CameraCmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<CameraCmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt)) {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt)) {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        } else {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<CameraCmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case CameraCmd::Field::LED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->led = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraCmd::Field::QUALITY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->quality = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            default:
                // skip unknown key
                cbor_value_advance(&mapIt);
                break;
        }

    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<CameraCmd*>::Ok(msg);
}

