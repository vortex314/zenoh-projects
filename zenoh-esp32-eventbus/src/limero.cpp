#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <cbor.h>
#include <msg.h>
#include <serdes.h>

#include "limero.h"

Result<Bytes> Sample::json_serialize(const Sample &msg)
{
    JsonDocument doc;
    if (msg.flag)
        doc["flag"] = *msg.flag;
    if (msg.identifier)
        doc["identifier"] = *msg.identifier;
    if (msg.name)
        doc["name"] = *msg.name;
    if (msg.values.size())
    {
        JsonArray arr = doc["values"].to<JsonArray>();
        for (const auto &item : msg.values)
        {
            arr.add(item);
        }
    }
    if (msg.f)
        doc["f"] = *msg.f;
    if (msg.d)
        doc["d"] = *msg.d;
    if (msg.data)
        doc["data"] = base64_encode(*msg.data);
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<Sample *> Sample::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    Sample *msg = new Sample();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<Sample *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["flag"].is<bool>())
        msg->flag = doc["flag"].as<bool>();
    if (doc["identifier"].is<int32_t>())
        msg->identifier = doc["identifier"].as<int32_t>();
    if (doc["name"].is<std::string>())
        msg->name = doc["name"].as<std::string>();
    if (doc["values"].is<JsonArray>())
    {
        JsonArray arr = doc["values"].as<JsonArray>();
        msg->values.clear();
        for (JsonVariant v : arr)
        {
            msg->values.push_back(v.as<float>());
        }
    }
    if (doc["f"].is<float>())
        msg->f = doc["f"].as<float>();
    if (doc["d"].is<double>())
        msg->d = doc["d"].as<double>();
    if (doc["data"].is<std::string>())
        msg->data = base64_decode(doc["data"].as<std::string>());
    return Result<Sample *>::Ok(msg);
}

Result<Bytes> ZenohInfo::json_serialize(const ZenohInfo &msg)
{
    JsonDocument doc;
    if (msg.zid)
        doc["zid"] = *msg.zid;
    if (msg.what_am_i)
        doc["what_am_i"] = *msg.what_am_i;
    if (msg.peers.size())
    {
        JsonArray arr = doc["peers"].to<JsonArray>();
        for (const auto &item : msg.peers)
        {
            arr.add(item);
        }
    }
    if (msg.prefix)
        doc["prefix"] = *msg.prefix;
    if (msg.routers.size())
    {
        JsonArray arr = doc["routers"].to<JsonArray>();
        for (const auto &item : msg.routers)
        {
            arr.add(item);
        }
    }
    if (msg.connect)
        doc["connect"] = *msg.connect;
    if (msg.listen)
        doc["listen"] = *msg.listen;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<ZenohInfo *> ZenohInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    ZenohInfo *msg = new ZenohInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<ZenohInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["zid"].is<std::string>())
        msg->zid = doc["zid"].as<std::string>();
    if (doc["what_am_i"].is<std::string>())
        msg->what_am_i = doc["what_am_i"].as<std::string>();
    if (doc["peers"].is<JsonArray>())
    {
        JsonArray arr = doc["peers"].as<JsonArray>();
        msg->peers.clear();
        for (JsonVariant v : arr)
        {
            msg->peers.push_back(v.as<std::string>());
        }
    }
    if (doc["prefix"].is<std::string>())
        msg->prefix = doc["prefix"].as<std::string>();
    if (doc["routers"].is<JsonArray>())
    {
        JsonArray arr = doc["routers"].as<JsonArray>();
        msg->routers.clear();
        for (JsonVariant v : arr)
        {
            msg->routers.push_back(v.as<std::string>());
        }
    }
    if (doc["connect"].is<std::string>())
        msg->connect = doc["connect"].as<std::string>();
    if (doc["listen"].is<std::string>())
        msg->listen = doc["listen"].as<std::string>();
    return Result<ZenohInfo *>::Ok(msg);
}

Result<Bytes> LogInfo::json_serialize(const LogInfo &msg)
{
    JsonDocument doc;
    if (msg.level)
        doc["level"] = *msg.level;
    if (msg.message)
        doc["message"] = *msg.message;
    if (msg.error_code)
        doc["error_code"] = *msg.error_code;
    if (msg.file)
        doc["file"] = *msg.file;
    if (msg.line)
        doc["line"] = *msg.line;
    if (msg.timestamp)
        doc["timestamp"] = *msg.timestamp;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<LogInfo *> LogInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    LogInfo *msg = new LogInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<LogInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["level"].is<LogLevel>())
        msg->level = doc["level"].as<LogLevel>();
    if (doc["message"].is<std::string>())
        msg->message = doc["message"].as<std::string>();
    if (doc["error_code"].is<int32_t>())
        msg->error_code = doc["error_code"].as<int32_t>();
    if (doc["file"].is<std::string>())
        msg->file = doc["file"].as<std::string>();
    if (doc["line"].is<int32_t>())
        msg->line = doc["line"].as<int32_t>();
    if (doc["timestamp"].is<uint64_t>())
        msg->timestamp = doc["timestamp"].as<uint64_t>();
    return Result<LogInfo *>::Ok(msg);
}

Result<Bytes> SysCmd::json_serialize(const SysCmd &msg)
{
    JsonDocument doc;
    doc["src"] = msg.src;
    if (msg.set_time)
        doc["set_time"] = *msg.set_time;
    if (msg.reboot)
        doc["reboot"] = *msg.reboot;
    if (msg.console)
        doc["console"] = *msg.console;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<SysCmd *> SysCmd::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    SysCmd *msg = new SysCmd();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<SysCmd *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["src"].is<std::string>())
        msg->src = doc["src"].as<std::string>();
    if (doc["set_time"].is<uint64_t>())
        msg->set_time = doc["set_time"].as<uint64_t>();
    if (doc["reboot"].is<bool>())
        msg->reboot = doc["reboot"].as<bool>();
    if (doc["console"].is<std::string>())
        msg->console = doc["console"].as<std::string>();
    return Result<SysCmd *>::Ok(msg);
}

