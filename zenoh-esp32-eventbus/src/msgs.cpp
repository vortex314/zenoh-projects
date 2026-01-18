#include "msgs.h"


Result<Bytes> Alive::json_serialize(const Alive& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.subscribe.size()) {
                    JsonArray arr = doc["subscribe"].to<JsonArray>();
                    for (const auto& item : msg.subscribe) {
                        arr.add(item);
                    }
                }
        if (msg.publish.size()) {
                    JsonArray arr = doc["publish"].to<JsonArray>();
                    for (const auto& item : msg.publish) {
                        arr.add(item);
                    }
                }
        if (msg.services.size()) {
                    JsonArray arr = doc["services"].to<JsonArray>();
                    for (const auto& item : msg.services) {
                        arr.add(item);
                    }
                }
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<Alive*> Alive::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Alive* msg = new Alive();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<Alive*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["subscribe"].is<JsonArray>()) {
                    JsonArray arr = doc["subscribe"].as<JsonArray>();
                    msg->subscribe.clear();
                    for (JsonVariant v : arr) {
                        msg->subscribe.push_back(v.as<std::string>());
                    }
                }
        if (doc["publish"].is<JsonArray>()) {
                    JsonArray arr = doc["publish"].as<JsonArray>();
                    msg->publish.clear();
                    for (JsonVariant v : arr) {
                        msg->publish.push_back(v.as<std::string>());
                    }
                }
        if (doc["services"].is<JsonArray>()) {
                    JsonArray arr = doc["services"].as<JsonArray>();
                    msg->services.clear();
                    for (JsonVariant v : arr) {
                        msg->services.push_back(v.as<std::string>());
                    }
                }
        return Result<Alive*>::Ok(msg);
    }


Result<Bytes> UdpMessage::json_serialize(const UdpMessage& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.dst)doc["dst"] = *msg.dst;
        if (msg.src)doc["src"] = *msg.src;
        if (msg.msg_type)doc["msg_type"] = *msg.msg_type;
        if (msg.payload)
                        doc["payload"] = base64_encode(*msg.payload);
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<UdpMessage*> UdpMessage::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        UdpMessage* msg = new UdpMessage();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<UdpMessage*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["dst"].is<std::string>() )  
                        msg->dst = doc["dst"].as<std::string>();
        if (doc["src"].is<std::string>() )  
                        msg->src = doc["src"].as<std::string>();
        if (doc["msg_type"].is<std::string>() )  
                        msg->msg_type = doc["msg_type"].as<std::string>();
        if (doc["payload"].is<std::string>() )  
                        msg->payload = base64_decode(doc["payload"].as<std::string>());
        return Result<UdpMessage*>::Ok(msg);
    }


Result<Bytes> UdpMessageCbor::json_serialize(const UdpMessageCbor& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.dst)doc["dst"] = *msg.dst;
        if (msg.src)doc["src"] = *msg.src;
        if (msg.msg_type)doc["msg_type"] = *msg.msg_type;
        if (msg.payload)
                        doc["payload"] = base64_encode(*msg.payload);
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<UdpMessageCbor*> UdpMessageCbor::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        UdpMessageCbor* msg = new UdpMessageCbor();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<UdpMessageCbor*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["dst"].is<uint32_t>() )  
                        msg->dst = doc["dst"].as<uint32_t>();
        if (doc["src"].is<uint32_t>() )  
                        msg->src = doc["src"].as<uint32_t>();
        if (doc["msg_type"].is<uint32_t>() )  
                        msg->msg_type = doc["msg_type"].as<uint32_t>();
        if (doc["payload"].is<std::string>() )  
                        msg->payload = base64_decode(doc["payload"].as<std::string>());
        return Result<UdpMessageCbor*>::Ok(msg);
    }


Result<Bytes> ZenohEvent::json_serialize(const ZenohEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<ZenohEvent*> ZenohEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        ZenohEvent* msg = new ZenohEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<ZenohEvent*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<ZenohEvent*>::Ok(msg);
    }


Result<Bytes> LogEvent::json_serialize(const LogEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<LogEvent*> LogEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LogEvent* msg = new LogEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LogEvent*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<LogEvent*>::Ok(msg);
    }


Result<Bytes> SysCmd::json_serialize(const SysCmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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


Result<Bytes> SysEvent::json_serialize(const SysEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<SysEvent*> SysEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        SysEvent* msg = new SysEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<SysEvent*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<SysEvent*>::Ok(msg);
    }


Result<Bytes> WifiEvent::json_serialize(const WifiEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<WifiEvent*> WifiEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        WifiEvent* msg = new WifiEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<WifiEvent*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<WifiEvent*>::Ok(msg);
    }


Result<Bytes> MulticastEvent::json_serialize(const MulticastEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.group)doc["group"] = *msg.group;
        if (msg.port)doc["port"] = *msg.port;
        if (msg.mtu)doc["mtu"] = *msg.mtu;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<MulticastEvent*> MulticastEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        MulticastEvent* msg = new MulticastEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<MulticastEvent*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["group"].is<std::string>() )  
                        msg->group = doc["group"].as<std::string>();
        if (doc["port"].is<int32_t>() )  
                        msg->port = doc["port"].as<int32_t>();
        if (doc["mtu"].is<uint32_t>() )  
                        msg->mtu = doc["mtu"].as<uint32_t>();
        return Result<MulticastEvent*>::Ok(msg);
    }


Result<Bytes> PingReq::json_serialize(const PingReq& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.number)doc["number"] = *msg.number;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<PingReq*> PingReq::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        PingReq* msg = new PingReq();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<PingReq*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["number"].is<uint32_t>() )  
                        msg->number = doc["number"].as<uint32_t>();
        return Result<PingReq*>::Ok(msg);
    }


Result<Bytes> PingRep::json_serialize(const PingRep& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.number)doc["number"] = *msg.number;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<PingRep*> PingRep::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        PingRep* msg = new PingRep();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<PingRep*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["number"].is<uint32_t>() )  
                        msg->number = doc["number"].as<uint32_t>();
        return Result<PingRep*>::Ok(msg);
    }


Result<Bytes> HoverboardEventRaw::json_serialize(const HoverboardEventRaw& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<HoverboardEventRaw*> HoverboardEventRaw::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardEventRaw* msg = new HoverboardEventRaw();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<HoverboardEventRaw*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["ctrl_mod"].is<int32_t>() )  
                        msg->ctrl_mod = doc["ctrl_mod"].as<int32_t>();
        if (doc["ctrl_typ"].is<int32_t>() )  
                        msg->ctrl_typ = doc["ctrl_typ"].as<int32_t>();
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
        if (doc["input1_typ"].is<int32_t>() )  
                        msg->input1_typ = doc["input1_typ"].as<int32_t>();
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
        if (doc["input2_typ"].is<int32_t>() )  
                        msg->input2_typ = doc["input2_typ"].as<int32_t>();
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
        if (doc["aux_input1_typ"].is<int32_t>() )  
                        msg->aux_input1_typ = doc["aux_input1_typ"].as<int32_t>();
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
        if (doc["aux_input2_typ"].is<int32_t>() )  
                        msg->aux_input2_typ = doc["aux_input2_typ"].as<int32_t>();
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
        return Result<HoverboardEventRaw*>::Ok(msg);
    }


Result<Bytes> HoverboardEvent::json_serialize(const HoverboardEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<HoverboardEvent*> HoverboardEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardEvent* msg = new HoverboardEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<HoverboardEvent*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["ctrl_mod"].is<int32_t>() )  
                        msg->ctrl_mod = doc["ctrl_mod"].as<int32_t>();
        if (doc["ctrl_typ"].is<int32_t>() )  
                        msg->ctrl_typ = doc["ctrl_typ"].as<int32_t>();
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
        if (doc["input1_typ"].is<int32_t>() )  
                        msg->input1_typ = doc["input1_typ"].as<int32_t>();
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
        if (doc["input2_typ"].is<int32_t>() )  
                        msg->input2_typ = doc["input2_typ"].as<int32_t>();
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
        if (doc["aux_input1_typ"].is<int32_t>() )  
                        msg->aux_input1_typ = doc["aux_input1_typ"].as<int32_t>();
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
        if (doc["aux_input2_typ"].is<int32_t>() )  
                        msg->aux_input2_typ = doc["aux_input2_typ"].as<int32_t>();
        if (doc["aux_input2_min"].is<int32_t>() )  
                        msg->aux_input2_min = doc["aux_input2_min"].as<int32_t>();
        if (doc["aux_input2_mid"].is<int32_t>() )  
                        msg->aux_input2_mid = doc["aux_input2_mid"].as<int32_t>();
        if (doc["aux_input2_max"].is<int32_t>() )  
                        msg->aux_input2_max = doc["aux_input2_max"].as<int32_t>();
        if (doc["aux_input2_cmd"].is<int32_t>() )  
                        msg->aux_input2_cmd = doc["aux_input2_cmd"].as<int32_t>();
        if (doc["dc_curr"].is<float>() )  
                        msg->dc_curr = doc["dc_curr"].as<float>();
        if (doc["rdc_curr"].is<float>() )  
                        msg->rdc_curr = doc["rdc_curr"].as<float>();
        if (doc["ldc_curr"].is<float>() )  
                        msg->ldc_curr = doc["ldc_curr"].as<float>();
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
        if (doc["batv"].is<float>() )  
                        msg->batv = doc["batv"].as<float>();
        if (doc["temp"].is<float>() )  
                        msg->temp = doc["temp"].as<float>();
        return Result<HoverboardEvent*>::Ok(msg);
    }


