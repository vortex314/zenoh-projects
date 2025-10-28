#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <cbor.h>
#include <tinycbor/cborparser.h>
#include <actor.h>
#include <result.h>

typedef std::vector<uint8_t> Bytes;
// Helper macros for serialization and deserialization

typedef enum
{
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    ALERT = 6,
} LogLevel;

typedef enum
{
    SYS_CMD = 1,
    SYS_INFO = 2,
    WIFI_INFO = 3,
    MOTOR_INFO = 4,
    MOTOR_CMD = 5,
} MessageType;

typedef enum
{
    OFF = 0,
    ON = 1,
} Toggle;

typedef enum
{
    VOLTAGE = 1,
    SPEED = 2,
    TORQUE = 3,
} CtrlMod;

typedef enum
{
    COMMUTATION = 0,
    SINUSOIDAL = 1,
    FOC = 2,
} CtrlTyp;

typedef enum
{
    DISABLED = 0,
    NORMAL_POT = 1,
    MIDDLE_RESTING_POT = 2,
    AUTO_DETECT = 3,
} InTyp;

typedef enum
{
    FLAG_INDEX = 1,
    ID_INDEX = 2,
    NAME_INDEX = 3,
    VALUES_INDEX = 4,
    F_INDEX = 5,
    D_INDEX = 6,
    DATA_INDEX = 7,
} Sample_Fields;

typedef enum
{
    ZID_INDEX = 2,
    WHAT_AM_I_INDEX = 3,
    PEERS_INDEX = 4,
    PREFIX_INDEX = 5,
    ROUTERS_INDEX = 6,
    CONNECT_INDEX = 7,
    LISTEN_INDEX = 8,
} ZenohInfo_Fields;

typedef enum
{
    LEVEL_INDEX = 2,
    MESSAGE_INDEX = 3,
    ERROR_CODE_INDEX = 4,
    FILE_INDEX = 5,
    LINE_INDEX = 6,
} LogInfo_Fields;

typedef enum
{
    SRC_INDEX = 2,
    SET_TIME_INDEX = 3,
    REBOOT_INDEX = 4,
    CONSOLE_INDEX = 5,
} SysCmd_Fields;

typedef enum
{
    UTC_INDEX = 1,
    UPTIME_INDEX = 2,
    FREE_HEAP_INDEX = 3,
    FLASH_INDEX = 4,
    CPU_BOARD_INDEX = 5,
    BUILD_DATE_INDEX = 6,
} SysInfo_Fields;

typedef enum
{
    SSID_INDEX = 2,
    BSSID_INDEX = 3,
    RSSI_INDEX = 4,
    IP_INDEX = 5,
    MAC_INDEX = 6,
    CHANNEL_INDEX = 7,
    GATEWAY_INDEX = 8,
    NETMASK_INDEX = 9,
} WifiInfo_Fields;

typedef enum
{
    GROUP_INDEX = 2,
    PORT_INDEX = 3,
    MTU_INDEX = 4,
} MulticastInfo_Fields;

typedef enum
{
    SPEED_INDEX = 1,
    DIRECTION_INDEX = 2,
    CURRENTA_INDEX = 3,
} HoverboardInfo_Fields;

typedef enum
{
    SRC_INDEX = 2,
    SPEED_INDEX = 3,
    DIRECTION_INDEX = 4,
} HoverboardCmd_Fields;

typedef enum
{
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
} HoverboardInfo_Fields;

typedef enum
{
    SPEED_INDEX = 0,
    STEER_INDEX = 1,
} HoverboardCmd_Fields;

class Sample : public Msg
{
public:
    static constexpr const char *id = "Sample";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 3386;