Result<Bytes> SysInfo::json_serialize(const SysInfo &msg)
{
    JsonDocument doc;
    if (msg.utc)
        doc["utc"] = *msg.utc;
    if (msg.uptime)
        doc["uptime"] = *msg.uptime;
    if (msg.free_heap)
        doc["free_heap"] = *msg.free_heap;
    if (msg.flash)
        doc["flash"] = *msg.flash;
    if (msg.cpu_board)
        doc["cpu_board"] = *msg.cpu_board;
    if (msg.build_date)
        doc["build_date"] = *msg.build_date;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<SysInfo *> SysInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    SysInfo *msg = new SysInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<SysInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["utc"].is<uint64_t>())
        msg->utc = doc["utc"].as<uint64_t>();
    if (doc["uptime"].is<uint64_t>())
        msg->uptime = doc["uptime"].as<uint64_t>();
    if (doc["free_heap"].is<uint64_t>())
        msg->free_heap = doc["free_heap"].as<uint64_t>();
    if (doc["flash"].is<uint64_t>())
        msg->flash = doc["flash"].as<uint64_t>();
    if (doc["cpu_board"].is<std::string>())
        msg->cpu_board = doc["cpu_board"].as<std::string>();
    if (doc["build_date"].is<std::string>())
        msg->build_date = doc["build_date"].as<std::string>();
    return Result<SysInfo *>::Ok(msg);
}

Result<Bytes> WifiInfo::json_serialize(const WifiInfo &msg)
{
    JsonDocument doc;
    if (msg.ssid)
        doc["ssid"] = *msg.ssid;
    if (msg.bssid)
        doc["bssid"] = *msg.bssid;
    if (msg.rssi)
        doc["rssi"] = *msg.rssi;
    if (msg.ip)
        doc["ip"] = *msg.ip;
    if (msg.mac)
        doc["mac"] = *msg.mac;
    if (msg.channel)
        doc["channel"] = *msg.channel;
    if (msg.gateway)
        doc["gateway"] = *msg.gateway;
    if (msg.netmask)
        doc["netmask"] = *msg.netmask;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<WifiInfo *> WifiInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    WifiInfo *msg = new WifiInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<WifiInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["ssid"].is<std::string>())
        msg->ssid = doc["ssid"].as<std::string>();
    if (doc["bssid"].is<std::string>())
        msg->bssid = doc["bssid"].as<std::string>();
    if (doc["rssi"].is<int32_t>())
        msg->rssi = doc["rssi"].as<int32_t>();
    if (doc["ip"].is<std::string>())
        msg->ip = doc["ip"].as<std::string>();
    if (doc["mac"].is<std::string>())
        msg->mac = doc["mac"].as<std::string>();
    if (doc["channel"].is<int32_t>())
        msg->channel = doc["channel"].as<int32_t>();
    if (doc["gateway"].is<std::string>())
        msg->gateway = doc["gateway"].as<std::string>();
    if (doc["netmask"].is<std::string>())
        msg->netmask = doc["netmask"].as<std::string>();
    return Result<WifiInfo *>::Ok(msg);
}

Result<Bytes> MulticastInfo::json_serialize(const MulticastInfo &msg)
{
    JsonDocument doc;
    if (msg.group)
        doc["group"] = *msg.group;
    if (msg.port)
        doc["port"] = *msg.port;
    if (msg.mtu)
        doc["mtu"] = *msg.mtu;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<MulticastInfo *> MulticastInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    MulticastInfo *msg = new MulticastInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<MulticastInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["group"].is<std::string>())
        msg->group = doc["group"].as<std::string>();
    if (doc["port"].is<int32_t>())
        msg->port = doc["port"].as<int32_t>();
    if (doc["mtu"].is<uint32_t>())
        msg->mtu = doc["mtu"].as<uint32_t>();
    return Result<MulticastInfo *>::Ok(msg);
}