Result<Bytes> HoverboardCmd::json_serialize(const HoverboardCmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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


Result<Bytes> HoverboardReply::json_serialize(const HoverboardReply& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.error_code)doc["error_code"] = *msg.error_code;
        if (msg.message)doc["message"] = *msg.message;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<HoverboardReply*> HoverboardReply::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        HoverboardReply* msg = new HoverboardReply();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<HoverboardReply*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["error_code"].is<int32_t>() )  
                        msg->error_code = doc["error_code"].as<int32_t>();
        if (doc["message"].is<std::string>() )  
                        msg->message = doc["message"].as<std::string>();
        return Result<HoverboardReply*>::Ok(msg);
    }


Result<Bytes> TouchPoint::json_serialize(const TouchPoint& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.active)doc["active"] = *msg.active;
        if (msg.id)doc["id"] = *msg.id;
        if (msg.x)doc["x"] = *msg.x;
        if (msg.y)doc["y"] = *msg.y;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<TouchPoint*> TouchPoint::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        TouchPoint* msg = new TouchPoint();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<TouchPoint*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["active"].is<bool>() )  
                        msg->active = doc["active"].as<bool>();
        if (doc["id"].is<int32_t>() )  
                        msg->id = doc["id"].as<int32_t>();
        if (doc["x"].is<int32_t>() )  
                        msg->x = doc["x"].as<int32_t>();
        if (doc["y"].is<int32_t>() )  
                        msg->y = doc["y"].as<int32_t>();
        return Result<TouchPoint*>::Ok(msg);
    }


Result<Bytes> Ps4Event::json_serialize(const Ps4Event& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.button_left)doc["button_left"] = *msg.button_left;
        if (msg.button_right)doc["button_right"] = *msg.button_right;
        if (msg.button_up)doc["button_up"] = *msg.button_up;
        if (msg.button_down)doc["button_down"] = *msg.button_down;
        if (msg.button_square)doc["button_square"] = *msg.button_square;
        if (msg.button_cross)doc["button_cross"] = *msg.button_cross;
        if (msg.button_circle)doc["button_circle"] = *msg.button_circle;
        if (msg.button_triangle)doc["button_triangle"] = *msg.button_triangle;
        if (msg.button_left_shoulder)doc["button_left_shoulder"] = *msg.button_left_shoulder;
        if (msg.button_right_shoulder)doc["button_right_shoulder"] = *msg.button_right_shoulder;
        if (msg.button_left_trigger)doc["button_left_trigger"] = *msg.button_left_trigger;
        if (msg.button_right_trigger)doc["button_right_trigger"] = *msg.button_right_trigger;
        if (msg.button_left_joystick)doc["button_left_joystick"] = *msg.button_left_joystick;
        if (msg.button_right_joystick)doc["button_right_joystick"] = *msg.button_right_joystick;
        if (msg.button_share)doc["button_share"] = *msg.button_share;
        if (msg.button_options)doc["button_options"] = *msg.button_options;
        if (msg.button_touchpad)doc["button_touchpad"] = *msg.button_touchpad;
        if (msg.button_ps)doc["button_ps"] = *msg.button_ps;
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
        if (msg.connected)doc["connected"] = *msg.connected;
        if (msg.battery_level)doc["battery_level"] = *msg.battery_level;
        if (msg.bluetooth)doc["bluetooth"] = *msg.bluetooth;
        if (msg.debug)doc["debug"] = *msg.debug;
        if (msg.temp)doc["temp"] = *msg.temp;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<Ps4Event*> Ps4Event::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        Ps4Event* msg = new Ps4Event();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<Ps4Event*>::Err(-1,"Cannot deserialize as object") ;
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
        if (doc["button_left_shoulder"].is<bool>() )  
                        msg->button_left_shoulder = doc["button_left_shoulder"].as<bool>();
        if (doc["button_right_shoulder"].is<bool>() )  
                        msg->button_right_shoulder = doc["button_right_shoulder"].as<bool>();
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
        if (doc["button_options"].is<bool>() )  
                        msg->button_options = doc["button_options"].as<bool>();
        if (doc["button_touchpad"].is<bool>() )  
                        msg->button_touchpad = doc["button_touchpad"].as<bool>();
        if (doc["button_ps"].is<bool>() )  
                        msg->button_ps = doc["button_ps"].as<bool>();
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
        if (doc["connected"].is<bool>() )  
                        msg->connected = doc["connected"].as<bool>();
        if (doc["battery_level"].is<int32_t>() )  
                        msg->battery_level = doc["battery_level"].as<int32_t>();
        if (doc["bluetooth"].is<bool>() )  
                        msg->bluetooth = doc["bluetooth"].as<bool>();
        if (doc["debug"].is<std::string>() )  
                        msg->debug = doc["debug"].as<std::string>();
        if (doc["temp"].is<int32_t>() )  
                        msg->temp = doc["temp"].as<int32_t>();
        return Result<Ps4Event*>::Ok(msg);
    }


Result<Bytes> Ps4Cmd::json_serialize(const Ps4Cmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.rumble_small)doc["rumble_small"] = *msg.rumble_small;
        if (msg.rumble_large)doc["rumble_large"] = *msg.rumble_large;
        if (msg.led_red)doc["led_red"] = *msg.led_red;
        if (msg.led_green)doc["led_green"] = *msg.led_green;
        if (msg.led_blue)doc["led_blue"] = *msg.led_blue;
        if (msg.led_flash_on)doc["led_flash_on"] = *msg.led_flash_on;
        if (msg.led_flash_off)doc["led_flash_off"] = *msg.led_flash_off;
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
        if (doc["rumble_small"].is<int32_t>() )  
                        msg->rumble_small = doc["rumble_small"].as<int32_t>();
        if (doc["rumble_large"].is<int32_t>() )  
                        msg->rumble_large = doc["rumble_large"].as<int32_t>();
        if (doc["led_red"].is<int32_t>() )  
                        msg->led_red = doc["led_red"].as<int32_t>();
        if (doc["led_green"].is<int32_t>() )  
                        msg->led_green = doc["led_green"].as<int32_t>();
        if (doc["led_blue"].is<int32_t>() )  
                        msg->led_blue = doc["led_blue"].as<int32_t>();
        if (doc["led_flash_on"].is<int32_t>() )  
                        msg->led_flash_on = doc["led_flash_on"].as<int32_t>();
        if (doc["led_flash_off"].is<int32_t>() )  
                        msg->led_flash_off = doc["led_flash_off"].as<int32_t>();
        return Result<Ps4Cmd*>::Ok(msg);
    }


Result<Bytes> CameraEvent::json_serialize(const CameraEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
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

    Result<CameraEvent*> CameraEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        CameraEvent* msg = new CameraEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<CameraEvent*>::Err(-1,"Cannot deserialize as object") ;
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
        return Result<CameraEvent*>::Ok(msg);
    }


Result<Bytes> CameraCmd::json_serialize(const CameraCmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.led)doc["led"] = *msg.led;
        if (msg.capture_tcp_destination)doc["capture_tcp_destination"] = *msg.capture_tcp_destination;
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
        if (doc["capture_tcp_destination"].is<std::string>() )  
                        msg->capture_tcp_destination = doc["capture_tcp_destination"].as<std::string>();
        if (doc["quality"].is<int32_t>() )  
                        msg->quality = doc["quality"].as<int32_t>();
        return Result<CameraCmd*>::Ok(msg);
    }