    std::optional<bool> flag;
    std::optional<int32_t> id;
    std::optional<std::string> name;
    std::vector<float> values;
    std::optional<float> f;
    std::optional<double> d;
    std::optional<Bytes> data;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (flag.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::FLAG_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(flag));
        }
        if (id.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::ID_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(id));
        }
        if (name.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::NAME_INDEX);
            cbor_encode_text_stringz(&mapEncoder, name.c_str());
        }
        if (!values.empty())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::VALUES_INDEX);
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, values.size());
            for (const auto &item : values)
            {
                cbor_encode_float(&arrayEncoder, static_cast<float>(values));
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
        }
        if (f.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::F_INDEX);
            cbor_encode_float(&mapEncoder, static_cast<float>(f));
        }
        if (d.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::D_INDEX);
            cbor_encode_double(&mapEncoder, static_cast<double>(d));
        }
        if (data.has_value())
        {
            cbor_encode_int(&mapEncoder, Sample_Fields::DATA_INDEX);
            cbor_encode_byte_string(&mapEncoder, data.data(), data.size());
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    Sample *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        Sample *msg = new Sample();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case Sample_Fields::FLAG_INDEX:
            {
                {
                    bool v;
                    cbor_value_get_boolean(&mapIt, &v);
                    msg->flag = v;
                }
                break;
            }

            case Sample_Fields::ID_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->id = static_cast<int32_t>(v);
                }
                break;
            }

            case Sample_Fields::NAME_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->name = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case Sample_Fields::VALUES_INDEX:
            {
                {
                    float v;
                    cbor_value_get_float(&mapIt, &v);
                    msg->values = static_cast<float>(v);
                }
                break;
            }

            case Sample_Fields::F_INDEX:
            {
                {
                    float v;
                    cbor_value_get_float(&mapIt, &v);
                    msg->f = static_cast<float>(v);
                }
                break;
            }

            case Sample_Fields::D_INDEX:
            {
                {
                    double v;
                    cbor_value_get_double(&mapIt, &v);
                    msg->d = static_cast<double>(v);
                }
                break;
            }

            case Sample_Fields::DATA_INDEX:
            {
                {
                    uint8_t tmpbuf[512];
                    size_t tmplen = sizeof(tmpbuf);
                    if (cbor_value_is_byte_string(&mapIt))
                    {
                        cbor_value_copy_byte_string(&mapIt, tmpbuf, &tmplen, &mapIt);
                        msg->data = Bytes(tmpbuf, tmpbuf + tmplen);
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

class ZenohInfo : public Msg
{
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

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (zid.has_value())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::ZID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, zid.c_str());
        }
        if (what_am_i.has_value())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::WHAT_AM_I_INDEX);
            cbor_encode_text_stringz(&mapEncoder, what_am_i.c_str());
        }
        if (!peers.empty())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::PEERS_INDEX);
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, peers.size());
            for (const auto &item : peers)
            {
                cbor_encode_text_stringz(&arrayEncoder, peers.c_str());
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
        }
        if (prefix.has_value())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::PREFIX_INDEX);
            cbor_encode_text_stringz(&mapEncoder, prefix.c_str());
        }
        if (!routers.empty())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::ROUTERS_INDEX);
            cbor_encoder_create_array(&mapEncoder, &arrayEncoder, routers.size());
            for (const auto &item : routers)
            {
                cbor_encode_text_stringz(&arrayEncoder, routers.c_str());
            }
            cbor_encoder_close_container(&mapEncoder, &arrayEncoder);
        }
        if (connect.has_value())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::CONNECT_INDEX);
            cbor_encode_text_stringz(&mapEncoder, connect.c_str());
        }
        if (listen.has_value())
        {
            cbor_encode_int(&mapEncoder, ZenohInfo_Fields::LISTEN_INDEX);
            cbor_encode_text_stringz(&mapEncoder, listen.c_str());
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    ZenohInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        ZenohInfo *msg = new ZenohInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case ZenohInfo_Fields::ZID_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->zid = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::WHAT_AM_I_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->what_am_i = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::PEERS_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->peers = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::PREFIX_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->prefix = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::ROUTERS_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->routers = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::CONNECT_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->connect = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case ZenohInfo_Fields::LISTEN_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->listen = std::string(valbuf, vallen - 1);
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

class LogInfo : public Msg
{
public:
    static constexpr const char *id = "LogInfo";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 34678;

    std::optional<LogLevel> level;
    std::optional<std::string> message;
    std::optional<int32_t> error_code;
    std::optional<std::string> file;
    std::optional<int32_t> line;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (level.has_value())
        {
            cbor_encode_int(&mapEncoder, LogInfo_Fields::LEVEL_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(level));
        }
        if (message.has_value())
        {
            cbor_encode_int(&mapEncoder, LogInfo_Fields::MESSAGE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, message.c_str());
        }
        if (error_code.has_value())
        {
            cbor_encode_int(&mapEncoder, LogInfo_Fields::ERROR_CODE_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(error_code));
        }
        if (file.has_value())
        {
            cbor_encode_int(&mapEncoder, LogInfo_Fields::FILE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, file.c_str());
        }
        if (line.has_value())
        {
            cbor_encode_int(&mapEncoder, LogInfo_Fields::LINE_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(line));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    LogInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        LogInfo *msg = new LogInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case LogInfo_Fields::LEVEL_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->level = static_cast<LogLevel>(v);
                }
                break;
            }

            case LogInfo_Fields::MESSAGE_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->message = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case LogInfo_Fields::ERROR_CODE_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->error_code = static_cast<int32_t>(v);
                }
                break;
            }

            case LogInfo_Fields::FILE_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->file = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case LogInfo_Fields::LINE_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->line = static_cast<int32_t>(v);
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