Result<Bytes> HoverboardInfo::json_serialize(const HoverboardInfo &msg)
{
    JsonDocument doc;
    if (msg.ctrl_mod)
        doc["ctrl_mod"] = *msg.ctrl_mod;
    if (msg.ctrl_typ)
        doc["ctrl_typ"] = *msg.ctrl_typ;
    if (msg.cur_mot_max)
        doc["cur_mot_max"] = *msg.cur_mot_max;
    if (msg.rpm_mot_max)
        doc["rpm_mot_max"] = *msg.rpm_mot_max;
    if (msg.fi_weak_ena)
        doc["fi_weak_ena"] = *msg.fi_weak_ena;
    if (msg.fi_weak_hi)
        doc["fi_weak_hi"] = *msg.fi_weak_hi;
    if (msg.fi_weak_lo)
        doc["fi_weak_lo"] = *msg.fi_weak_lo;
    if (msg.fi_weak_max)
        doc["fi_weak_max"] = *msg.fi_weak_max;
    if (msg.phase_adv_max_deg)
        doc["phase_adv_max_deg"] = *msg.phase_adv_max_deg;
    if (msg.input1_raw)
        doc["input1_raw"] = *msg.input1_raw;
    if (msg.input1_typ)
        doc["input1_typ"] = *msg.input1_typ;
    if (msg.input1_min)
        doc["input1_min"] = *msg.input1_min;
    if (msg.input1_mid)
        doc["input1_mid"] = *msg.input1_mid;
    if (msg.input1_max)
        doc["input1_max"] = *msg.input1_max;
    if (msg.input1_cmd)
        doc["input1_cmd"] = *msg.input1_cmd;
    if (msg.input2_raw)
        doc["input2_raw"] = *msg.input2_raw;
    if (msg.input2_typ)
        doc["input2_typ"] = *msg.input2_typ;
    if (msg.input2_min)
        doc["input2_min"] = *msg.input2_min;
    if (msg.input2_mid)
        doc["input2_mid"] = *msg.input2_mid;
    if (msg.input2_max)
        doc["input2_max"] = *msg.input2_max;
    if (msg.input2_cmd)
        doc["input2_cmd"] = *msg.input2_cmd;
    if (msg.aux_input1_raw)
        doc["aux_input1_raw"] = *msg.aux_input1_raw;
    if (msg.aux_input1_typ)
        doc["aux_input1_typ"] = *msg.aux_input1_typ;
    if (msg.aux_input1_min)
        doc["aux_input1_min"] = *msg.aux_input1_min;
    if (msg.aux_input1_mid)
        doc["aux_input1_mid"] = *msg.aux_input1_mid;
    if (msg.aux_input1_max)
        doc["aux_input1_max"] = *msg.aux_input1_max;
    if (msg.aux_input1_cmd)
        doc["aux_input1_cmd"] = *msg.aux_input1_cmd;
    if (msg.aux_input2_raw)
        doc["aux_input2_raw"] = *msg.aux_input2_raw;
    if (msg.aux_input2_typ)
        doc["aux_input2_typ"] = *msg.aux_input2_typ;
    if (msg.aux_input2_min)
        doc["aux_input2_min"] = *msg.aux_input2_min;
    if (msg.aux_input2_mid)
        doc["aux_input2_mid"] = *msg.aux_input2_mid;
    if (msg.aux_input2_max)
        doc["aux_input2_max"] = *msg.aux_input2_max;
    if (msg.aux_input2_cmd)
        doc["aux_input2_cmd"] = *msg.aux_input2_cmd;
    if (msg.dc_curr)
        doc["dc_curr"] = *msg.dc_curr;
    if (msg.rdc_curr)
        doc["rdc_curr"] = *msg.rdc_curr;
    if (msg.ldc_curr)
        doc["ldc_curr"] = *msg.ldc_curr;
    if (msg.cmdl)
        doc["cmdl"] = *msg.cmdl;
    if (msg.cmdr)
        doc["cmdr"] = *msg.cmdr;
    if (msg.spd_avg)
        doc["spd_avg"] = *msg.spd_avg;
    if (msg.spdl)
        doc["spdl"] = *msg.spdl;
    if (msg.spdr)
        doc["spdr"] = *msg.spdr;
    if (msg.filter_rate)
        doc["filter_rate"] = *msg.filter_rate;
    if (msg.spd_coef)
        doc["spd_coef"] = *msg.spd_coef;
    if (msg.str_coef)
        doc["str_coef"] = *msg.str_coef;
    if (msg.batv)
        doc["batv"] = *msg.batv;
    if (msg.temp)
        doc["temp"] = *msg.temp;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<HoverboardInfo *> HoverboardInfo::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    HoverboardInfo *msg = new HoverboardInfo();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<HoverboardInfo *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["ctrl_mod"].is<CtrlMod>())
        msg->ctrl_mod = doc["ctrl_mod"].as<CtrlMod>();
    if (doc["ctrl_typ"].is<CtrlTyp>())
        msg->ctrl_typ = doc["ctrl_typ"].as<CtrlTyp>();
    if (doc["cur_mot_max"].is<int32_t>())
        msg->cur_mot_max = doc["cur_mot_max"].as<int32_t>();
    if (doc["rpm_mot_max"].is<int32_t>())
        msg->rpm_mot_max = doc["rpm_mot_max"].as<int32_t>();
    if (doc["fi_weak_ena"].is<int32_t>())
        msg->fi_weak_ena = doc["fi_weak_ena"].as<int32_t>();
    if (doc["fi_weak_hi"].is<int32_t>())
        msg->fi_weak_hi = doc["fi_weak_hi"].as<int32_t>();
    if (doc["fi_weak_lo"].is<int32_t>())
        msg->fi_weak_lo = doc["fi_weak_lo"].as<int32_t>();
    if (doc["fi_weak_max"].is<int32_t>())
        msg->fi_weak_max = doc["fi_weak_max"].as<int32_t>();
    if (doc["phase_adv_max_deg"].is<int32_t>())
        msg->phase_adv_max_deg = doc["phase_adv_max_deg"].as<int32_t>();
    if (doc["input1_raw"].is<int32_t>())
        msg->input1_raw = doc["input1_raw"].as<int32_t>();
    if (doc["input1_typ"].is<InTyp>())
        msg->input1_typ = doc["input1_typ"].as<InTyp>();
    if (doc["input1_min"].is<int32_t>())
        msg->input1_min = doc["input1_min"].as<int32_t>();
    if (doc["input1_mid"].is<int32_t>())
        msg->input1_mid = doc["input1_mid"].as<int32_t>();
    if (doc["input1_max"].is<int32_t>())
        msg->input1_max = doc["input1_max"].as<int32_t>();
    if (doc["input1_cmd"].is<int32_t>())
        msg->input1_cmd = doc["input1_cmd"].as<int32_t>();
    if (doc["input2_raw"].is<int32_t>())
        msg->input2_raw = doc["input2_raw"].as<int32_t>();
    if (doc["input2_typ"].is<InTyp>())
        msg->input2_typ = doc["input2_typ"].as<InTyp>();
    if (doc["input2_min"].is<int32_t>())
        msg->input2_min = doc["input2_min"].as<int32_t>();
    if (doc["input2_mid"].is<int32_t>())
        msg->input2_mid = doc["input2_mid"].as<int32_t>();
    if (doc["input2_max"].is<int32_t>())
        msg->input2_max = doc["input2_max"].as<int32_t>();
    if (doc["input2_cmd"].is<int32_t>())
        msg->input2_cmd = doc["input2_cmd"].as<int32_t>();
    if (doc["aux_input1_raw"].is<int32_t>())
        msg->aux_input1_raw = doc["aux_input1_raw"].as<int32_t>();
    if (doc["aux_input1_typ"].is<InTyp>())
        msg->aux_input1_typ = doc["aux_input1_typ"].as<InTyp>();
    if (doc["aux_input1_min"].is<int32_t>())
        msg->aux_input1_min = doc["aux_input1_min"].as<int32_t>();
    if (doc["aux_input1_mid"].is<int32_t>())
        msg->aux_input1_mid = doc["aux_input1_mid"].as<int32_t>();
    if (doc["aux_input1_max"].is<int32_t>())
        msg->aux_input1_max = doc["aux_input1_max"].as<int32_t>();
    if (doc["aux_input1_cmd"].is<int32_t>())
        msg->aux_input1_cmd = doc["aux_input1_cmd"].as<int32_t>();
    if (doc["aux_input2_raw"].is<int32_t>())
        msg->aux_input2_raw = doc["aux_input2_raw"].as<int32_t>();
    if (doc["aux_input2_typ"].is<InTyp>())
        msg->aux_input2_typ = doc["aux_input2_typ"].as<InTyp>();
    if (doc["aux_input2_min"].is<int32_t>())
        msg->aux_input2_min = doc["aux_input2_min"].as<int32_t>();
    if (doc["aux_input2_mid"].is<int32_t>())
        msg->aux_input2_mid = doc["aux_input2_mid"].as<int32_t>();
    if (doc["aux_input2_max"].is<int32_t>())
        msg->aux_input2_max = doc["aux_input2_max"].as<int32_t>();
    if (doc["aux_input2_cmd"].is<int32_t>())
        msg->aux_input2_cmd = doc["aux_input2_cmd"].as<int32_t>();
    if (doc["dc_curr"].is<int32_t>())
        msg->dc_curr = doc["dc_curr"].as<int32_t>();
    if (doc["rdc_curr"].is<int32_t>())
        msg->rdc_curr = doc["rdc_curr"].as<int32_t>();
    if (doc["ldc_curr"].is<int32_t>())
        msg->ldc_curr = doc["ldc_curr"].as<int32_t>();
    if (doc["cmdl"].is<int32_t>())
        msg->cmdl = doc["cmdl"].as<int32_t>();
    if (doc["cmdr"].is<int32_t>())
        msg->cmdr = doc["cmdr"].as<int32_t>();
    if (doc["spd_avg"].is<int32_t>())
        msg->spd_avg = doc["spd_avg"].as<int32_t>();
    if (doc["spdl"].is<int32_t>())
        msg->spdl = doc["spdl"].as<int32_t>();
    if (doc["spdr"].is<int32_t>())
        msg->spdr = doc["spdr"].as<int32_t>();
    if (doc["filter_rate"].is<int32_t>())
        msg->filter_rate = doc["filter_rate"].as<int32_t>();
    if (doc["spd_coef"].is<int32_t>())
        msg->spd_coef = doc["spd_coef"].as<int32_t>();
    if (doc["str_coef"].is<int32_t>())
        msg->str_coef = doc["str_coef"].as<int32_t>();
    if (doc["batv"].is<int32_t>())
        msg->batv = doc["batv"].as<int32_t>();
    if (doc["temp"].is<int32_t>())
        msg->temp = doc["temp"].as<int32_t>();
    return Result<HoverboardInfo *>::Ok(msg);
}