Result<Bytes> CameraReply::json_serialize(const CameraReply& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.error_code)doc["error_code"] = *msg.error_code;
        if (msg.message)doc["message"] = *msg.message;
        if (msg.data)
                        doc["data"] = base64_encode(*msg.data);
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<CameraReply*> CameraReply::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        CameraReply* msg = new CameraReply();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<CameraReply*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["error_code"].is<int32_t>() )  
                        msg->error_code = doc["error_code"].as<int32_t>();
        if (doc["message"].is<std::string>() )  
                        msg->message = doc["message"].as<std::string>();
        if (doc["data"].is<std::string>() )  
                        msg->data = base64_decode(doc["data"].as<std::string>());
        return Result<CameraReply*>::Ok(msg);
    }


Result<Bytes> LawnmowerManualEvent::json_serialize(const LawnmowerManualEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.speed)doc["speed"] = *msg.speed;
        if (msg.steering)doc["steering"] = *msg.steering;
        if (msg.blade)doc["blade"] = *msg.blade;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerManualEvent*> LawnmowerManualEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerManualEvent* msg = new LawnmowerManualEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerManualEvent*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["speed"].is<int32_t>() )  
                        msg->speed = doc["speed"].as<int32_t>();
        if (doc["steering"].is<int32_t>() )  
                        msg->steering = doc["steering"].as<int32_t>();
        if (doc["blade"].is<bool>() )  
                        msg->blade = doc["blade"].as<bool>();
        return Result<LawnmowerManualEvent*>::Ok(msg);
    }


Result<Bytes> LawnmowerManualCmd::json_serialize(const LawnmowerManualCmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.speed)doc["speed"] = *msg.speed;
        if (msg.steer)doc["steer"] = *msg.steer;
        if (msg.blade)doc["blade"] = *msg.blade;
        if (msg.start_manual_control)doc["start_manual_control"] = *msg.start_manual_control;
        if (msg.stop_manual_control)doc["stop_manual_control"] = *msg.stop_manual_control;
        if (msg.emergency_stop)doc["emergency_stop"] = *msg.emergency_stop;
        if (msg.start_auto_mode)doc["start_auto_mode"] = *msg.start_auto_mode;
        if (msg.stop_auto_mode)doc["stop_auto_mode"] = *msg.stop_auto_mode;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerManualCmd*> LawnmowerManualCmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerManualCmd* msg = new LawnmowerManualCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerManualCmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["speed"].is<float>() )  
                        msg->speed = doc["speed"].as<float>();
        if (doc["steer"].is<float>() )  
                        msg->steer = doc["steer"].as<float>();
        if (doc["blade"].is<bool>() )  
                        msg->blade = doc["blade"].as<bool>();
        if (doc["start_manual_control"].is<bool>() )  
                        msg->start_manual_control = doc["start_manual_control"].as<bool>();
        if (doc["stop_manual_control"].is<bool>() )  
                        msg->stop_manual_control = doc["stop_manual_control"].as<bool>();
        if (doc["emergency_stop"].is<bool>() )  
                        msg->emergency_stop = doc["emergency_stop"].as<bool>();
        if (doc["start_auto_mode"].is<bool>() )  
                        msg->start_auto_mode = doc["start_auto_mode"].as<bool>();
        if (doc["stop_auto_mode"].is<bool>() )  
                        msg->stop_auto_mode = doc["stop_auto_mode"].as<bool>();
        return Result<LawnmowerManualCmd*>::Ok(msg);
    }


Result<Bytes> LawnmowerManualReply::json_serialize(const LawnmowerManualReply& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.error_code)doc["error_code"] = *msg.error_code;
        if (msg.message)doc["message"] = *msg.message;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerManualReply*> LawnmowerManualReply::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerManualReply* msg = new LawnmowerManualReply();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerManualReply*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["error_code"].is<int32_t>() )  
                        msg->error_code = doc["error_code"].as<int32_t>();
        if (doc["message"].is<std::string>() )  
                        msg->message = doc["message"].as<std::string>();
        return Result<LawnmowerManualReply*>::Ok(msg);
    }


Result<Bytes> LawnmowerAutoEvent::json_serialize(const LawnmowerAutoEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.started)doc["started"] = *msg.started;
        if (msg.stopped)doc["stopped"] = *msg.stopped;
        if (msg.paused)doc["paused"] = *msg.paused;
        if (msg.resumed)doc["resumed"] = *msg.resumed;
        if (msg.mode)doc["mode"] = *msg.mode;
        if (msg.path)doc["path"] = *msg.path;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerAutoEvent*> LawnmowerAutoEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerAutoEvent* msg = new LawnmowerAutoEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerAutoEvent*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["started"].is<bool>() )  
                        msg->started = doc["started"].as<bool>();
        if (doc["stopped"].is<bool>() )  
                        msg->stopped = doc["stopped"].as<bool>();
        if (doc["paused"].is<bool>() )  
                        msg->paused = doc["paused"].as<bool>();
        if (doc["resumed"].is<bool>() )  
                        msg->resumed = doc["resumed"].as<bool>();
        if (doc["mode"].is<std::string>() )  
                        msg->mode = doc["mode"].as<std::string>();
        if (doc["path"].is<std::string>() )  
                        msg->path = doc["path"].as<std::string>();
        return Result<LawnmowerAutoEvent*>::Ok(msg);
    }


Result<Bytes> LawnmowerAutoCmd::json_serialize(const LawnmowerAutoCmd& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.start)doc["start"] = *msg.start;
        if (msg.stop)doc["stop"] = *msg.stop;
        if (msg.pause)doc["pause"] = *msg.pause;
        if (msg.resume)doc["resume"] = *msg.resume;
        if (msg.mode)doc["mode"] = *msg.mode;
        if (msg.path)doc["path"] = *msg.path;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerAutoCmd*> LawnmowerAutoCmd::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerAutoCmd* msg = new LawnmowerAutoCmd();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerAutoCmd*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["start"].is<bool>() )  
                        msg->start = doc["start"].as<bool>();
        if (doc["stop"].is<bool>() )  
                        msg->stop = doc["stop"].as<bool>();
        if (doc["pause"].is<bool>() )  
                        msg->pause = doc["pause"].as<bool>();
        if (doc["resume"].is<bool>() )  
                        msg->resume = doc["resume"].as<bool>();
        if (doc["mode"].is<std::string>() )  
                        msg->mode = doc["mode"].as<std::string>();
        if (doc["path"].is<std::string>() )  
                        msg->path = doc["path"].as<std::string>();
        return Result<LawnmowerAutoCmd*>::Ok(msg);
    }


Result<Bytes> LawnmowerStatus::json_serialize(const LawnmowerStatus& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.battery_level)doc["battery_level"] = *msg.battery_level;
        if (msg.blade_status)doc["blade_status"] = *msg.blade_status;
        if (msg.current_mode)doc["current_mode"] = *msg.current_mode;
        if (msg.error_message)doc["error_message"] = *msg.error_message;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<LawnmowerStatus*> LawnmowerStatus::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        LawnmowerStatus* msg = new LawnmowerStatus();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<LawnmowerStatus*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["battery_level"].is<int32_t>() )  
                        msg->battery_level = doc["battery_level"].as<int32_t>();
        if (doc["blade_status"].is<bool>() )  
                        msg->blade_status = doc["blade_status"].as<bool>();
        if (doc["current_mode"].is<std::string>() )  
                        msg->current_mode = doc["current_mode"].as<std::string>();
        if (doc["error_message"].is<std::string>() )  
                        msg->error_message = doc["error_message"].as<std::string>();
        return Result<LawnmowerStatus*>::Ok(msg);
    }


Result<Bytes> MotorEvent::json_serialize(const MotorEvent& msg)  {
        JsonDocument doc;
        doc.to<JsonObject>();
        if (msg.motor_id)doc["motor_id"] = *msg.motor_id;
        if (msg.temperature)doc["temperature"] = *msg.temperature;
        if (msg.voltage)doc["voltage"] = *msg.voltage;
        if (msg.current)doc["current"] = *msg.current;
        if (msg.speed)doc["speed"] = *msg.speed;
        if (msg.position)doc["position"] = *msg.position;
        std::string str;
        ArduinoJson::serializeJson(doc,str);
        return Result<Bytes>::Ok(Bytes(str.begin(),str.end()));
    }

    Result<MotorEvent*> MotorEvent::json_deserialize(const Bytes& bytes) {
        JsonDocument doc;
        MotorEvent* msg = new MotorEvent();
        auto err = deserializeJson(doc,bytes);
        if ( err != DeserializationError::Ok || doc.is<JsonObject>() == false ) {
            delete msg;
            return Result<MotorEvent*>::Err(-1,"Cannot deserialize as object") ;
        };        
        if (doc["motor_id"].is<int32_t>() )  
                        msg->motor_id = doc["motor_id"].as<int32_t>();
        if (doc["temperature"].is<float>() )  
                        msg->temperature = doc["temperature"].as<float>();
        if (doc["voltage"].is<float>() )  
                        msg->voltage = doc["voltage"].as<float>();
        if (doc["current"].is<float>() )  
                        msg->current = doc["current"].as<float>();
        if (doc["speed"].is<float>() )  
                        msg->speed = doc["speed"].as<float>();
        if (doc["position"].is<float>() )  
                        msg->position = doc["position"].as<float>();
        return Result<MotorEvent*>::Ok(msg);
    }