class SysCmd : public Msg
{
public:
    static constexpr const char *id = "SysCmd";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 51983;

    std::string src;
    std::optional<uint64_t> set_time;
    std::optional<bool> reboot;
    std::optional<std::string> console;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        // field: src
        cbor_encode_int(&mapEncoder, SysCmd_Fields::SRC_INDEX);
        cbor_encode_text_stringz(&mapEncoder, src.c_str());
        if (set_time.has_value())
        {
            cbor_encode_int(&mapEncoder, SysCmd_Fields::SET_TIME_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint64_t>(set_time));
        }
        if (reboot.has_value())
        {
            cbor_encode_int(&mapEncoder, SysCmd_Fields::REBOOT_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(reboot));
        }
        if (console.has_value())
        {
            cbor_encode_int(&mapEncoder, SysCmd_Fields::CONSOLE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, console.c_str());
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    SysCmd *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        SysCmd *msg = new SysCmd();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case SysCmd_Fields::SRC_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->src = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case SysCmd_Fields::SET_TIME_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->set_time = static_cast<uint64_t>(v);
                }
                break;
            }

            case SysCmd_Fields::REBOOT_INDEX:
            {
                {
                    bool v;
                    cbor_value_get_boolean(&mapIt, &v);
                    msg->reboot = v;
                }
                break;
            }

            case SysCmd_Fields::CONSOLE_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->console = std::string(valbuf, vallen - 1);
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

class SysInfo : public Msg
{
public:
    static constexpr const char *id = "SysInfo";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 10347;

    std::optional<uint64_t> utc;
    std::optional<uint64_t> uptime;
    std::optional<uint64_t> free_heap;
    std::optional<uint64_t> flash;
    std::optional<std::string> cpu_board;
    std::optional<std::string> build_date;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (utc.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::UTC_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint64_t>(utc));
        }
        if (uptime.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::UPTIME_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint64_t>(uptime));
        }
        if (free_heap.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::FREE_HEAP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint64_t>(free_heap));
        }
        if (flash.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::FLASH_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint64_t>(flash));
        }
        if (cpu_board.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::CPU_BOARD_INDEX);
            cbor_encode_text_stringz(&mapEncoder, cpu_board.c_str());
        }
        if (build_date.has_value())
        {
            cbor_encode_int(&mapEncoder, SysInfo_Fields::BUILD_DATE_INDEX);
            cbor_encode_text_stringz(&mapEncoder, build_date.c_str());
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    SysInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        SysInfo *msg = new SysInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case SysInfo_Fields::UTC_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->utc = static_cast<uint64_t>(v);
                }
                break;
            }

            case SysInfo_Fields::UPTIME_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->uptime = static_cast<uint64_t>(v);
                }
                break;
            }

            case SysInfo_Fields::FREE_HEAP_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->free_heap = static_cast<uint64_t>(v);
                }
                break;
            }

            case SysInfo_Fields::FLASH_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->flash = static_cast<uint64_t>(v);
                }
                break;
            }

            case SysInfo_Fields::CPU_BOARD_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->cpu_board = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case SysInfo_Fields::BUILD_DATE_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->build_date = std::string(valbuf, vallen - 1);
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