Result<Bytes> HoverboardCmd::json_serialize(const HoverboardCmd &msg)
{
    JsonDocument doc;
    if (msg.speed)
        doc["speed"] = *msg.speed;
    if (msg.steer)
        doc["steer"] = *msg.steer;
    std::string str;
    ArduinoJson::serializeJson(doc, str);
    return Result<Bytes>::Ok(Bytes(str.begin(), str.end()));
}

Result<HoverboardCmd *> HoverboardCmd::json_deserialize(const Bytes &bytes)
{
    JsonDocument doc;
    HoverboardCmd *msg = new HoverboardCmd();
    auto err = deserializeJson(doc, bytes);
    if (err != DeserializationError::Ok || doc.is<JsonObject>() == false)
    {
        delete msg;
        return Result<HoverboardCmd *>::Err(-1, "Cannot deserialize as object");
    };
    if (doc["speed"].is<int32_t>())
        msg->speed = doc["speed"].as<int32_t>();
    if (doc["steer"].is<int32_t>())
        msg->steer = doc["steer"].as<int32_t>();
    return Result<HoverboardCmd *>::Ok(msg);
}

// Helper macros for serialization and deserialization

// CBOR Serialization/Deserialization

Result<Bytes> Sample::cbor_serialize(const Sample &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.flag)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::FLAG_INDEX);
        cbor_encode_boolean(&mapEncoder, msg.flag.value());
    }
    if (msg.identifier)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::IDENTIFIER_INDEX);
        cbor_encode_int(&mapEncoder, msg.identifier.value());
    }
    if (msg.name)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::NAME_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.name.value().c_str());
    }
    {
        CborEncoder arrayEncoder;
        cbor_encode_int(&mapEncoder, Sample::Field::VALUES_INDEX);
        cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.values.size());
        for (const auto &item : msg.values)
        {
            cbor_encode_float(&arrayEncoder, item);
        }
        cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
    }
    if (msg.f)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::F_INDEX);
        cbor_encode_float(&mapEncoder, msg.f.value());
    }
    if (msg.d)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::D_INDEX);
        cbor_encode_double(&mapEncoder, msg.d.value());
    }
    if (msg.data)
    {
        cbor_encode_int(&mapEncoder, Sample::Field::DATA_INDEX);
        cbor_encode_byte_string(&mapEncoder, msg.data.value().data(), msg.data.value().size());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<Sample *> Sample::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    Sample *msg = new Sample();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<Sample *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<Sample *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<Sample *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<Sample *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case Sample::Field::FLAG_INDEX:
        {
            bool b;
            cbor_value_get_boolean(&mapIt, &b);
            msg->flag = b;
            cbor_value_advance(&mapIt);

            break;
        }

        case Sample::Field::IDENTIFIER_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->identifier = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case Sample::Field::NAME_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->name = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case Sample::Field::VALUES_INDEX:
        {
            CborValue tmp;
            cbor_value_enter_container(&mapIt, &tmp);
            while (!cbor_value_at_end(&tmp))
            {
                float v;
                float f;
                cbor_value_get_float(&tmp, &f);
                v = f;
                cbor_value_advance(&tmp);

                msg->values.push_back(v);
            };
            cbor_value_leave_container(&mapIt, &tmp);
            break;
        }

        case Sample::Field::F_INDEX:
        {
            float f;
            cbor_value_get_float(&mapIt, &f);
            msg->f = f;
            cbor_value_advance(&mapIt);

            break;
        }

        case Sample::Field::D_INDEX:
        {
            double d;
            cbor_value_get_double(&mapIt, &d);
            msg->d = d;
            cbor_value_advance(&mapIt);

            break;
        }

        case Sample::Field::DATA_INDEX:
        {
            {
                uint8_t tmpbuf[512];
                size_t tmplen = sizeof(tmpbuf);
                if (cbor_value_is_byte_string(&mapIt))
                {
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

    return Result<Sample *>::Ok(msg);
}

Result<Bytes> ZenohInfo::cbor_serialize(const ZenohInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.zid)
    {
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::ZID_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.zid.value().c_str());
    }
    if (msg.what_am_i)
    {
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::WHAT_AM_I_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.what_am_i.value().c_str());
    }
    {
        CborEncoder arrayEncoder;
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::PEERS_INDEX);
        cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.peers.size());
        for (const auto &item : msg.peers)
        {
            cbor_encode_text_stringz(&arrayEncoder, item.c_str());
        }
        cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
    }
    if (msg.prefix)
    {
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::PREFIX_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.prefix.value().c_str());
    }
    {
        CborEncoder arrayEncoder;
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::ROUTERS_INDEX);
        cbor_encoder_create_array(&mapEncoder, &arrayEncoder, msg.routers.size());
        for (const auto &item : msg.routers)
        {
            cbor_encode_text_stringz(&arrayEncoder, item.c_str());
        }
        cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
    }
    if (msg.connect)
    {
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::CONNECT_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.connect.value().c_str());
    }
    if (msg.listen)
    {
        cbor_encode_int(&mapEncoder, ZenohInfo::Field::LISTEN_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.listen.value().c_str());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<ZenohInfo *> ZenohInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    ZenohInfo *msg = new ZenohInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<ZenohInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<ZenohInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<ZenohInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<ZenohInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case ZenohInfo::Field::ZID_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->zid = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case ZenohInfo::Field::WHAT_AM_I_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->what_am_i = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case ZenohInfo::Field::PEERS_INDEX:
        {
            CborValue tmp;
            cbor_value_enter_container(&mapIt, &tmp);
            while (!cbor_value_at_end(&tmp))
            {
                std::string v;
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&tmp))
                    {
                        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
                        v = std::string(valbuf, vallen - 1);
                    }
                };
                cbor_value_advance(&tmp);

                msg->peers.push_back(v);
            };
            cbor_value_leave_container(&mapIt, &tmp);
            break;
        }

        case ZenohInfo::Field::PREFIX_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->prefix = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case ZenohInfo::Field::ROUTERS_INDEX:
        {
            CborValue tmp;
            cbor_value_enter_container(&mapIt, &tmp);
            while (!cbor_value_at_end(&tmp))
            {
                std::string v;
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&tmp))
                    {
                        cbor_value_copy_text_string(&tmp, valbuf, &vallen, NULL);
                        v = std::string(valbuf, vallen - 1);
                    }
                };
                cbor_value_advance(&tmp);

                msg->routers.push_back(v);
            };
            cbor_value_leave_container(&mapIt, &tmp);
            break;
        }

        case ZenohInfo::Field::CONNECT_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->connect = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case ZenohInfo::Field::LISTEN_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
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

    return Result<ZenohInfo *>::Ok(msg);
}