// Helper macros for serialization and deserialization




// CBOR Serialization/Deserialization
#define RC_OK(rc) if ( (rc) != CborNoError ) { return Result<Bytes>::Err(-1,"CBOR serialization error"); }


Result<Bytes> Alive::cbor_serialize(const Alive& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    {
            CborEncoder arrayEncoder;
            RC_OK(cbor_encode_int(&mapEncoder, Alive::Field::SUBSCRIBE_INDEX ));
            RC_OK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.subscribe.size()));
            for (const auto & item : msg.subscribe) {
                RC_OK(cbor_encode_text_stringz(&arrayEncoder, item.c_str()));
            }
            RC_OK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));
            }
    {
            CborEncoder arrayEncoder;
            RC_OK(cbor_encode_int(&mapEncoder, Alive::Field::PUBLISH_INDEX ));
            RC_OK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.publish.size()));
            for (const auto & item : msg.publish) {
                RC_OK(cbor_encode_text_stringz(&arrayEncoder, item.c_str()));
            }
            RC_OK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));
            }
    {
            CborEncoder arrayEncoder;
            RC_OK(cbor_encode_int(&mapEncoder, Alive::Field::SERVICES_INDEX ));
            RC_OK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.services.size()));
            for (const auto & item : msg.services) {
                RC_OK(cbor_encode_text_stringz(&arrayEncoder, item.c_str()));
            }
            RC_OK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<Alive*> Alive::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    Alive* msg = new Alive();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<Alive*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Alive*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Alive*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<Alive*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case Alive::Field::SUBSCRIBE_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {

    if (cbor_value_is_text_string(&tmp)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&tmp, &vallen);        
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&tmp);

                    msg->subscribe.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case Alive::Field::PUBLISH_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {

    if (cbor_value_is_text_string(&tmp)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&tmp, &vallen);        
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&tmp);

                    msg->publish.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case Alive::Field::SERVICES_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {

    if (cbor_value_is_text_string(&tmp)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&tmp, &vallen);        
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&tmp);

                    msg->services.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
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

    return Result<Alive*>::Ok(msg);
}