class WifiInfo : public Msg
{
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

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (ssid.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::SSID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, ssid.c_str());
        }
        if (bssid.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::BSSID_INDEX);
            cbor_encode_text_stringz(&mapEncoder, bssid.c_str());
        }
        if (rssi.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::RSSI_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(rssi));
        }
        if (ip.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::IP_INDEX);
            cbor_encode_text_stringz(&mapEncoder, ip.c_str());
        }
        if (mac.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::MAC_INDEX);
            cbor_encode_text_stringz(&mapEncoder, mac.c_str());
        }
        if (channel.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::CHANNEL_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(channel));
        }
        if (gateway.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::GATEWAY_INDEX);
            cbor_encode_text_stringz(&mapEncoder, gateway.c_str());
        }
        if (netmask.has_value())
        {
            cbor_encode_int(&mapEncoder, WifiInfo_Fields::NETMASK_INDEX);
            cbor_encode_text_stringz(&mapEncoder, netmask.c_str());
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    WifiInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        WifiInfo *msg = new WifiInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case WifiInfo_Fields::SSID_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->ssid = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case WifiInfo_Fields::BSSID_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->bssid = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case WifiInfo_Fields::RSSI_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->rssi = static_cast<int32_t>(v);
                }
                break;
            }

            case WifiInfo_Fields::IP_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->ip = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case WifiInfo_Fields::MAC_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->mac = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case WifiInfo_Fields::CHANNEL_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->channel = static_cast<int32_t>(v);
                }
                break;
            }

            case WifiInfo_Fields::GATEWAY_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->gateway = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case WifiInfo_Fields::NETMASK_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->netmask = std::string(valbuf, vallen - 1);
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

class MulticastInfo : public Msg
{
public:
    static constexpr const char *id = "MulticastInfo";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 61310;

    std::optional<std::string> group;
    std::optional<int32_t> port;
    std::optional<uint32_t> mtu;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (group.has_value())
        {
            cbor_encode_int(&mapEncoder, MulticastInfo_Fields::GROUP_INDEX);
            cbor_encode_text_stringz(&mapEncoder, group.c_str());
        }
        if (port.has_value())
        {
            cbor_encode_int(&mapEncoder, MulticastInfo_Fields::PORT_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(port));
        }
        if (mtu.has_value())
        {
            cbor_encode_int(&mapEncoder, MulticastInfo_Fields::MTU_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(mtu));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    MulticastInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        MulticastInfo *msg = new MulticastInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case MulticastInfo_Fields::GROUP_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->group = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case MulticastInfo_Fields::PORT_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->port = static_cast<int32_t>(v);
                }
                break;
            }

            case MulticastInfo_Fields::MTU_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->mtu = static_cast<uint32_t>(v);
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

class HoverboardInfo : public Msg
{
public:
    static constexpr const char *id = "HoverboardInfo";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 59150;

    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
    std::optional<int32_t> currentA;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (speed.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::SPEED_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(speed));
        }
        if (direction.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::DIRECTION_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(direction));
        }
        if (currentA.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CURRENTA_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(currentA));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    HoverboardInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        HoverboardInfo *msg = new HoverboardInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case HoverboardInfo_Fields::SPEED_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->speed = static_cast<int32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::DIRECTION_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->direction = static_cast<int32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::CURRENTA_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->currentA = static_cast<int32_t>(v);
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

class HoverboardCmd : public Msg
{
public:
    static constexpr const char *id = "HoverboardCmd";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 58218;

    std::optional<std::string> src;
    std::optional<int32_t> speed;
    std::optional<int32_t> direction;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (src.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardCmd_Fields::SRC_INDEX);
            cbor_encode_text_stringz(&mapEncoder, src.c_str());
        }
        if (speed.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardCmd_Fields::SPEED_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(speed));
        }
        if (direction.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardCmd_Fields::DIRECTION_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(direction));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    HoverboardCmd *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        HoverboardCmd *msg = new HoverboardCmd();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case HoverboardCmd_Fields::SRC_INDEX:
            {
                {
                    char valbuf[256];
                    size_t vallen = sizeof(valbuf);
                    if (cbor_value_is_text_string(&mapIt))
                    {
                        cbor_value_copy_text_string(&mapIt, valbuf, &vallen, &mapIt);
                        msg->src = std::string(valbuf, vallen - 1);
                    }
                }
                break;
            }

            case HoverboardCmd_Fields::SPEED_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->speed = static_cast<int32_t>(v);
                }
                break;
            }

            case HoverboardCmd_Fields::DIRECTION_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->direction = static_cast<int32_t>(v);
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