Result<Bytes> LogInfo::cbor_serialize(const LogInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.level)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::LEVEL_INDEX);
        cbor_encode_int(&mapEncoder, msg.level.value());
    }
    if (msg.message)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::MESSAGE_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.message.value().c_str());
    }
    if (msg.error_code)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::ERROR_CODE_INDEX);
        cbor_encode_int(&mapEncoder, msg.error_code.value());
    }
    if (msg.file)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::FILE_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.file.value().c_str());
    }
    if (msg.line)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::LINE_INDEX);
        cbor_encode_int(&mapEncoder, msg.line.value());
    }
    if (msg.timestamp)
    {
        cbor_encode_int(&mapEncoder, LogInfo::Field::TIMESTAMP_INDEX);
        cbor_encode_int(&mapEncoder, msg.timestamp.value());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<LogInfo *> LogInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    LogInfo *msg = new LogInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<LogInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<LogInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<LogInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<LogInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case LogInfo::Field::LEVEL_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->level = static_cast<LogLevel>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case LogInfo::Field::MESSAGE_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->message = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case LogInfo::Field::ERROR_CODE_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->error_code = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case LogInfo::Field::FILE_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->file = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case LogInfo::Field::LINE_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->line = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case LogInfo::Field::TIMESTAMP_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->timestamp = v; // Assigning the value to target
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

    return Result<LogInfo *>::Ok(msg);
}