Result<Bytes> UdpMessage::cbor_serialize(const UdpMessage& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.dst) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessage::Field::DST_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.dst.value().c_str()));
            }
    if (msg.src) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessage::Field::SRC_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.src.value().c_str()));
            }
    if (msg.msg_type) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessage::Field::MSG_TYPE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.msg_type.value().c_str()));
            }
    if (msg.payload) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessage::Field::PAYLOAD_INDEX));
            RC_OK(cbor_encode_byte_string(&mapEncoder, msg.payload.value().data(), msg.payload.value().size()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<UdpMessage*> UdpMessage::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    UdpMessage* msg = new UdpMessage();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<UdpMessage*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<UdpMessage*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<UdpMessage*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<UdpMessage*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case UdpMessage::Field::DST_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->dst = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessage::Field::SRC_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->src = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessage::Field::MSG_TYPE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->msg_type = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessage::Field::PAYLOAD_INDEX:{{
    if (cbor_value_is_byte_string(&mapIt)) {
        uint8_t tmpbuf[1024];
        size_t tmplen ;
        cbor_value_calculate_string_length(&mapIt, &tmplen);
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, NULL);
        msg->payload = Bytes(tmpbuf, tmpbuf + tmplen);
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

    return Result<UdpMessage*>::Ok(msg);
}

Result<Bytes> UdpMessageCbor::cbor_serialize(const UdpMessageCbor& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.dst) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessageCbor::Field::DST_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.dst.value()));
            }
    if (msg.src) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessageCbor::Field::SRC_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.src.value()));
            }
    if (msg.msg_type) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessageCbor::Field::MSG_TYPE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.msg_type.value()));
            }
    if (msg.payload) {
            RC_OK(cbor_encode_int(&mapEncoder, UdpMessageCbor::Field::PAYLOAD_INDEX));
            RC_OK(cbor_encode_byte_string(&mapEncoder, msg.payload.value().data(), msg.payload.value().size()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<UdpMessageCbor*> UdpMessageCbor::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    UdpMessageCbor* msg = new UdpMessageCbor();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<UdpMessageCbor*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<UdpMessageCbor*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<UdpMessageCbor*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<UdpMessageCbor*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case UdpMessageCbor::Field::DST_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->dst = v;
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessageCbor::Field::SRC_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->src = v;
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessageCbor::Field::MSG_TYPE_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->msg_type = v;
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case UdpMessageCbor::Field::PAYLOAD_INDEX:{{
    if (cbor_value_is_byte_string(&mapIt)) {
        uint8_t tmpbuf[1024];
        size_t tmplen ;
        cbor_value_calculate_string_length(&mapIt, &tmplen);
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, NULL);
        msg->payload = Bytes(tmpbuf, tmpbuf + tmplen);
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

    return Result<UdpMessageCbor*>::Ok(msg);
}

Result<Bytes> ZenohEvent::cbor_serialize(const ZenohEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.zid) {
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::ZID_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.zid.value().c_str()));
            }
    if (msg.what_am_i) {
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::WHAT_AM_I_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.what_am_i.value().c_str()));
            }
    {
            CborEncoder arrayEncoder;
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::PEERS_INDEX ));
            RC_OK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.peers.size()));
            for (const auto & item : msg.peers) {
                RC_OK(cbor_encode_text_stringz(&arrayEncoder, item.c_str()));
            }
            RC_OK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));
            }
    if (msg.prefix) {
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::PREFIX_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.prefix.value().c_str()));
            }
    {
            CborEncoder arrayEncoder;
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::ROUTERS_INDEX ));
            RC_OK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.routers.size()));
            for (const auto & item : msg.routers) {
                RC_OK(cbor_encode_text_stringz(&arrayEncoder, item.c_str()));
            }
            RC_OK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));
            }
    if (msg.connect) {
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::CONNECT_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.connect.value().c_str()));
            }
    if (msg.listen) {
            RC_OK(cbor_encode_int(&mapEncoder, ZenohEvent::Field::LISTEN_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.listen.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<ZenohEvent*> ZenohEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    ZenohEvent* msg = new ZenohEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<ZenohEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<ZenohEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<ZenohEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<ZenohEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case ZenohEvent::Field::ZID_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->zid = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohEvent::Field::WHAT_AM_I_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->what_am_i = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohEvent::Field::PEERS_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {

    if (cbor_value_is_text_string(&tmp)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&tmp, &vallen);        
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&tmp);

                    msg->peers.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case ZenohEvent::Field::PREFIX_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->prefix = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohEvent::Field::ROUTERS_INDEX:{CborValue tmp;
                cbor_value_enter_container(&mapIt,&tmp);
                while (!cbor_value_at_end(&tmp)) {
                    std::string v;
                    {

    if (cbor_value_is_text_string(&tmp)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&tmp, &vallen);        
        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
        v = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&tmp);

                    msg->routers.push_back(v);
                };
                cbor_value_leave_container(&mapIt,&tmp);
                break;
            }
            
            case ZenohEvent::Field::CONNECT_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->connect = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case ZenohEvent::Field::LISTEN_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->listen = std::string(valbuf, vallen);
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

    return Result<ZenohEvent*>::Ok(msg);
}

Result<Bytes> LogEvent::cbor_serialize(const LogEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.level) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::LEVEL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.level.value()));
            }
    if (msg.message) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::MESSAGE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str()));
            }
    if (msg.error_code) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::ERROR_CODE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.error_code.value()));
            }
    if (msg.file) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::FILE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.file.value().c_str()));
            }
    if (msg.line) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::LINE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.line.value()));
            }
    if (msg.timestamp) {
            RC_OK(cbor_encode_int(&mapEncoder, LogEvent::Field::TIMESTAMP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.timestamp.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LogEvent*> LogEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LogEvent* msg = new LogEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LogEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LogEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LogEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LogEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LogEvent::Field::LEVEL_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &(v));
    msg->level = static_cast<LogLevel>(v);
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogEvent::Field::MESSAGE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->message = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogEvent::Field::ERROR_CODE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->error_code = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogEvent::Field::FILE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->file = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogEvent::Field::LINE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->line = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LogEvent::Field::TIMESTAMP_INDEX:{uint64_t v;
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

    return Result<LogEvent*>::Ok(msg);
}

Result<Bytes> SysCmd::cbor_serialize(const SysCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    // field: src
            RC_OK(cbor_encode_int(&mapEncoder, SysCmd::Field::SRC_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.src.c_str()));
    if (msg.set_time) {
            RC_OK(cbor_encode_int(&mapEncoder, SysCmd::Field::SET_TIME_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.set_time.value()));
            }
    if (msg.reboot) {
            RC_OK(cbor_encode_int(&mapEncoder, SysCmd::Field::REBOOT_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.reboot.value()));
            }
    if (msg.console) {
            RC_OK(cbor_encode_int(&mapEncoder, SysCmd::Field::CONSOLE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.console.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
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

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->src = std::string(valbuf, vallen);
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

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->console = std::string(valbuf, vallen);
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

Result<Bytes> SysEvent::cbor_serialize(const SysEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.utc) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::UTC_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.utc.value()));
            }
    if (msg.uptime) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::UPTIME_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.uptime.value()));
            }
    if (msg.free_heap) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::FREE_HEAP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.free_heap.value()));
            }
    if (msg.flash) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::FLASH_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.flash.value()));
            }
    if (msg.cpu_board) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::CPU_BOARD_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.cpu_board.value().c_str()));
            }
    if (msg.build_date) {
            RC_OK(cbor_encode_int(&mapEncoder, SysEvent::Field::BUILD_DATE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.build_date.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<SysEvent*> SysEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    SysEvent* msg = new SysEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<SysEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<SysEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<SysEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<SysEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case SysEvent::Field::UTC_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->utc = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysEvent::Field::UPTIME_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->uptime = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysEvent::Field::FREE_HEAP_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->free_heap = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysEvent::Field::FLASH_INDEX:{uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    msg->flash = v;  // Assigning the value to target
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysEvent::Field::CPU_BOARD_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->cpu_board = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case SysEvent::Field::BUILD_DATE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->build_date = std::string(valbuf, vallen);
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

    return Result<SysEvent*>::Ok(msg);
}

Result<Bytes> WifiEvent::cbor_serialize(const WifiEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.ssid) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::SSID_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.ssid.value().c_str()));
            }
    if (msg.bssid) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::BSSID_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.bssid.value().c_str()));
            }
    if (msg.rssi) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::RSSI_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rssi.value()));
            }
    if (msg.ip) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::IP_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.ip.value().c_str()));
            }
    if (msg.mac) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::MAC_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.mac.value().c_str()));
            }
    if (msg.channel) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::CHANNEL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.channel.value()));
            }
    if (msg.gateway) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::GATEWAY_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.gateway.value().c_str()));
            }
    if (msg.netmask) {
            RC_OK(cbor_encode_int(&mapEncoder, WifiEvent::Field::NETMASK_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.netmask.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<WifiEvent*> WifiEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    WifiEvent* msg = new WifiEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<WifiEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<WifiEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<WifiEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<WifiEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case WifiEvent::Field::SSID_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->ssid = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::BSSID_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->bssid = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::RSSI_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rssi = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::IP_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->ip = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::MAC_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->mac = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::CHANNEL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->channel = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::GATEWAY_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->gateway = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case WifiEvent::Field::NETMASK_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->netmask = std::string(valbuf, vallen);
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

    return Result<WifiEvent*>::Ok(msg);
}

Result<Bytes> MulticastEvent::cbor_serialize(const MulticastEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.group) {
            RC_OK(cbor_encode_int(&mapEncoder, MulticastEvent::Field::GROUP_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.group.value().c_str()));
            }
    if (msg.port) {
            RC_OK(cbor_encode_int(&mapEncoder, MulticastEvent::Field::PORT_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.port.value()));
            }
    if (msg.mtu) {
            RC_OK(cbor_encode_int(&mapEncoder, MulticastEvent::Field::MTU_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.mtu.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<MulticastEvent*> MulticastEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    MulticastEvent* msg = new MulticastEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<MulticastEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<MulticastEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<MulticastEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<MulticastEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case MulticastEvent::Field::GROUP_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->group = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MulticastEvent::Field::PORT_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->port = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MulticastEvent::Field::MTU_INDEX:{{
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

    return Result<MulticastEvent*>::Ok(msg);
}

Result<Bytes> PingReq::cbor_serialize(const PingReq& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.number) {
            RC_OK(cbor_encode_int(&mapEncoder, PingReq::Field::NUMBER_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.number.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<PingReq*> PingReq::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    PingReq* msg = new PingReq();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<PingReq*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<PingReq*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<PingReq*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<PingReq*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case PingReq::Field::NUMBER_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->number = v;
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

    return Result<PingReq*>::Ok(msg);
}

Result<Bytes> PingRep::cbor_serialize(const PingRep& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.number) {
            RC_OK(cbor_encode_int(&mapEncoder, PingRep::Field::NUMBER_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.number.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<PingRep*> PingRep::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    PingRep* msg = new PingRep();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<PingRep*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<PingRep*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<PingRep*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<PingRep*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case PingRep::Field::NUMBER_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &(v));
    msg->number = v;
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

    return Result<PingRep*>::Ok(msg);
}

Result<Bytes> HoverboardEventRaw::cbor_serialize(const HoverboardEventRaw& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.ctrl_mod) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::CTRL_MOD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.ctrl_mod.value()));
            }
    if (msg.ctrl_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::CTRL_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.ctrl_typ.value()));
            }
    if (msg.cur_mot_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::CUR_MOT_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cur_mot_max.value()));
            }
    if (msg.rpm_mot_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::RPM_MOT_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rpm_mot_max.value()));
            }
    if (msg.fi_weak_ena) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::FI_WEAK_ENA_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_ena.value()));
            }
    if (msg.fi_weak_hi) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::FI_WEAK_HI_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_hi.value()));
            }
    if (msg.fi_weak_lo) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::FI_WEAK_LO_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_lo.value()));
            }
    if (msg.fi_weak_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::FI_WEAK_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_max.value()));
            }
    if (msg.phase_adv_max_deg) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::PHASE_ADV_MAX_DEG_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.phase_adv_max_deg.value()));
            }
    if (msg.input1_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_raw.value()));
            }
    if (msg.input1_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_typ.value()));
            }
    if (msg.input1_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_min.value()));
            }
    if (msg.input1_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_mid.value()));
            }
    if (msg.input1_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_max.value()));
            }
    if (msg.input1_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT1_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_cmd.value()));
            }
    if (msg.input2_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_raw.value()));
            }
    if (msg.input2_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_typ.value()));
            }
    if (msg.input2_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_min.value()));
            }
    if (msg.input2_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_mid.value()));
            }
    if (msg.input2_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_max.value()));
            }
    if (msg.input2_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::INPUT2_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_cmd.value()));
            }
    if (msg.aux_input1_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_raw.value()));
            }
    if (msg.aux_input1_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_typ.value()));
            }
    if (msg.aux_input1_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_min.value()));
            }
    if (msg.aux_input1_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_mid.value()));
            }
    if (msg.aux_input1_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_max.value()));
            }
    if (msg.aux_input1_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT1_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_cmd.value()));
            }
    if (msg.aux_input2_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_raw.value()));
            }
    if (msg.aux_input2_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_typ.value()));
            }
    if (msg.aux_input2_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_min.value()));
            }
    if (msg.aux_input2_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_mid.value()));
            }
    if (msg.aux_input2_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_max.value()));
            }
    if (msg.aux_input2_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::AUX_INPUT2_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_cmd.value()));
            }
    if (msg.dc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::DC_CURR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.dc_curr.value()));
            }
    if (msg.rdc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::RDC_CURR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rdc_curr.value()));
            }
    if (msg.ldc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::LDC_CURR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.ldc_curr.value()));
            }
    if (msg.cmdl) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::CMDL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cmdl.value()));
            }
    if (msg.cmdr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::CMDR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cmdr.value()));
            }
    if (msg.spd_avg) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::SPD_AVG_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spd_avg.value()));
            }
    if (msg.spdl) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::SPDL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spdl.value()));
            }
    if (msg.spdr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::SPDR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spdr.value()));
            }
    if (msg.filter_rate) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::FILTER_RATE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.filter_rate.value()));
            }
    if (msg.spd_coef) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::SPD_COEF_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spd_coef.value()));
            }
    if (msg.str_coef) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::STR_COEF_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.str_coef.value()));
            }
    if (msg.batv) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::BATV_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.batv.value()));
            }
    if (msg.temp) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEventRaw::Field::TEMP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.temp.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<HoverboardEventRaw*> HoverboardEventRaw::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    HoverboardEventRaw* msg = new HoverboardEventRaw();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<HoverboardEventRaw*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardEventRaw*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardEventRaw*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<HoverboardEventRaw*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case HoverboardEventRaw::Field::CTRL_MOD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ctrl_mod = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::CTRL_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ctrl_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::CUR_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cur_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::RPM_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rpm_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::FI_WEAK_ENA_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_ena = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::FI_WEAK_HI_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_hi = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::FI_WEAK_LO_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_lo = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::FI_WEAK_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::PHASE_ADV_MAX_DEG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->phase_adv_max_deg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::AUX_INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::DC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->dc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::RDC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rdc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::LDC_CURR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ldc_curr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::CMDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::CMDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::SPD_AVG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_avg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::SPDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::SPDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::FILTER_RATE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->filter_rate = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::SPD_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::STR_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->str_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::BATV_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->batv = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEventRaw::Field::TEMP_INDEX:{int64_t v;
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

    return Result<HoverboardEventRaw*>::Ok(msg);
}

