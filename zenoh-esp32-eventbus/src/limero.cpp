#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <cbor.h>
#include <functional>
#include <actor.h>

typedef std::vector<uint8_t> Bytes;


// Helper macros for serialization and deserialization





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



static constexpr const char SAMPLE_NAME[] = "Sample";

class Sample : public Msg<SAMPLE_NAME,3386> {
    public:

    std::optional<bool> flag;
    std::optional<int32_t> identifier;
    std::optional<std::string> name;
    std::vector<float> values;
    std::optional<float> f;
    std::optional<double> d;
    std::optional<Bytes> data;
    

    typedef enum {
        FLAG_INDEX = 1,
        IDENTIFIER_INDEX = 2,
        NAME_INDEX = 3,
        VALUES_INDEX = 4,
        F_INDEX = 5,
        D_INDEX = 6,
        DATA_INDEX = 7,
    } Field;

    Bytes serialize() const {
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

    Sample* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::FLAG_INDEX:{cbor_value_get_boolean(&mapIt, &*flag);
                    break;
                }
                
                case Field::IDENTIFIER_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *identifier = v;
}
                    break;
                }
                
                case Field::NAME_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *name = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::VALUES_INDEX:{CborValue tmp;
                    cbor_value_enter_container(&mapIt,&tmp);
                    while (!cbor_value_at_end(&tmp)) {
                        float v;
                        cbor_value_get_float(&tmp, &v);
                        values.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::F_INDEX:{cbor_value_get_float(&mapIt, &*f);
                    break;
                }
                
                case Field::D_INDEX:{cbor_value_get_double(&mapIt, &*d);
                    break;
                }
                
                case Field::DATA_INDEX:{{
    uint8_t tmpbuf[512];
    size_t tmplen = sizeof(tmpbuf);
    if (cbor_value_is_byte_string(&mapIt)) {
        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, &mapIt);
        *data = Bytes(tmpbuf, tmpbuf + tmplen);
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

static constexpr const char ZENOHINFO_NAME[] = "ZenohInfo";

class ZenohInfo : public Msg<ZENOHINFO_NAME,33380> {
    public:

    std::optional<std::string> zid;
    std::optional<std::string> what_am_i;
    std::vector<std::string> peers;
    std::optional<std::string> prefix;
    std::vector<std::string> routers;
    std::optional<std::string> connect;
    std::optional<std::string> listen;
    

    typedef enum {
        ZID_INDEX = 2,
        WHAT_AM_I_INDEX = 3,
        PEERS_INDEX = 4,
        PREFIX_INDEX = 5,
        ROUTERS_INDEX = 6,
        CONNECT_INDEX = 7,
        LISTEN_INDEX = 8,
    } Field;

    Bytes serialize() const {
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

    ZenohInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::ZID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *zid = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::WHAT_AM_I_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *what_am_i = std::string(valbuf, vallen - 1);
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
                        peers.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::PREFIX_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *prefix = std::string(valbuf, vallen - 1);
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
                        routers.push_back(v);
                    }
                    cbor_value_leave_container(&mapIt,&tmp);
                    break;
                }
                
                case Field::CONNECT_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *connect = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::LISTEN_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *listen = std::string(valbuf, vallen - 1);
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

static constexpr const char LOGINFO_NAME[] = "LogInfo";

class LogInfo : public Msg<LOGINFO_NAME,34678> {
    public:

    std::optional<LogLevel> level;
    std::optional<std::string> message;
    std::optional<int32_t> error_code;
    std::optional<std::string> file;
    std::optional<int32_t> line;
    std::optional<uint64_t> timestamp;
    

    typedef enum {
        LEVEL_INDEX = 2,
        MESSAGE_INDEX = 3,
        ERROR_CODE_INDEX = 4,
        FILE_INDEX = 5,
        LINE_INDEX = 6,
        TIMESTAMP_INDEX = 7,
    } Field;

    Bytes serialize() const {
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

    LogInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::LEVEL_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *level = static_cast<LogLevel>(v);
}
                    break;
                }
                
                case Field::MESSAGE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *message = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::ERROR_CODE_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *error_code = v;
}
                    break;
                }
                
                case Field::FILE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *file = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::LINE_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *line = v;
}
                    break;
                }
                
                case Field::TIMESTAMP_INDEX:{cbor_value_get_uint64(&mapIt, &*timestamp);
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

static constexpr const char SYSCMD_NAME[] = "SysCmd";

class SysCmd : public Msg<SYSCMD_NAME,51983> {
    public:

    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    std::optional<std::string> console;
    

    typedef enum {
        SRC_INDEX = 2,
        SET_TIME_INDEX = 3,
        REBOOT_INDEX = 4,
        CONSOLE_INDEX = 5,
    } Field;

    Bytes serialize() const {
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

    SysCmd* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::SRC_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        src = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::SET_TIME_INDEX:{cbor_value_get_uint64(&mapIt, &*set_time);
                    break;
                }
                
                case Field::REBOOT_INDEX:{cbor_value_get_boolean(&mapIt, &*reboot);
                    break;
                }
                
                case Field::CONSOLE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *console = std::string(valbuf, vallen - 1);
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

static constexpr const char SYSINFO_NAME[] = "SysInfo";

class SysInfo : public Msg<SYSINFO_NAME,10347> {
    public:

    std::optional<uint64_t> utc;
    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_heap;
    std::optional<uint64_t> flash;
    std::optional<std::string> cpu_board;
    std::optional<std::string> build_date;
    

    typedef enum {
        UTC_INDEX = 1,
        UPTIME_INDEX = 2,
        FREE_HEAP_INDEX = 3,
        FLASH_INDEX = 4,
        CPU_BOARD_INDEX = 5,
        BUILD_DATE_INDEX = 6,
    } Field;

    Bytes serialize() const {
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

    SysInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::UTC_INDEX:{cbor_value_get_uint64(&mapIt, &*utc);
                    break;
                }
                
                case Field::UPTIME_INDEX:{cbor_value_get_uint64(&mapIt, &*uptime);
                    break;
                }
                
                case Field::FREE_HEAP_INDEX:{cbor_value_get_uint64(&mapIt, &*free_heap);
                    break;
                }
                
                case Field::FLASH_INDEX:{cbor_value_get_uint64(&mapIt, &*flash);
                    break;
                }
                
                case Field::CPU_BOARD_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *cpu_board = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::BUILD_DATE_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *build_date = std::string(valbuf, vallen - 1);
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

static constexpr const char WIFIINFO_NAME[] = "WifiInfo";

class WifiInfo : public Msg<WIFIINFO_NAME,15363> {
    public:

    std::optional<std::string> ssid;
    std::optional<std::string> bssid;
    std::optional<int32_t> rssi;
    std::optional<std::string> ip;
    std::optional<std::string> mac;
    std::optional<int32_t> channel;
    std::optional<std::string> gateway;
    std::optional<std::string> netmask;
    

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

    Bytes serialize() const {
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

    WifiInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::SSID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *ssid = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::BSSID_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *bssid = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::RSSI_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *rssi = v;
}
                    break;
                }
                
                case Field::IP_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *ip = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::MAC_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *mac = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::CHANNEL_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *channel = v;
}
                    break;
                }
                
                case Field::GATEWAY_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *gateway = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::NETMASK_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *netmask = std::string(valbuf, vallen - 1);
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

static constexpr const char MULTICASTINFO_NAME[] = "MulticastInfo";

class MulticastInfo : public Msg<MULTICASTINFO_NAME,61310> {
    public:

    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;
    

    typedef enum {
        GROUP_INDEX = 2,
        PORT_INDEX = 3,
        MTU_INDEX = 4,
    } Field;

    Bytes serialize() const {
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

    MulticastInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::GROUP_INDEX:{{
    char valbuf[256];
    size_t vallen = sizeof(valbuf);
    if (cbor_value_is_text_string(&mapIt)) {
        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
        *group = std::string(valbuf, vallen - 1);
    }
}
                    break;
                }
                
                case Field::PORT_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *port = v;
}
                    break;
                }
                
                case Field::MTU_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *mtu = v;
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

static constexpr const char HOVERBOARDINFO_NAME[] = "HoverboardInfo";

class HoverboardInfo : public Msg<HOVERBOARDINFO_NAME,59150> {
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

    Bytes serialize() const {
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

    HoverboardInfo* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::CTRL_MOD_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *ctrl_mod = static_cast<CtrlMod>(v);
}
                    break;
                }
                
                case Field::CTRL_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *ctrl_typ = static_cast<CtrlTyp>(v);
}
                    break;
                }
                
                case Field::CUR_MOT_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *cur_mot_max = v;
}
                    break;
                }
                
                case Field::RPM_MOT_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *rpm_mot_max = v;
}
                    break;
                }
                
                case Field::FI_WEAK_ENA_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *fi_weak_ena = v;
}
                    break;
                }
                
                case Field::FI_WEAK_HI_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *fi_weak_hi = v;
}
                    break;
                }
                
                case Field::FI_WEAK_LO_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *fi_weak_lo = v;
}
                    break;
                }
                
                case Field::FI_WEAK_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *fi_weak_max = v;
}
                    break;
                }
                
                case Field::PHASE_ADV_MAX_DEG_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *phase_adv_max_deg = v;
}
                    break;
                }
                
                case Field::INPUT1_RAW_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input1_raw = v;
}
                    break;
                }
                
                case Field::INPUT1_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *input1_typ = static_cast<InTyp>(v);
}
                    break;
                }
                
                case Field::INPUT1_MIN_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input1_min = v;
}
                    break;
                }
                
                case Field::INPUT1_MID_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input1_mid = v;
}
                    break;
                }
                
                case Field::INPUT1_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input1_max = v;
}
                    break;
                }
                
                case Field::INPUT1_CMD_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input1_cmd = v;
}
                    break;
                }
                
                case Field::INPUT2_RAW_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input2_raw = v;
}
                    break;
                }
                
                case Field::INPUT2_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *input2_typ = static_cast<InTyp>(v);
}
                    break;
                }
                
                case Field::INPUT2_MIN_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input2_min = v;
}
                    break;
                }
                
                case Field::INPUT2_MID_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input2_mid = v;
}
                    break;
                }
                
                case Field::INPUT2_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input2_max = v;
}
                    break;
                }
                
                case Field::INPUT2_CMD_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *input2_cmd = v;
}
                    break;
                }
                
                case Field::AUX_INPUT1_RAW_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input1_raw = v;
}
                    break;
                }
                
                case Field::AUX_INPUT1_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *aux_input1_typ = static_cast<InTyp>(v);
}
                    break;
                }
                
                case Field::AUX_INPUT1_MIN_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input1_min = v;
}
                    break;
                }
                
                case Field::AUX_INPUT1_MID_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input1_mid = v;
}
                    break;
                }
                
                case Field::AUX_INPUT1_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input1_max = v;
}
                    break;
                }
                
                case Field::AUX_INPUT1_CMD_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input1_cmd = v;
}
                    break;
                }
                
                case Field::AUX_INPUT2_RAW_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input2_raw = v;
}
                    break;
                }
                
                case Field::AUX_INPUT2_TYP_INDEX:{{
    long long v;
    cbor_value_get_int64(&mapIt, &v);
    *aux_input2_typ = static_cast<InTyp>(v);
}
                    break;
                }
                
                case Field::AUX_INPUT2_MIN_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input2_min = v;
}
                    break;
                }
                
                case Field::AUX_INPUT2_MID_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input2_mid = v;
}
                    break;
                }
                
                case Field::AUX_INPUT2_MAX_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input2_max = v;
}
                    break;
                }
                
                case Field::AUX_INPUT2_CMD_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *aux_input2_cmd = v;
}
                    break;
                }
                
                case Field::DC_CURR_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *dc_curr = v;
}
                    break;
                }
                
                case Field::RDC_CURR_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *rdc_curr = v;
}
                    break;
                }
                
                case Field::LDC_CURR_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *ldc_curr = v;
}
                    break;
                }
                
                case Field::CMDL_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *cmdl = v;
}
                    break;
                }
                
                case Field::CMDR_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *cmdr = v;
}
                    break;
                }
                
                case Field::SPD_AVG_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *spd_avg = v;
}
                    break;
                }
                
                case Field::SPDL_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *spdl = v;
}
                    break;
                }
                
                case Field::SPDR_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *spdr = v;
}
                    break;
                }
                
                case Field::FILTER_RATE_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *filter_rate = v;
}
                    break;
                }
                
                case Field::SPD_COEF_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *spd_coef = v;
}
                    break;
                }
                
                case Field::STR_COEF_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *str_coef = v;
}
                    break;
                }
                
                case Field::BATV_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *batv = v;
}
                    break;
                }
                
                case Field::TEMP_INDEX:{{
    uint64_t v;
    cbor_value_get_uint64(&mapIt, &v);
    *temp = v;
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

static constexpr const char HOVERBOARDCMD_NAME[] = "HoverboardCmd";

class HoverboardCmd : public Msg<HOVERBOARDCMD_NAME,58218> {
    public:

    std::optional<int32_t> speed;
    std::optional<int32_t> steer;
    

    typedef enum {
        SPEED_INDEX = 0,
        STEER_INDEX = 1,
    } Field;

    Bytes serialize() const {
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

    HoverboardCmd* deserialize(const Bytes& bytes) {
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
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError) {
            delete msg;
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
                delete msg;
                return nullptr;
            }
            switch (key) {
                
                case Field::SPEED_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *speed = v;
}
                    break;
                }
                
                case Field::STEER_INDEX:{{
    int64_t v;
    cbor_value_get_int64(&mapIt, &v);
    *steer = v;
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