Result<Bytes> SysCmd::cbor_serialize(const SysCmd &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    // field: src
    cbor_encode_int(&mapEncoder, SysCmd::Field::SRC_INDEX);
    cbor_encode_text_stringz(&mapEncoder, msg.src.c_str());
    if (msg.set_time)
    {
        cbor_encode_int(&mapEncoder, SysCmd::Field::SET_TIME_INDEX);
        cbor_encode_int(&mapEncoder, msg.set_time.value());
    }
    if (msg.reboot)
    {
        cbor_encode_int(&mapEncoder, SysCmd::Field::REBOOT_INDEX);
        cbor_encode_boolean(&mapEncoder, msg.reboot.value());
    }
    if (msg.console)
    {
        cbor_encode_int(&mapEncoder, SysCmd::Field::CONSOLE_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.console.value().c_str());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<SysCmd *> SysCmd::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    SysCmd *msg = new SysCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<SysCmd *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<SysCmd *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<SysCmd *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<SysCmd *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case SysCmd::Field::SRC_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->src = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case SysCmd::Field::SET_TIME_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->set_time = v; // Assigning the value to target
            cbor_value_advance(&mapIt);

            break;
        }

        case SysCmd::Field::REBOOT_INDEX:
        {
            bool b;
            cbor_value_get_boolean(&mapIt, &b);
            msg->reboot = b;
            cbor_value_advance(&mapIt);

            break;
        }

        case SysCmd::Field::CONSOLE_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
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

    return Result<SysCmd *>::Ok(msg);
}

Result<Bytes> SysInfo::cbor_serialize(const SysInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.utc)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::UTC_INDEX);
        cbor_encode_int(&mapEncoder, msg.utc.value());
    }
    if (msg.uptime)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::UPTIME_INDEX);
        cbor_encode_int(&mapEncoder, msg.uptime.value());
    }
    if (msg.free_heap)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::FREE_HEAP_INDEX);
        cbor_encode_int(&mapEncoder, msg.free_heap.value());
    }
    if (msg.flash)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::FLASH_INDEX);
        cbor_encode_int(&mapEncoder, msg.flash.value());
    }
    if (msg.cpu_board)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::CPU_BOARD_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.cpu_board.value().c_str());
    }
    if (msg.build_date)
    {
        cbor_encode_int(&mapEncoder, SysInfo::Field::BUILD_DATE_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.build_date.value().c_str());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<SysInfo *> SysInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    SysInfo *msg = new SysInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<SysInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<SysInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<SysInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<SysInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case SysInfo::Field::UTC_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->utc = v; // Assigning the value to target
            cbor_value_advance(&mapIt);

            break;
        }

        case SysInfo::Field::UPTIME_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->uptime = v; // Assigning the value to target
            cbor_value_advance(&mapIt);

            break;
        }

        case SysInfo::Field::FREE_HEAP_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->free_heap = v; // Assigning the value to target
            cbor_value_advance(&mapIt);

            break;
        }

        case SysInfo::Field::FLASH_INDEX:
        {
            uint64_t v;
            cbor_value_get_uint64(&mapIt, &v);
            msg->flash = v; // Assigning the value to target
            cbor_value_advance(&mapIt);

            break;
        }

        case SysInfo::Field::CPU_BOARD_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->cpu_board = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case SysInfo::Field::BUILD_DATE_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
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

    return Result<SysInfo *>::Ok(msg);
}

Result<Bytes> WifiInfo::cbor_serialize(const WifiInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.ssid)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::SSID_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.ssid.value().c_str());
    }
    if (msg.bssid)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::BSSID_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.bssid.value().c_str());
    }
    if (msg.rssi)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::RSSI_INDEX);
        cbor_encode_int(&mapEncoder, msg.rssi.value());
    }
    if (msg.ip)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::IP_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.ip.value().c_str());
    }
    if (msg.mac)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::MAC_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.mac.value().c_str());
    }
    if (msg.channel)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::CHANNEL_INDEX);
        cbor_encode_int(&mapEncoder, msg.channel.value());
    }
    if (msg.gateway)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::GATEWAY_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.gateway.value().c_str());
    }
    if (msg.netmask)
    {
        cbor_encode_int(&mapEncoder, WifiInfo::Field::NETMASK_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.netmask.value().c_str());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<WifiInfo *> WifiInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    WifiInfo *msg = new WifiInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<WifiInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<WifiInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<WifiInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<WifiInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case WifiInfo::Field::SSID_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->ssid = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::BSSID_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->bssid = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::RSSI_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->rssi = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::IP_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->ip = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::MAC_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->mac = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::CHANNEL_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->channel = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::GATEWAY_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->gateway = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case WifiInfo::Field::NETMASK_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
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

    return Result<WifiInfo *>::Ok(msg);
}