Result<Bytes> HoverboardEvent::cbor_serialize(const HoverboardEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.ctrl_mod) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::CTRL_MOD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.ctrl_mod.value()));
            }
    if (msg.ctrl_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::CTRL_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.ctrl_typ.value()));
            }
    if (msg.cur_mot_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::CUR_MOT_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cur_mot_max.value()));
            }
    if (msg.rpm_mot_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::RPM_MOT_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rpm_mot_max.value()));
            }
    if (msg.fi_weak_ena) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::FI_WEAK_ENA_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_ena.value()));
            }
    if (msg.fi_weak_hi) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::FI_WEAK_HI_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_hi.value()));
            }
    if (msg.fi_weak_lo) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::FI_WEAK_LO_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_lo.value()));
            }
    if (msg.fi_weak_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::FI_WEAK_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.fi_weak_max.value()));
            }
    if (msg.phase_adv_max_deg) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::PHASE_ADV_MAX_DEG_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.phase_adv_max_deg.value()));
            }
    if (msg.input1_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_raw.value()));
            }
    if (msg.input1_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_typ.value()));
            }
    if (msg.input1_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_min.value()));
            }
    if (msg.input1_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_mid.value()));
            }
    if (msg.input1_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_max.value()));
            }
    if (msg.input1_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT1_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input1_cmd.value()));
            }
    if (msg.input2_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_raw.value()));
            }
    if (msg.input2_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_typ.value()));
            }
    if (msg.input2_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_min.value()));
            }
    if (msg.input2_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_mid.value()));
            }
    if (msg.input2_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_max.value()));
            }
    if (msg.input2_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::INPUT2_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.input2_cmd.value()));
            }
    if (msg.aux_input1_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_raw.value()));
            }
    if (msg.aux_input1_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_typ.value()));
            }
    if (msg.aux_input1_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_min.value()));
            }
    if (msg.aux_input1_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_mid.value()));
            }
    if (msg.aux_input1_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_max.value()));
            }
    if (msg.aux_input1_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT1_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input1_cmd.value()));
            }
    if (msg.aux_input2_raw) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_RAW_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_raw.value()));
            }
    if (msg.aux_input2_typ) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_TYP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_typ.value()));
            }
    if (msg.aux_input2_min) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_MIN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_min.value()));
            }
    if (msg.aux_input2_mid) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_MID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_mid.value()));
            }
    if (msg.aux_input2_max) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_MAX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_max.value()));
            }
    if (msg.aux_input2_cmd) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::AUX_INPUT2_CMD_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.aux_input2_cmd.value()));
            }
    if (msg.dc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::DC_CURR_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.dc_curr.value()));
            }
    if (msg.rdc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::RDC_CURR_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.rdc_curr.value()));
            }
    if (msg.ldc_curr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::LDC_CURR_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.ldc_curr.value()));
            }
    if (msg.cmdl) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::CMDL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cmdl.value()));
            }
    if (msg.cmdr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::CMDR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.cmdr.value()));
            }
    if (msg.spd_avg) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::SPD_AVG_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spd_avg.value()));
            }
    if (msg.spdl) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::SPDL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spdl.value()));
            }
    if (msg.spdr) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::SPDR_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spdr.value()));
            }
    if (msg.filter_rate) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::FILTER_RATE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.filter_rate.value()));
            }
    if (msg.spd_coef) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::SPD_COEF_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.spd_coef.value()));
            }
    if (msg.str_coef) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::STR_COEF_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.str_coef.value()));
            }
    if (msg.batv) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::BATV_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.batv.value()));
            }
    if (msg.temp) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardEvent::Field::TEMP_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.temp.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<HoverboardEvent*> HoverboardEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    HoverboardEvent* msg = new HoverboardEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<HoverboardEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<HoverboardEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case HoverboardEvent::Field::CTRL_MOD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ctrl_mod = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::CTRL_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->ctrl_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::CUR_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cur_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::RPM_MOT_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rpm_mot_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::FI_WEAK_ENA_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_ena = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::FI_WEAK_HI_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_hi = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::FI_WEAK_LO_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_lo = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::FI_WEAK_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->fi_weak_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::PHASE_ADV_MAX_DEG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->phase_adv_max_deg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT1_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input1_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_RAW_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_raw = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_TYP_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_typ = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_MIN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_min = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_MID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_mid = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_MAX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_max = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::AUX_INPUT2_CMD_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->aux_input2_cmd = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::DC_CURR_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->dc_curr = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::RDC_CURR_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->rdc_curr = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::LDC_CURR_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->ldc_curr = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::CMDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::CMDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->cmdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::SPD_AVG_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_avg = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::SPDL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdl = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::SPDR_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spdr = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::FILTER_RATE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->filter_rate = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::SPD_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->spd_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::STR_COEF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->str_coef = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::BATV_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->batv = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardEvent::Field::TEMP_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->temp = f;
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

    return Result<HoverboardEvent*>::Ok(msg);
}

Result<Bytes> HoverboardCmd::cbor_serialize(const HoverboardCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.speed) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardCmd::Field::SPEED_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.speed.value()));
            }
    if (msg.steer) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardCmd::Field::STEER_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.steer.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
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

