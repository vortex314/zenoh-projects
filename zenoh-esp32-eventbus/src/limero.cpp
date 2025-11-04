#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <cbor.h>
#include <msg.h>
#include <serdes.h>

    // Helper macros for serialization and deserialization
    
    


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

    typedef enum {
            FLAG_INDEX = 1,
            IDENTIFIER_INDEX = 2,
            NAME_INDEX = 3,
            VALUES_INDEX = 4,
            F_INDEX = 5,
            D_INDEX = 6,
            DATA_INDEX = 7,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (flag) {
                cbor_encode_int(&mapEncoder, Field::FLAG_INDEX);
                cbor_encode_boolean(&mapEncoder, flag.value());
                }
        if (identifier) {
                cbor_encode_int(&mapEncoder, Field::IDENTIFIER_INDEX);
                cbor_encode_int(&mapEncoder, identifier.value());
                }
        if (name) {
                cbor_encode_int(&mapEncoder, Field::NAME_INDEX);
                cbor_encode_text_stringz(&mapEncoder, name.value().c_str());
                }
        {
                CborEncoder arrayEncoder;
                cbor_encode_int(&mapEncoder, Field::VALUES_INDEX );
                cbor_encoder_create_array(&mapEncoder, &arrayEncoder, values.size());
                for (const auto & item : values) {
                    cbor_encode_float(&arrayEncoder, item);
                }
                cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
                }
        if (f) {
                cbor_encode_int(&mapEncoder, Field::F_INDEX);
                cbor_encode_float(&mapEncoder, f.value());
                }
        if (d) {
                cbor_encode_int(&mapEncoder, Field::D_INDEX);
                cbor_encode_double(&mapEncoder, d.value());
                }
        if (data) {
                cbor_encode_int(&mapEncoder, Field::DATA_INDEX);
                cbor_encode_byte_string(&mapEncoder, data.value().data(), data.value().size());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static Sample* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        Sample* msg = new Sample();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::FLAG_INDEX:{cbor_value_get_boolean(&mapIt, &(*msg->flag));
                    break;
                }
                
                case Field::IDENTIFIER_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->identifier = v;
    }
                    break;
                }
                
                case Field::NAME_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->name = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::VALUES_INDEX:{CborValue tmp;
                    cbor_value_enter_container(&mapIt,&tmp);
                    while (!cbor_value_at_end(&tmp)) {
                        float v;
                        cbor_value_get_float(&tmp, &(v));
                        msg->values.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::F_INDEX:{cbor_value_get_float(&mapIt, &(*msg->f));
                    break;
                }
                
                case Field::D_INDEX:{cbor_value_get_double(&mapIt, &(*msg->d));
                    break;
                }
                
                case Field::DATA_INDEX:{{
        uint8_t tmpbuf[512];
        size_t tmplen = sizeof(tmpbuf);
        if (cbor_value_is_byte_string(&mapIt)) {
            cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, &mapIt);
            *msg->data = Bytes(tmpbuf, tmpbuf + tmplen);
        }
    }
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

    typedef enum {
            ZID_INDEX = 2,
            WHAT_AM_I_INDEX = 3,
            PEERS_INDEX = 4,
            PREFIX_INDEX = 5,
            ROUTERS_INDEX = 6,
            CONNECT_INDEX = 7,
            LISTEN_INDEX = 8,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (zid) {
                cbor_encode_int(&mapEncoder, Field::ZID_INDEX);
                cbor_encode_text_stringz(&mapEncoder, zid.value().c_str());
                }
        if (what_am_i) {
                cbor_encode_int(&mapEncoder, Field::WHAT_AM_I_INDEX);
                cbor_encode_text_stringz(&mapEncoder, what_am_i.value().c_str());
                }
        {
                CborEncoder arrayEncoder;
                cbor_encode_int(&mapEncoder, Field::PEERS_INDEX );
                cbor_encoder_create_array(&mapEncoder, &arrayEncoder, peers.size());
                for (const auto & item : peers) {
                    cbor_encode_text_stringz(&arrayEncoder, item.c_str());
                }
                cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
                }
        if (prefix) {
                cbor_encode_int(&mapEncoder, Field::PREFIX_INDEX);
                cbor_encode_text_stringz(&mapEncoder, prefix.value().c_str());
                }
        {
                CborEncoder arrayEncoder;
                cbor_encode_int(&mapEncoder, Field::ROUTERS_INDEX );
                cbor_encoder_create_array(&mapEncoder, &arrayEncoder, routers.size());
                for (const auto & item : routers) {
                    cbor_encode_text_stringz(&arrayEncoder, item.c_str());
                }
                cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
                }
        if (connect) {
                cbor_encode_int(&mapEncoder, Field::CONNECT_INDEX);
                cbor_encode_text_stringz(&mapEncoder, connect.value().c_str());
                }
        if (listen) {
                cbor_encode_int(&mapEncoder, Field::LISTEN_INDEX);
                cbor_encode_text_stringz(&mapEncoder, listen.value().c_str());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static ZenohInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        ZenohInfo* msg = new ZenohInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::ZID_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->zid = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::WHAT_AM_I_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->what_am_i = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::PEERS_INDEX:{CborValue tmp;
                    cbor_value_enter_container(&mapIt,&tmp);
                    while (!cbor_value_at_end(&tmp)) {
                        std::string v;
                        {
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&tmp)) {
            cbor_value_copy_text_string(&tmp, valbuf, &vallen, &tmp);
            v = std::string(valbuf, vallen - 1);
        }
    }
                        msg->peers.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::PREFIX_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->prefix = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::ROUTERS_INDEX:{CborValue tmp;
                    cbor_value_enter_container(&mapIt,&tmp);
                    while (!cbor_value_at_end(&tmp)) {
                        std::string v;
                        {
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&tmp)) {
            cbor_value_copy_text_string(&tmp, valbuf, &vallen, &tmp);
            v = std::string(valbuf, vallen - 1);
        }
    }
                        msg->routers.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::CONNECT_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->connect = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::LISTEN_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->listen = std::string(valbuf, vallen - 1);
        }
    }
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

    typedef enum {
            LEVEL_INDEX = 2,
            MESSAGE_INDEX = 3,
            ERROR_CODE_INDEX = 4,
            FILE_INDEX = 5,
            LINE_INDEX = 6,
            TIMESTAMP_INDEX = 7,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (level) {
                cbor_encode_int(&mapEncoder, Field::LEVEL_INDEX);
                cbor_encode_int(&mapEncoder, level.value());
                }
        if (message) {
                cbor_encode_int(&mapEncoder, Field::MESSAGE_INDEX);
                cbor_encode_text_stringz(&mapEncoder, message.value().c_str());
                }
        if (error_code) {
                cbor_encode_int(&mapEncoder, Field::ERROR_CODE_INDEX);
                cbor_encode_int(&mapEncoder, error_code.value());
                }
        if (file) {
                cbor_encode_int(&mapEncoder, Field::FILE_INDEX);
                cbor_encode_text_stringz(&mapEncoder, file.value().c_str());
                }
        if (line) {
                cbor_encode_int(&mapEncoder, Field::LINE_INDEX);
                cbor_encode_int(&mapEncoder, line.value());
                }
        if (timestamp) {
                cbor_encode_int(&mapEncoder, Field::TIMESTAMP_INDEX);
                cbor_encode_int(&mapEncoder, timestamp.value());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static LogInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        LogInfo* msg = new LogInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::LEVEL_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->level = static_cast<LogLevel>(v);
    }
                    break;
                }
                
                case Field::MESSAGE_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->message = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::ERROR_CODE_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->error_code = v;
    }
                    break;
                }
                
                case Field::FILE_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->file = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::LINE_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->line = v;
    }
                    break;
                }
                
                case Field::TIMESTAMP_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->timestamp));
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

    typedef enum {
            SRC_INDEX = 2,
            SET_TIME_INDEX = 3,
            REBOOT_INDEX = 4,
            CONSOLE_INDEX = 5,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        // field: src
                cbor_encode_int(&mapEncoder, Field::SRC_INDEX);
                cbor_encode_text_stringz(&mapEncoder, src.c_str());
        if (set_time) {
                cbor_encode_int(&mapEncoder, Field::SET_TIME_INDEX);
                cbor_encode_int(&mapEncoder, set_time.value());
                }
        if (reboot) {
                cbor_encode_int(&mapEncoder, Field::REBOOT_INDEX);
                cbor_encode_boolean(&mapEncoder, reboot.value());
                }
        if (console) {
                cbor_encode_int(&mapEncoder, Field::CONSOLE_INDEX);
                cbor_encode_text_stringz(&mapEncoder, console.value().c_str());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static SysCmd* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        SysCmd* msg = new SysCmd();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::SRC_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            msg->src = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::SET_TIME_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->set_time));
                    break;
                }
                
                case Field::REBOOT_INDEX:{cbor_value_get_boolean(&mapIt, &(*msg->reboot));
                    break;
                }
                
                case Field::CONSOLE_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->console = std::string(valbuf, vallen - 1);
        }
    }
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

    typedef enum {
            UTC_INDEX = 1,
            UPTIME_INDEX = 2,
            FREE_HEAP_INDEX = 3,
            FLASH_INDEX = 4,
            CPU_BOARD_INDEX = 5,
            BUILD_DATE_INDEX = 6,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (utc) {
                cbor_encode_int(&mapEncoder, Field::UTC_INDEX);
                cbor_encode_int(&mapEncoder, utc.value());
                }
        if (uptime) {
                cbor_encode_int(&mapEncoder, Field::UPTIME_INDEX);
                cbor_encode_int(&mapEncoder, uptime.value());
                }
        if (free_heap) {
                cbor_encode_int(&mapEncoder, Field::FREE_HEAP_INDEX);
                cbor_encode_int(&mapEncoder, free_heap.value());
                }
        if (flash) {
                cbor_encode_int(&mapEncoder, Field::FLASH_INDEX);
                cbor_encode_int(&mapEncoder, flash.value());
                }
        if (cpu_board) {
                cbor_encode_int(&mapEncoder, Field::CPU_BOARD_INDEX);
                cbor_encode_text_stringz(&mapEncoder, cpu_board.value().c_str());
                }
        if (build_date) {
                cbor_encode_int(&mapEncoder, Field::BUILD_DATE_INDEX);
                cbor_encode_text_stringz(&mapEncoder, build_date.value().c_str());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static SysInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        SysInfo* msg = new SysInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::UTC_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->utc));
                    break;
                }
                
                case Field::UPTIME_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->uptime));
                    break;
                }
                
                case Field::FREE_HEAP_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->free_heap));
                    break;
                }
                
                case Field::FLASH_INDEX:{cbor_value_get_uint64(&mapIt, &(*msg->flash));
                    break;
                }
                
                case Field::CPU_BOARD_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->cpu_board = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::BUILD_DATE_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->build_date = std::string(valbuf, vallen - 1);
        }
    }
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


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (ssid) {
                cbor_encode_int(&mapEncoder, Field::SSID_INDEX);
                cbor_encode_text_stringz(&mapEncoder, ssid.value().c_str());
                }
        if (bssid) {
                cbor_encode_int(&mapEncoder, Field::BSSID_INDEX);
                cbor_encode_text_stringz(&mapEncoder, bssid.value().c_str());
                }
        if (rssi) {
                cbor_encode_int(&mapEncoder, Field::RSSI_INDEX);
                cbor_encode_int(&mapEncoder, rssi.value());
                }
        if (ip) {
                cbor_encode_int(&mapEncoder, Field::IP_INDEX);
                cbor_encode_text_stringz(&mapEncoder, ip.value().c_str());
                }
        if (mac) {
                cbor_encode_int(&mapEncoder, Field::MAC_INDEX);
                cbor_encode_text_stringz(&mapEncoder, mac.value().c_str());
                }
        if (channel) {
                cbor_encode_int(&mapEncoder, Field::CHANNEL_INDEX);
                cbor_encode_int(&mapEncoder, channel.value());
                }
        if (gateway) {
                cbor_encode_int(&mapEncoder, Field::GATEWAY_INDEX);
                cbor_encode_text_stringz(&mapEncoder, gateway.value().c_str());
                }
        if (netmask) {
                cbor_encode_int(&mapEncoder, Field::NETMASK_INDEX);
                cbor_encode_text_stringz(&mapEncoder, netmask.value().c_str());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static WifiInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        WifiInfo* msg = new WifiInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::SSID_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->ssid = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::BSSID_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->bssid = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::RSSI_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->rssi = v;
    }
                    break;
                }
                
                case Field::IP_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->ip = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::MAC_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->mac = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::CHANNEL_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->channel = v;
    }
                    break;
                }
                
                case Field::GATEWAY_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->gateway = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::NETMASK_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->netmask = std::string(valbuf, vallen - 1);
        }
    }
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

    typedef enum {
            GROUP_INDEX = 2,
            PORT_INDEX = 3,
            MTU_INDEX = 4,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (group) {
                cbor_encode_int(&mapEncoder, Field::GROUP_INDEX);
                cbor_encode_text_stringz(&mapEncoder, group.value().c_str());
                }
        if (port) {
                cbor_encode_int(&mapEncoder, Field::PORT_INDEX);
                cbor_encode_int(&mapEncoder, port.value());
                }
        if (mtu) {
                cbor_encode_int(&mapEncoder, Field::MTU_INDEX);
                cbor_encode_int(&mapEncoder, mtu.value());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static MulticastInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        MulticastInfo* msg = new MulticastInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::GROUP_INDEX:{{
        char valbuf[256];
        size_t vallen = sizeof(valbuf);
        if (cbor_value_is_text_string(&mapIt)) {
            cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
            *msg->group = std::string(valbuf, vallen - 1);
        }
    }
                    break;
                }
                
                case Field::PORT_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->port = v;
    }
                    break;
                }
                
                case Field::MTU_INDEX:{{
        uint64_t v;
        cbor_value_get_uint64(&mapIt, &(v));
        *msg->mtu = v;
    }
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

        return msg;
    }

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

    HoverboardInfo* deserialize(const Bytes& bytes) {
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
        return msg;
    }

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


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (ctrl_mod) {
                cbor_encode_int(&mapEncoder, Field::CTRL_MOD_INDEX);
                cbor_encode_int(&mapEncoder, ctrl_mod.value());
                }
        if (ctrl_typ) {
                cbor_encode_int(&mapEncoder, Field::CTRL_TYP_INDEX);
                cbor_encode_int(&mapEncoder, ctrl_typ.value());
                }
        if (cur_mot_max) {
                cbor_encode_int(&mapEncoder, Field::CUR_MOT_MAX_INDEX);
                cbor_encode_int(&mapEncoder, cur_mot_max.value());
                }
        if (rpm_mot_max) {
                cbor_encode_int(&mapEncoder, Field::RPM_MOT_MAX_INDEX);
                cbor_encode_int(&mapEncoder, rpm_mot_max.value());
                }
        if (fi_weak_ena) {
                cbor_encode_int(&mapEncoder, Field::FI_WEAK_ENA_INDEX);
                cbor_encode_int(&mapEncoder, fi_weak_ena.value());
                }
        if (fi_weak_hi) {
                cbor_encode_int(&mapEncoder, Field::FI_WEAK_HI_INDEX);
                cbor_encode_int(&mapEncoder, fi_weak_hi.value());
                }
        if (fi_weak_lo) {
                cbor_encode_int(&mapEncoder, Field::FI_WEAK_LO_INDEX);
                cbor_encode_int(&mapEncoder, fi_weak_lo.value());
                }
        if (fi_weak_max) {
                cbor_encode_int(&mapEncoder, Field::FI_WEAK_MAX_INDEX);
                cbor_encode_int(&mapEncoder, fi_weak_max.value());
                }
        if (phase_adv_max_deg) {
                cbor_encode_int(&mapEncoder, Field::PHASE_ADV_MAX_DEG_INDEX);
                cbor_encode_int(&mapEncoder, phase_adv_max_deg.value());
                }
        if (input1_raw) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_RAW_INDEX);
                cbor_encode_int(&mapEncoder, input1_raw.value());
                }
        if (input1_typ) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_TYP_INDEX);
                cbor_encode_int(&mapEncoder, input1_typ.value());
                }
        if (input1_min) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_MIN_INDEX);
                cbor_encode_int(&mapEncoder, input1_min.value());
                }
        if (input1_mid) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_MID_INDEX);
                cbor_encode_int(&mapEncoder, input1_mid.value());
                }
        if (input1_max) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_MAX_INDEX);
                cbor_encode_int(&mapEncoder, input1_max.value());
                }
        if (input1_cmd) {
                cbor_encode_int(&mapEncoder, Field::INPUT1_CMD_INDEX);
                cbor_encode_int(&mapEncoder, input1_cmd.value());
                }
        if (input2_raw) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_RAW_INDEX);
                cbor_encode_int(&mapEncoder, input2_raw.value());
                }
        if (input2_typ) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_TYP_INDEX);
                cbor_encode_int(&mapEncoder, input2_typ.value());
                }
        if (input2_min) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_MIN_INDEX);
                cbor_encode_int(&mapEncoder, input2_min.value());
                }
        if (input2_mid) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_MID_INDEX);
                cbor_encode_int(&mapEncoder, input2_mid.value());
                }
        if (input2_max) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_MAX_INDEX);
                cbor_encode_int(&mapEncoder, input2_max.value());
                }
        if (input2_cmd) {
                cbor_encode_int(&mapEncoder, Field::INPUT2_CMD_INDEX);
                cbor_encode_int(&mapEncoder, input2_cmd.value());
                }
        if (aux_input1_raw) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_RAW_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_raw.value());
                }
        if (aux_input1_typ) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_TYP_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_typ.value());
                }
        if (aux_input1_min) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_MIN_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_min.value());
                }
        if (aux_input1_mid) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_MID_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_mid.value());
                }
        if (aux_input1_max) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_MAX_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_max.value());
                }
        if (aux_input1_cmd) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT1_CMD_INDEX);
                cbor_encode_int(&mapEncoder, aux_input1_cmd.value());
                }
        if (aux_input2_raw) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_RAW_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_raw.value());
                }
        if (aux_input2_typ) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_TYP_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_typ.value());
                }
        if (aux_input2_min) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_MIN_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_min.value());
                }
        if (aux_input2_mid) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_MID_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_mid.value());
                }
        if (aux_input2_max) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_MAX_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_max.value());
                }
        if (aux_input2_cmd) {
                cbor_encode_int(&mapEncoder, Field::AUX_INPUT2_CMD_INDEX);
                cbor_encode_int(&mapEncoder, aux_input2_cmd.value());
                }
        if (dc_curr) {
                cbor_encode_int(&mapEncoder, Field::DC_CURR_INDEX);
                cbor_encode_int(&mapEncoder, dc_curr.value());
                }
        if (rdc_curr) {
                cbor_encode_int(&mapEncoder, Field::RDC_CURR_INDEX);
                cbor_encode_int(&mapEncoder, rdc_curr.value());
                }
        if (ldc_curr) {
                cbor_encode_int(&mapEncoder, Field::LDC_CURR_INDEX);
                cbor_encode_int(&mapEncoder, ldc_curr.value());
                }
        if (cmdl) {
                cbor_encode_int(&mapEncoder, Field::CMDL_INDEX);
                cbor_encode_int(&mapEncoder, cmdl.value());
                }
        if (cmdr) {
                cbor_encode_int(&mapEncoder, Field::CMDR_INDEX);
                cbor_encode_int(&mapEncoder, cmdr.value());
                }
        if (spd_avg) {
                cbor_encode_int(&mapEncoder, Field::SPD_AVG_INDEX);
                cbor_encode_int(&mapEncoder, spd_avg.value());
                }
        if (spdl) {
                cbor_encode_int(&mapEncoder, Field::SPDL_INDEX);
                cbor_encode_int(&mapEncoder, spdl.value());
                }
        if (spdr) {
                cbor_encode_int(&mapEncoder, Field::SPDR_INDEX);
                cbor_encode_int(&mapEncoder, spdr.value());
                }
        if (filter_rate) {
                cbor_encode_int(&mapEncoder, Field::FILTER_RATE_INDEX);
                cbor_encode_int(&mapEncoder, filter_rate.value());
                }
        if (spd_coef) {
                cbor_encode_int(&mapEncoder, Field::SPD_COEF_INDEX);
                cbor_encode_int(&mapEncoder, spd_coef.value());
                }
        if (str_coef) {
                cbor_encode_int(&mapEncoder, Field::STR_COEF_INDEX);
                cbor_encode_int(&mapEncoder, str_coef.value());
                }
        if (batv) {
                cbor_encode_int(&mapEncoder, Field::BATV_INDEX);
                cbor_encode_int(&mapEncoder, batv.value());
                }
        if (temp) {
                cbor_encode_int(&mapEncoder, Field::TEMP_INDEX);
                cbor_encode_int(&mapEncoder, temp.value());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static HoverboardInfo* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        HoverboardInfo* msg = new HoverboardInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::CTRL_MOD_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->ctrl_mod = static_cast<CtrlMod>(v);
    }
                    break;
                }
                
                case Field::CTRL_TYP_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->ctrl_typ = static_cast<CtrlTyp>(v);
    }
                    break;
                }
                
                case Field::CUR_MOT_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->cur_mot_max = v;
    }
                    break;
                }
                
                case Field::RPM_MOT_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->rpm_mot_max = v;
    }
                    break;
                }
                
                case Field::FI_WEAK_ENA_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->fi_weak_ena = v;
    }
                    break;
                }
                
                case Field::FI_WEAK_HI_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->fi_weak_hi = v;
    }
                    break;
                }
                
                case Field::FI_WEAK_LO_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->fi_weak_lo = v;
    }
                    break;
                }
                
                case Field::FI_WEAK_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->fi_weak_max = v;
    }
                    break;
                }
                
                case Field::PHASE_ADV_MAX_DEG_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->phase_adv_max_deg = v;
    }
                    break;
                }
                
                case Field::INPUT1_RAW_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_raw = v;
    }
                    break;
                }
                
                case Field::INPUT1_TYP_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_typ = static_cast<InTyp>(v);
    }
                    break;
                }
                
                case Field::INPUT1_MIN_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_min = v;
    }
                    break;
                }
                
                case Field::INPUT1_MID_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_mid = v;
    }
                    break;
                }
                
                case Field::INPUT1_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_max = v;
    }
                    break;
                }
                
                case Field::INPUT1_CMD_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input1_cmd = v;
    }
                    break;
                }
                
                case Field::INPUT2_RAW_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_raw = v;
    }
                    break;
                }
                
                case Field::INPUT2_TYP_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_typ = static_cast<InTyp>(v);
    }
                    break;
                }
                
                case Field::INPUT2_MIN_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_min = v;
    }
                    break;
                }
                
                case Field::INPUT2_MID_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_mid = v;
    }
                    break;
                }
                
                case Field::INPUT2_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_max = v;
    }
                    break;
                }
                
                case Field::INPUT2_CMD_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->input2_cmd = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT1_RAW_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_raw = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT1_TYP_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_typ = static_cast<InTyp>(v);
    }
                    break;
                }
                
                case Field::AUX_INPUT1_MIN_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_min = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT1_MID_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_mid = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT1_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_max = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT1_CMD_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input1_cmd = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT2_RAW_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_raw = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT2_TYP_INDEX:{{
        long long v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_typ = static_cast<InTyp>(v);
    }
                    break;
                }
                
                case Field::AUX_INPUT2_MIN_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_min = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT2_MID_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_mid = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT2_MAX_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_max = v;
    }
                    break;
                }
                
                case Field::AUX_INPUT2_CMD_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->aux_input2_cmd = v;
    }
                    break;
                }
                
                case Field::DC_CURR_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->dc_curr = v;
    }
                    break;
                }
                
                case Field::RDC_CURR_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->rdc_curr = v;
    }
                    break;
                }
                
                case Field::LDC_CURR_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->ldc_curr = v;
    }
                    break;
                }
                
                case Field::CMDL_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->cmdl = v;
    }
                    break;
                }
                
                case Field::CMDR_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->cmdr = v;
    }
                    break;
                }
                
                case Field::SPD_AVG_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->spd_avg = v;
    }
                    break;
                }
                
                case Field::SPDL_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->spdl = v;
    }
                    break;
                }
                
                case Field::SPDR_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->spdr = v;
    }
                    break;
                }
                
                case Field::FILTER_RATE_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->filter_rate = v;
    }
                    break;
                }
                
                case Field::SPD_COEF_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->spd_coef = v;
    }
                    break;
                }
                
                case Field::STR_COEF_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->str_coef = v;
    }
                    break;
                }
                
                case Field::BATV_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->batv = v;
    }
                    break;
                }
                
                case Field::TEMP_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->temp = v;
    }
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

    typedef enum {
            SPEED_INDEX = 0,
            STEER_INDEX = 1,
        } Field;


    Bytes cbor_serialize() const {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (speed) {
                cbor_encode_int(&mapEncoder, Field::SPEED_INDEX);
                cbor_encode_int(&mapEncoder, speed.value());
                }
        if (steer) {
                cbor_encode_int(&mapEncoder, Field::STEER_INDEX);
                cbor_encode_int(&mapEncoder, steer.value());
                }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    static HoverboardCmd* cbor_deserialize(const Bytes& bytes) {
        CborParser parser;
        CborValue it, mapIt;
        HoverboardCmd* msg = new HoverboardCmd();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError) {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it)) {
            delete msg;
            INFO("CBOR deserialization error: not a map");
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
            INFO("CBOR deserialization error: failed to enter container");
            return nullptr;
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
                return nullptr;
            }
            switch (key) {
                
                case Field::SPEED_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->speed = v;
    }
                    break;
                }
                
                case Field::STEER_INDEX:{{
        int64_t v;
        cbor_value_get_int64(&mapIt, &(v));
        *msg->steer = v;
    }
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

        return msg;
    }

};