Result<Bytes> MulticastInfo::cbor_serialize(const MulticastInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.group)
    {
        cbor_encode_int(&mapEncoder, MulticastInfo::Field::GROUP_INDEX);
        cbor_encode_text_stringz(&mapEncoder, msg.group.value().c_str());
    }
    if (msg.port)
    {
        cbor_encode_int(&mapEncoder, MulticastInfo::Field::PORT_INDEX);
        cbor_encode_int(&mapEncoder, msg.port.value());
    }
    if (msg.mtu)
    {
        cbor_encode_int(&mapEncoder, MulticastInfo::Field::MTU_INDEX);
        cbor_encode_int(&mapEncoder, msg.mtu.value());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<MulticastInfo *> MulticastInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    MulticastInfo *msg = new MulticastInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<MulticastInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<MulticastInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<MulticastInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<MulticastInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case MulticastInfo::Field::GROUP_INDEX:
        {
            {
                char valbuf[256];
                size_t vallen = sizeof(valbuf);
                if (cbor_value_is_text_string(&mapIt))
                {
                    cbor_value_copy_text_string(&mapIt, valbuf, &vallen, NULL);
                    msg->group = std::string(valbuf, vallen - 1);
                }
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case MulticastInfo::Field::PORT_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->port = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case MulticastInfo::Field::MTU_INDEX:
        {
            {
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

    return Result<MulticastInfo *>::Ok(msg);
}

Result<Bytes> HoverboardInfo::cbor_serialize(const HoverboardInfo &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.ctrl_mod)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CTRL_MOD_INDEX);
        cbor_encode_int(&mapEncoder, msg.ctrl_mod.value());
    }
    if (msg.ctrl_typ)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CTRL_TYP_INDEX);
        cbor_encode_int(&mapEncoder, msg.ctrl_typ.value());
    }
    if (msg.cur_mot_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CUR_MOT_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.cur_mot_max.value());
    }
    if (msg.rpm_mot_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::RPM_MOT_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.rpm_mot_max.value());
    }
    if (msg.fi_weak_ena)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_ENA_INDEX);
        cbor_encode_int(&mapEncoder, msg.fi_weak_ena.value());
    }
    if (msg.fi_weak_hi)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_HI_INDEX);
        cbor_encode_int(&mapEncoder, msg.fi_weak_hi.value());
    }
    if (msg.fi_weak_lo)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_LO_INDEX);
        cbor_encode_int(&mapEncoder, msg.fi_weak_lo.value());
    }
    if (msg.fi_weak_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FI_WEAK_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.fi_weak_max.value());
    }
    if (msg.phase_adv_max_deg)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::PHASE_ADV_MAX_DEG_INDEX);
        cbor_encode_int(&mapEncoder, msg.phase_adv_max_deg.value());
    }
    if (msg.input1_raw)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_RAW_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_raw.value());
    }
    if (msg.input1_typ)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_TYP_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_typ.value());
    }
    if (msg.input1_min)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MIN_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_min.value());
    }
    if (msg.input1_mid)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MID_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_mid.value());
    }
    if (msg.input1_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_max.value());
    }
    if (msg.input1_cmd)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT1_CMD_INDEX);
        cbor_encode_int(&mapEncoder, msg.input1_cmd.value());
    }
    if (msg.input2_raw)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_RAW_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_raw.value());
    }
    if (msg.input2_typ)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_TYP_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_typ.value());
    }
    if (msg.input2_min)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MIN_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_min.value());
    }
    if (msg.input2_mid)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MID_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_mid.value());
    }
    if (msg.input2_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_max.value());
    }
    if (msg.input2_cmd)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::INPUT2_CMD_INDEX);
        cbor_encode_int(&mapEncoder, msg.input2_cmd.value());
    }
    if (msg.aux_input1_raw)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_RAW_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_raw.value());
    }
    if (msg.aux_input1_typ)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_TYP_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_typ.value());
    }
    if (msg.aux_input1_min)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MIN_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_min.value());
    }
    if (msg.aux_input1_mid)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MID_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_mid.value());
    }
    if (msg.aux_input1_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_max.value());
    }
    if (msg.aux_input1_cmd)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT1_CMD_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input1_cmd.value());
    }
    if (msg.aux_input2_raw)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_RAW_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_raw.value());
    }
    if (msg.aux_input2_typ)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_TYP_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_typ.value());
    }
    if (msg.aux_input2_min)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MIN_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_min.value());
    }
    if (msg.aux_input2_mid)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MID_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_mid.value());
    }
    if (msg.aux_input2_max)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_MAX_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_max.value());
    }
    if (msg.aux_input2_cmd)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::AUX_INPUT2_CMD_INDEX);
        cbor_encode_int(&mapEncoder, msg.aux_input2_cmd.value());
    }
    if (msg.dc_curr)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::DC_CURR_INDEX);
        cbor_encode_int(&mapEncoder, msg.dc_curr.value());
    }
    if (msg.rdc_curr)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::RDC_CURR_INDEX);
        cbor_encode_int(&mapEncoder, msg.rdc_curr.value());
    }
    if (msg.ldc_curr)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::LDC_CURR_INDEX);
        cbor_encode_int(&mapEncoder, msg.ldc_curr.value());
    }
    if (msg.cmdl)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CMDL_INDEX);
        cbor_encode_int(&mapEncoder, msg.cmdl.value());
    }
    if (msg.cmdr)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::CMDR_INDEX);
        cbor_encode_int(&mapEncoder, msg.cmdr.value());
    }
    if (msg.spd_avg)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPD_AVG_INDEX);
        cbor_encode_int(&mapEncoder, msg.spd_avg.value());
    }
    if (msg.spdl)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPDL_INDEX);
        cbor_encode_int(&mapEncoder, msg.spdl.value());
    }
    if (msg.spdr)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPDR_INDEX);
        cbor_encode_int(&mapEncoder, msg.spdr.value());
    }
    if (msg.filter_rate)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::FILTER_RATE_INDEX);
        cbor_encode_int(&mapEncoder, msg.filter_rate.value());
    }
    if (msg.spd_coef)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::SPD_COEF_INDEX);
        cbor_encode_int(&mapEncoder, msg.spd_coef.value());
    }
    if (msg.str_coef)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::STR_COEF_INDEX);
        cbor_encode_int(&mapEncoder, msg.str_coef.value());
    }
    if (msg.batv)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::BATV_INDEX);
        cbor_encode_int(&mapEncoder, msg.batv.value());
    }
    if (msg.temp)
    {
        cbor_encode_int(&mapEncoder, HoverboardInfo::Field::TEMP_INDEX);
        cbor_encode_int(&mapEncoder, msg.temp.value());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<HoverboardInfo *> HoverboardInfo::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    HoverboardInfo *msg = new HoverboardInfo();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<HoverboardInfo *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardInfo *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardInfo *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
            cbor_value_advance(&mapIt);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type %d ",cbor_value_get_type(&mapIt));
            delete msg;
            return Result<HoverboardInfo *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case HoverboardInfo::Field::CTRL_MOD_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->ctrl_mod = static_cast<CtrlMod>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::CTRL_TYP_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->ctrl_typ = static_cast<CtrlTyp>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::CUR_MOT_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->cur_mot_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::RPM_MOT_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->rpm_mot_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::FI_WEAK_ENA_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->fi_weak_ena = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::FI_WEAK_HI_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->fi_weak_hi = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::FI_WEAK_LO_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->fi_weak_lo = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::FI_WEAK_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->fi_weak_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::PHASE_ADV_MAX_DEG_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->phase_adv_max_deg = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_RAW_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input1_raw = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_TYP_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->input1_typ = static_cast<InTyp>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_MIN_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input1_min = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_MID_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input1_mid = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input1_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT1_CMD_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input1_cmd = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_RAW_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input2_raw = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_TYP_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->input2_typ = static_cast<InTyp>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_MIN_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input2_min = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_MID_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input2_mid = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input2_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::INPUT2_CMD_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->input2_cmd = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_RAW_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input1_raw = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_TYP_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->aux_input1_typ = static_cast<InTyp>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_MIN_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input1_min = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_MID_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input1_mid = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input1_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT1_CMD_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input1_cmd = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_RAW_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input2_raw = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_TYP_INDEX:
        {
            {
                long long v;
                cbor_value_get_int64(&mapIt, &(v));
                msg->aux_input2_typ = static_cast<InTyp>(v);
            };
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_MIN_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input2_min = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_MID_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input2_mid = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_MAX_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input2_max = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::AUX_INPUT2_CMD_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->aux_input2_cmd = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::DC_CURR_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->dc_curr = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::RDC_CURR_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->rdc_curr = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::LDC_CURR_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->ldc_curr = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::CMDL_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->cmdl = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::CMDR_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->cmdr = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::SPD_AVG_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->spd_avg = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::SPDL_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->spdl = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::SPDR_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->spdr = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::FILTER_RATE_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->filter_rate = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::SPD_COEF_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->spd_coef = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::STR_COEF_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->str_coef = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::BATV_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->batv = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardInfo::Field::TEMP_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->temp = v;
            cbor_value_advance(&mapIt);

            break;
        }

        default:
            // skip unknown key
            INFO("CBOR deserialization: skipping unknown key %llu", key);
            cbor_value_advance(&mapIt);
            break;
        }
    }

    // leave container
    cbor_value_leave_container(&it, &mapIt);

    return Result<HoverboardInfo *>::Ok(msg);
}