Result<Bytes> HoverboardReply::cbor_serialize(const HoverboardReply& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.error_code) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardReply::Field::ERROR_CODE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.error_code.value()));
            }
    if (msg.message) {
            RC_OK(cbor_encode_int(&mapEncoder, HoverboardReply::Field::MESSAGE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<HoverboardReply*> HoverboardReply::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    HoverboardReply* msg = new HoverboardReply();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<HoverboardReply*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardReply*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardReply*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<HoverboardReply*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case HoverboardReply::Field::ERROR_CODE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->error_code = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case HoverboardReply::Field::MESSAGE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->message = std::string(valbuf, vallen);
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

    return Result<HoverboardReply*>::Ok(msg);
}

Result<Bytes> TouchPoint::cbor_serialize(const TouchPoint& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.active) {
            RC_OK(cbor_encode_int(&mapEncoder, TouchPoint::Field::ACTIVE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.active.value()));
            }
    if (msg.id) {
            RC_OK(cbor_encode_int(&mapEncoder, TouchPoint::Field::ID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.id.value()));
            }
    if (msg.x) {
            RC_OK(cbor_encode_int(&mapEncoder, TouchPoint::Field::X_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.x.value()));
            }
    if (msg.y) {
            RC_OK(cbor_encode_int(&mapEncoder, TouchPoint::Field::Y_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.y.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<TouchPoint*> TouchPoint::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    TouchPoint* msg = new TouchPoint();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<TouchPoint*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<TouchPoint*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<TouchPoint*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<TouchPoint*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case TouchPoint::Field::ACTIVE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->active = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case TouchPoint::Field::ID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->id = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case TouchPoint::Field::X_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->x = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case TouchPoint::Field::Y_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->y = v;
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

    return Result<TouchPoint*>::Ok(msg);
}

Result<Bytes> Ps4Event::cbor_serialize(const Ps4Event& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.button_left) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_LEFT_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_left.value()));
            }
    if (msg.button_right) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_RIGHT_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_right.value()));
            }
    if (msg.button_up) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_UP_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_up.value()));
            }
    if (msg.button_down) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_DOWN_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_down.value()));
            }
    if (msg.button_square) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_SQUARE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_square.value()));
            }
    if (msg.button_cross) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_CROSS_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_cross.value()));
            }
    if (msg.button_circle) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_CIRCLE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_circle.value()));
            }
    if (msg.button_triangle) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_TRIANGLE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_triangle.value()));
            }
    if (msg.button_left_shoulder) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_LEFT_SHOULDER_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_left_shoulder.value()));
            }
    if (msg.button_right_shoulder) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_RIGHT_SHOULDER_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_right_shoulder.value()));
            }
    if (msg.button_left_trigger) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_LEFT_TRIGGER_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_left_trigger.value()));
            }
    if (msg.button_right_trigger) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_RIGHT_TRIGGER_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_right_trigger.value()));
            }
    if (msg.button_left_joystick) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_LEFT_JOYSTICK_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_left_joystick.value()));
            }
    if (msg.button_right_joystick) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_RIGHT_JOYSTICK_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_right_joystick.value()));
            }
    if (msg.button_share) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_SHARE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_share.value()));
            }
    if (msg.button_options) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_OPTIONS_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_options.value()));
            }
    if (msg.button_touchpad) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_TOUCHPAD_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_touchpad.value()));
            }
    if (msg.button_ps) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BUTTON_PS_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.button_ps.value()));
            }
    if (msg.axis_lx) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::AXIS_LX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.axis_lx.value()));
            }
    if (msg.axis_ly) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::AXIS_LY_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.axis_ly.value()));
            }
    if (msg.axis_rx) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::AXIS_RX_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.axis_rx.value()));
            }
    if (msg.axis_ry) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::AXIS_RY_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.axis_ry.value()));
            }
    if (msg.gyro_x) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::GYRO_X_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.gyro_x.value()));
            }
    if (msg.gyro_y) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::GYRO_Y_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.gyro_y.value()));
            }
    if (msg.gyro_z) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::GYRO_Z_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.gyro_z.value()));
            }
    if (msg.accel_x) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::ACCEL_X_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.accel_x.value()));
            }
    if (msg.accel_y) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::ACCEL_Y_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.accel_y.value()));
            }
    if (msg.accel_z) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::ACCEL_Z_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.accel_z.value()));
            }
    if (msg.connected) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::CONNECTED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.connected.value()));
            }
    if (msg.battery_level) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BATTERY_LEVEL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.battery_level.value()));
            }
    if (msg.bluetooth) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::BLUETOOTH_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.bluetooth.value()));
            }
    if (msg.debug) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::DEBUG_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.debug.value().c_str()));
            }
    if (msg.temp) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Event::Field::TEMP_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.temp.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<Ps4Event*> Ps4Event::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    Ps4Event* msg = new Ps4Event();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<Ps4Event*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Ps4Event*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Ps4Event*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<Ps4Event*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case Ps4Event::Field::BUTTON_LEFT_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_RIGHT_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_UP_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_up = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_DOWN_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_down = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_SQUARE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_square = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_CROSS_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_cross = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_CIRCLE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_circle = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_TRIANGLE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_triangle = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_LEFT_SHOULDER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_shoulder = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_RIGHT_SHOULDER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_shoulder = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_LEFT_TRIGGER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_trigger = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_RIGHT_TRIGGER_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_trigger = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_LEFT_JOYSTICK_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_left_joystick = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_RIGHT_JOYSTICK_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_right_joystick = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_SHARE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_share = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_OPTIONS_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_options = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_TOUCHPAD_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_touchpad = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BUTTON_PS_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->button_ps = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::AXIS_LX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_lx = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::AXIS_LY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_ly = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::AXIS_RX_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_rx = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::AXIS_RY_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->axis_ry = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::GYRO_X_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_x = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::GYRO_Y_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_y = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::GYRO_Z_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->gyro_z = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::ACCEL_X_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_x = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::ACCEL_Y_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_y = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::ACCEL_Z_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->accel_z = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::CONNECTED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->connected = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BATTERY_LEVEL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->battery_level = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::BLUETOOTH_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->bluetooth = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::DEBUG_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->debug = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Event::Field::TEMP_INDEX:{int64_t v;
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

    return Result<Ps4Event*>::Ok(msg);
}

Result<Bytes> Ps4Cmd::cbor_serialize(const Ps4Cmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.rumble_small) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::RUMBLE_SMALL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rumble_small.value()));
            }
    if (msg.rumble_large) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::RUMBLE_LARGE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.rumble_large.value()));
            }
    if (msg.led_red) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_RED_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.led_red.value()));
            }
    if (msg.led_green) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_GREEN_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.led_green.value()));
            }
    if (msg.led_blue) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_BLUE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.led_blue.value()));
            }
    if (msg.led_flash_on) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_FLASH_ON_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.led_flash_on.value()));
            }
    if (msg.led_flash_off) {
            RC_OK(cbor_encode_int(&mapEncoder, Ps4Cmd::Field::LED_FLASH_OFF_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.led_flash_off.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
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
            
            case Ps4Cmd::Field::RUMBLE_SMALL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rumble_small = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::RUMBLE_LARGE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->rumble_large = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_RED_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_red = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_GREEN_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_green = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_BLUE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_blue = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_FLASH_ON_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_flash_on = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case Ps4Cmd::Field::LED_FLASH_OFF_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->led_flash_off = v;
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

Result<Bytes> CameraEvent::cbor_serialize(const CameraEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.width) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::WIDTH_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.width.value()));
            }
    if (msg.height) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::HEIGHT_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.height.value()));
            }
    if (msg.format) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::FORMAT_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.format.value().c_str()));
            }
    if (msg.data) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::DATA_INDEX));
            RC_OK(cbor_encode_byte_string(&mapEncoder, msg.data.value().data(), msg.data.value().size()));
            }
    if (msg.led) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::LED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.led.value()));
            }
    if (msg.quality) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraEvent::Field::QUALITY_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.quality.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<CameraEvent*> CameraEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    CameraEvent* msg = new CameraEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<CameraEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<CameraEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<CameraEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<CameraEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case CameraEvent::Field::WIDTH_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->width = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraEvent::Field::HEIGHT_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->height = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraEvent::Field::FORMAT_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->format = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraEvent::Field::DATA_INDEX:{{
    if (cbor_value_is_byte_string(&mapIt)) {
        uint8_t tmpbuf[1024];
        size_t tmplen ;
        cbor_value_calculate_string_length(&mapIt, &tmplen);
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, NULL);
        msg->data = Bytes(tmpbuf, tmpbuf + tmplen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraEvent::Field::LED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->led = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraEvent::Field::QUALITY_INDEX:{int64_t v;
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

    return Result<CameraEvent*>::Ok(msg);
}

Result<Bytes> CameraCmd::cbor_serialize(const CameraCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.led) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraCmd::Field::LED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.led.value()));
            }
    if (msg.capture_tcp_destination) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraCmd::Field::CAPTURE_TCP_DESTINATION_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.capture_tcp_destination.value().c_str()));
            }
    if (msg.quality) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraCmd::Field::QUALITY_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.quality.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
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
            
            case CameraCmd::Field::CAPTURE_TCP_DESTINATION_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->capture_tcp_destination = std::string(valbuf, vallen);
    }
};
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

Result<Bytes> CameraReply::cbor_serialize(const CameraReply& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.error_code) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraReply::Field::ERROR_CODE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.error_code.value()));
            }
    if (msg.message) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraReply::Field::MESSAGE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str()));
            }
    if (msg.data) {
            RC_OK(cbor_encode_int(&mapEncoder, CameraReply::Field::DATA_INDEX));
            RC_OK(cbor_encode_byte_string(&mapEncoder, msg.data.value().data(), msg.data.value().size()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<CameraReply*> CameraReply::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    CameraReply* msg = new CameraReply();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<CameraReply*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<CameraReply*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<CameraReply*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<CameraReply*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case CameraReply::Field::ERROR_CODE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->error_code = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraReply::Field::MESSAGE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->message = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case CameraReply::Field::DATA_INDEX:{{
    if (cbor_value_is_byte_string(&mapIt)) {
        uint8_t tmpbuf[1024];
        size_t tmplen ;
        cbor_value_calculate_string_length(&mapIt, &tmplen);
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

    return Result<CameraReply*>::Ok(msg);
}

Result<Bytes> LawnmowerManualEvent::cbor_serialize(const LawnmowerManualEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.speed) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualEvent::Field::SPEED_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.speed.value()));
            }
    if (msg.steering) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualEvent::Field::STEERING_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.steering.value()));
            }
    if (msg.blade) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualEvent::Field::BLADE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.blade.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerManualEvent*> LawnmowerManualEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerManualEvent* msg = new LawnmowerManualEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerManualEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerManualEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerManualEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerManualEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerManualEvent::Field::SPEED_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->speed = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualEvent::Field::STEERING_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->steering = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualEvent::Field::BLADE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->blade = b;
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

    return Result<LawnmowerManualEvent*>::Ok(msg);
}