class HoverboardInfo : public Msg
{
public:
    static constexpr const char *id = "HoverboardInfo";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 59150;

    std::optional<CtrlMod> ctrl_mod;
    std::optional<CtrlTyp> ctrl_typ;
    std::optional<uint32_t> cur_mot_max;
    std::optional<uint32_t> rpm_mot_max;
    std::optional<uuint32> fi_weak_ena;
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

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (ctrl_mod.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CTRL_MOD_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(ctrl_mod));
        }
        if (ctrl_typ.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CTRL_TYP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(ctrl_typ));
        }
        if (cur_mot_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CUR_MOT_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(cur_mot_max));
        }
        if (rpm_mot_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::RPM_MOT_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(rpm_mot_max));
        }
        if (fi_weak_ena.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::FI_WEAK_ENA_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(fi_weak_ena));
        }
        if (fi_weak_hi.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::FI_WEAK_HI_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(fi_weak_hi));
        }
        if (fi_weak_lo.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::FI_WEAK_LO_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(fi_weak_lo));
        }
        if (fi_weak_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::FI_WEAK_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(fi_weak_max));
        }
        if (phase_adv_max_deg.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::PHASE_ADV_MAX_DEG_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(phase_adv_max_deg));
        }
        if (input1_raw.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_RAW_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input1_raw));
        }
        if (input1_typ.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_TYP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(input1_typ));
        }
        if (input1_min.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_MIN_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input1_min));
        }
        if (input1_mid.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_MID_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input1_mid));
        }
        if (input1_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input1_max));
        }
        if (input1_cmd.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT1_CMD_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input1_cmd));
        }
        if (input2_raw.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_RAW_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input2_raw));
        }
        if (input2_typ.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_TYP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(input2_typ));
        }
        if (input2_min.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_MIN_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input2_min));
        }
        if (input2_mid.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_MID_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input2_mid));
        }
        if (input2_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input2_max));
        }
        if (input2_cmd.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::INPUT2_CMD_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(input2_cmd));
        }
        if (aux_input1_raw.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_RAW_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input1_raw));
        }
        if (aux_input1_typ.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_TYP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(aux_input1_typ));
        }
        if (aux_input1_min.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_MIN_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input1_min));
        }
        if (aux_input1_mid.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_MID_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input1_mid));
        }
        if (aux_input1_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input1_max));
        }
        if (aux_input1_cmd.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT1_CMD_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input1_cmd));
        }
        if (aux_input2_raw.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_RAW_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input2_raw));
        }
        if (aux_input2_typ.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_TYP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<long long>(aux_input2_typ));
        }
        if (aux_input2_min.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_MIN_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input2_min));
        }
        if (aux_input2_mid.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_MID_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input2_mid));
        }
        if (aux_input2_max.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_MAX_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input2_max));
        }
        if (aux_input2_cmd.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::AUX_INPUT2_CMD_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(aux_input2_cmd));
        }
        if (dc_curr.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::DC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(dc_curr));
        }
        if (rdc_curr.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::RDC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(rdc_curr));
        }
        if (ldc_curr.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::LDC_CURR_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(ldc_curr));
        }
        if (cmdl.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CMDL_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(cmdl));
        }
        if (cmdr.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::CMDR_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(cmdr));
        }
        if (spd_avg.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::SPD_AVG_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(spd_avg));
        }
        if (spdl.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::SPDL_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(spdl));
        }
        if (spdr.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::SPDR_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(spdr));
        }
        if (filter_rate.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::FILTER_RATE_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(filter_rate));
        }
        if (spd_coef.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::SPD_COEF_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(spd_coef));
        }
        if (str_coef.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::STR_COEF_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(str_coef));
        }
        if (batv.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::BATV_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(batv));
        }
        if (temp.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardInfo_Fields::TEMP_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<uint32_t>(temp));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    HoverboardInfo *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        HoverboardInfo *msg = new HoverboardInfo();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case HoverboardInfo_Fields::CTRL_MOD_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->ctrl_mod = static_cast<CtrlMod>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::CTRL_TYP_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->ctrl_typ = static_cast<CtrlTyp>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::CUR_MOT_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->cur_mot_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::RPM_MOT_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->rpm_mot_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::FI_WEAK_ENA_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->fi_weak_ena = static_cast<uuint32>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::FI_WEAK_HI_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->fi_weak_hi = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::FI_WEAK_LO_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->fi_weak_lo = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::FI_WEAK_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->fi_weak_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::PHASE_ADV_MAX_DEG_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->phase_adv_max_deg = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_RAW_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input1_raw = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_TYP_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->input1_typ = static_cast<InTyp>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_MIN_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input1_min = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_MID_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input1_mid = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input1_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT1_CMD_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input1_cmd = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_RAW_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input2_raw = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_TYP_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->input2_typ = static_cast<InTyp>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_MIN_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input2_min = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_MID_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input2_mid = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input2_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::INPUT2_CMD_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->input2_cmd = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_RAW_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input1_raw = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_TYP_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->aux_input1_typ = static_cast<InTyp>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_MIN_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input1_min = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_MID_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input1_mid = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input1_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT1_CMD_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input1_cmd = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_RAW_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input2_raw = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_TYP_INDEX:
            {
                {
                    long long v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->aux_input2_typ = static_cast<InTyp>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_MIN_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input2_min = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_MID_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input2_mid = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_MAX_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input2_max = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::AUX_INPUT2_CMD_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->aux_input2_cmd = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::DC_CURR_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->dc_curr = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::RDC_CURR_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->rdc_curr = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::LDC_CURR_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->ldc_curr = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::CMDL_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->cmdl = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::CMDR_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->cmdr = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::SPD_AVG_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->spd_avg = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::SPDL_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->spdl = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::SPDR_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->spdr = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::FILTER_RATE_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->filter_rate = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::SPD_COEF_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->spd_coef = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::STR_COEF_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->str_coef = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::BATV_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->batv = static_cast<uint32_t>(v);
                }
                break;
            }

            case HoverboardInfo_Fields::TEMP_INDEX:
            {
                {
                    uint64_t v;
                    cbor_value_get_uint64(&mapIt, &v);
                    msg->temp = static_cast<uint32_t>(v);
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

class HoverboardCmd : public Msg
{
public:
    static constexpr const char *id = "HoverboardCmd";
    inline const char *type_id() const override { return id; };
    static const uint32_t ID = 58218;

    std::optional<int32_t> speed;
    std::optional<int32_t> steer;

    Bytes serialize() const
    {
        // buffer: grow if needed by changing initial size
        std::vector<uint8_t> buffer(512);
        CborEncoder encoder, mapEncoder, arrayEncoder;
        cbor_encoder_init(&encoder, buffer.data(), buffer.size(), 0);

        // Start top-level map
        cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);

        if (speed.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardCmd_Fields::SPEED_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(speed));
        }
        if (steer.has_value())
        {
            cbor_encode_int(&mapEncoder, HoverboardCmd_Fields::STEER_INDEX);
            cbor_encode_int(&mapEncoder, static_cast<int32_t>(steer));
        }
        cbor_encoder_close_container(&encoder, &mapEncoder);
        // get used size
        size_t used = cbor_encoder_get_buffer_size(&encoder, buffer.data());
        return Bytes(buffer.begin(), buffer.begin() + used);
    }

    HoverboardCmd *deserialize(const Bytes &bytes)
    {
        CborParser parser;
        CborValue it, mapIt, tmp;
        HoverboardCmd *msg = new HoverboardCmd();

        CborError err = cbor_parser_init(bytes.data(), bytes.size(), 0, &parser, &it);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
        }

        if (!cbor_value_is_map(&it))
        {
            delete msg;
            return nullptr;
        }

        // enter map
        err = cbor_value_enter_container(&it, &mapIt);
        if (err != CborNoError)
        {
            delete msg;
            return nullptr;
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
                delete msg;
                return nullptr;
            }
            switch (key)
            {

            case HoverboardCmd_Fields::SPEED_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->speed = static_cast<int32_t>(v);
                }
                break;
            }

            case HoverboardCmd_Fields::STEER_INDEX:
            {
                {
                    int64_t v;
                    cbor_value_get_int64(&mapIt, &v);
                    msg->steer = static_cast<int32_t>(v);
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