Result<Bytes> HoverboardCmd::cbor_serialize(const HoverboardCmd &msg)
{
    // buffer: grow if needed by changing initial size
    std::vector<uint8_t> buffer(512);
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

    // Start top-level map
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

    if (msg.speed)
    {
        cbor_encode_int(&mapEncoder, HoverboardCmd::Field::SPEED_INDEX);
        cbor_encode_int(&mapEncoder, msg.speed.value());
    }
    if (msg.steer)
    {
        cbor_encode_int(&mapEncoder, HoverboardCmd::Field::STEER_INDEX);
        cbor_encode_int(&mapEncoder, msg.steer.value());
    }
    cbor_encoder_close_container(&encoder, &mapEncoder);
    // get used size
    size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
    return Bytes(buffer.begin(), buffer.begin() + used);
}

Result<HoverboardCmd *> HoverboardCmd::cbor_deserialize(const Bytes &bytes)
{
    CborParser parser;
    CborValue it, mapIt;
    HoverboardCmd *msg = new HoverboardCmd();

    CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
    if (err != CborNoError)
    {
        delete msg;
        return Result<HoverboardCmd *>::Err(-1, "CBOR parse error");
    }

    if (!cbor_value_is_map(&it))
    {
        delete msg;
        INFO("CBOR deserialization error: not a map");
        return Result<HoverboardCmd *>::Err(-2, "CBOR deserialization error: not a map");
    }

    // enter map
    err = cbor_value_enter_container(&it, &mapIt);
    if (err != CborNoError)
    {
        delete msg;
        INFO("CBOR deserialization error: failed to enter container");
        return Result<HoverboardCmd *>::Err(-3, "CBOR deserialization error: failed to enter container");
    }

    // iterate key/value pairs
    while (!cbor_value_at_end(&mapIt))
    {
        uint64_t key = 0;
        if (cbor_value_is_unsigned_integer(&mapIt))
        {
            cbor_value_get_uint64(&mapIt, &key);
        }
        else
        {
            // invalid key type
            INFO("CBOR deserialization error: invalid key type");
            delete msg;
            return Result<HoverboardCmd *>::Err(-4, "CBOR deserialization error: invalid key type");
        }
        switch (key)
        {

        case HoverboardCmd::Field::SPEED_INDEX:
        {
            int64_t v;
            cbor_value_get_int64(&mapIt, &v);
            msg->speed = v;
            cbor_value_advance(&mapIt);

            break;
        }

        case HoverboardCmd::Field::STEER_INDEX:
        {
            int64_t v;
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

    return Result<HoverboardCmd *>::Ok(msg);
}