Result<Bytes> LawnmowerManualCmd::cbor_serialize(const LawnmowerManualCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.speed) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::SPEED_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.speed.value()));
            }
    if (msg.steer) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::STEER_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.steer.value()));
            }
    if (msg.blade) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::BLADE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.blade.value()));
            }
    if (msg.start_manual_control) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::START_MANUAL_CONTROL_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.start_manual_control.value()));
            }
    if (msg.stop_manual_control) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::STOP_MANUAL_CONTROL_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.stop_manual_control.value()));
            }
    if (msg.emergency_stop) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::EMERGENCY_STOP_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.emergency_stop.value()));
            }
    if (msg.start_auto_mode) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::START_AUTO_MODE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.start_auto_mode.value()));
            }
    if (msg.stop_auto_mode) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualCmd::Field::STOP_AUTO_MODE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.stop_auto_mode.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerManualCmd*> LawnmowerManualCmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerManualCmd* msg = new LawnmowerManualCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerManualCmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerManualCmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerManualCmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerManualCmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerManualCmd::Field::SPEED_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->speed = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::STEER_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->steer = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::BLADE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->blade = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::START_MANUAL_CONTROL_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->start_manual_control = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::STOP_MANUAL_CONTROL_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->stop_manual_control = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::EMERGENCY_STOP_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->emergency_stop = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::START_AUTO_MODE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->start_auto_mode = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualCmd::Field::STOP_AUTO_MODE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->stop_auto_mode = b;
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

    return Result<LawnmowerManualCmd*>::Ok(msg);
}

Result<Bytes> LawnmowerManualReply::cbor_serialize(const LawnmowerManualReply& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.error_code) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualReply::Field::ERROR_CODE_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.error_code.value()));
            }
    if (msg.message) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerManualReply::Field::MESSAGE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerManualReply*> LawnmowerManualReply::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerManualReply* msg = new LawnmowerManualReply();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerManualReply*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerManualReply*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerManualReply*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerManualReply*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerManualReply::Field::ERROR_CODE_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->error_code = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerManualReply::Field::MESSAGE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->message = std::string(valbuf, vallen);
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

    return Result<LawnmowerManualReply*>::Ok(msg);
}

Result<Bytes> LawnmowerAutoEvent::cbor_serialize(const LawnmowerAutoEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.started) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::STARTED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.started.value()));
            }
    if (msg.stopped) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::STOPPED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.stopped.value()));
            }
    if (msg.paused) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::PAUSED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.paused.value()));
            }
    if (msg.resumed) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::RESUMED_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.resumed.value()));
            }
    if (msg.mode) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::MODE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.mode.value().c_str()));
            }
    if (msg.path) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoEvent::Field::PATH_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.path.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerAutoEvent*> LawnmowerAutoEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerAutoEvent* msg = new LawnmowerAutoEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerAutoEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerAutoEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerAutoEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerAutoEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerAutoEvent::Field::STARTED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->started = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoEvent::Field::STOPPED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->stopped = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoEvent::Field::PAUSED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->paused = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoEvent::Field::RESUMED_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->resumed = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoEvent::Field::MODE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->mode = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoEvent::Field::PATH_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->path = std::string(valbuf, vallen);
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

    return Result<LawnmowerAutoEvent*>::Ok(msg);
}

Result<Bytes> LawnmowerAutoCmd::cbor_serialize(const LawnmowerAutoCmd& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.start) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::START_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.start.value()));
            }
    if (msg.stop) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::STOP_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.stop.value()));
            }
    if (msg.pause) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::PAUSE_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.pause.value()));
            }
    if (msg.resume) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::RESUME_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.resume.value()));
            }
    if (msg.mode) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::MODE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.mode.value().c_str()));
            }
    if (msg.path) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerAutoCmd::Field::PATH_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.path.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerAutoCmd*> LawnmowerAutoCmd::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerAutoCmd* msg = new LawnmowerAutoCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerAutoCmd*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerAutoCmd*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerAutoCmd*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerAutoCmd*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerAutoCmd::Field::START_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->start = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoCmd::Field::STOP_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->stop = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoCmd::Field::PAUSE_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->pause = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoCmd::Field::RESUME_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->resume = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoCmd::Field::MODE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->mode = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerAutoCmd::Field::PATH_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->path = std::string(valbuf, vallen);
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

    return Result<LawnmowerAutoCmd*>::Ok(msg);
}

Result<Bytes> LawnmowerStatus::cbor_serialize(const LawnmowerStatus& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.battery_level) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerStatus::Field::BATTERY_LEVEL_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.battery_level.value()));
            }
    if (msg.blade_status) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerStatus::Field::BLADE_STATUS_INDEX));
            RC_OK(cbor_encode_boolean(&mapEncoder, msg.blade_status.value()));
            }
    if (msg.current_mode) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerStatus::Field::CURRENT_MODE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.current_mode.value().c_str()));
            }
    if (msg.error_message) {
            RC_OK(cbor_encode_int(&mapEncoder, LawnmowerStatus::Field::ERROR_MESSAGE_INDEX));
            RC_OK(cbor_encode_text_stringz(&mapEncoder, msg.error_message.value().c_str()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<LawnmowerStatus*> LawnmowerStatus::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    LawnmowerStatus* msg = new LawnmowerStatus();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<LawnmowerStatus*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LawnmowerStatus*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LawnmowerStatus*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<LawnmowerStatus*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case LawnmowerStatus::Field::BATTERY_LEVEL_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->battery_level = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerStatus::Field::BLADE_STATUS_INDEX:{bool b;
    cbor_value_get_boolean(&mapIt, &b);
    msg->blade_status = b;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerStatus::Field::CURRENT_MODE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->current_mode = std::string(valbuf, vallen);
    }
};
    cbor_value_advance(&mapIt);

                break;
            }
            
            case LawnmowerStatus::Field::ERROR_MESSAGE_INDEX:{{

    if (cbor_value_is_text_string(&mapIt)) {
        char valbuf[256];
        size_t vallen ;
        cbor_value_calculate_string_length(&mapIt, &vallen);        
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
        msg->error_message = std::string(valbuf, vallen);
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

    return Result<LawnmowerStatus*>::Ok(msg);
}

Result<Bytes> MotorEvent::cbor_serialize(const MotorEvent& msg)  {
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(1024);
    CborEncoder encoder, mapEncoder;
     cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    RC_OK(cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength));

    if (msg.motor_id) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::MOTOR_ID_INDEX));
            RC_OK(cbor_encode_int(&mapEncoder, msg.motor_id.value()));
            }
    if (msg.temperature) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::TEMPERATURE_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.temperature.value()));
            }
    if (msg.voltage) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::VOLTAGE_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.voltage.value()));
            }
    if (msg.current) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::CURRENT_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.current.value()));
            }
    if (msg.speed) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::SPEED_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.speed.value()));
            }
    if (msg.position) {
            RC_OK(cbor_encode_int(&mapEncoder, MotorEvent::Field::POSITION_INDEX));
            RC_OK(cbor_encode_float(&mapEncoder, msg.position.value()));
            }
    RC_OK(cbor_encoder_close_container(&encoder, &mapEncoder));
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Result<Bytes>::Ok(Bytes(buffer.begin(), buffer.begin() + used));
}

 Result<MotorEvent*> MotorEvent::cbor_deserialize(const Bytes& bytes) {
    CborParser parser;
    CborValue it, mapIt;
    MotorEvent* msg = new MotorEvent();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError) {
        delete msg;
        return Result<MotorEvent*>::Err(-1,"CBOR parse error");
    }

    if (!cbor_value_is_map(&it)) {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<MotorEvent*>::Err(-2,"CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError) {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<MotorEvent*>::Err(-3,"CBOR deserialization error: failed to enter container");
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
            return Result<MotorEvent*>::Err(-4,"CBOR deserialization error: invalid key type");
        }
        switch (key) {
            
            case MotorEvent::Field::MOTOR_ID_INDEX:{int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    msg->motor_id = v;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MotorEvent::Field::TEMPERATURE_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->temperature = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MotorEvent::Field::VOLTAGE_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->voltage = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MotorEvent::Field::CURRENT_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->current = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MotorEvent::Field::SPEED_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->speed = f;
    cbor_value_advance(&mapIt);

                break;
            }
            
            case MotorEvent::Field::POSITION_INDEX:{float f;
    cbor_value_get_float(&mapIt, &f);
    msg->position = f;
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

    return Result<MotorEvent*>::Ok(msg);
}

