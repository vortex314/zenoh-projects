#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <ArduinoJson.h>
#include <cbor.h>
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

class Ps4Info : public Msg {
    MSG(Ps4Info);
    public:
    std::optional<bool> button_left;
    std::optional<bool> button_right;
    std::optional<bool> button_up;
    std::optional<bool> button_down;
    std::optional<bool> button_square;
    std::optional<bool> button_cross;
    std::optional<bool> button_circle;
    std::optional<bool> button_triangle;
    std::optional<bool> button_left_sholder;
    std::optional<bool> button_right_sholder;
    std::optional<bool> button_left_trigger;
    std::optional<bool> button_right_trigger;
    std::optional<bool> button_left_joystick;
    std::optional<bool> button_right_joystick;
    std::optional<bool> button_share;
    std::optional<int32_t> axis_lx;
    std::optional<int32_t> axis_ly;
    std::optional<int32_t> axis_rx;
    std::optional<int32_t> axis_ry;
    std::optional<int32_t> gyro_x;
    std::optional<int32_t> gyro_y;
    std::optional<int32_t> gyro_z;
    std::optional<int32_t> accel_x;
    std::optional<int32_t> accel_y;
    std::optional<int32_t> accel_z;
    
    // Field indexes
        typedef enum {
        BUTTON_LEFT_INDEX = 1,
        BUTTON_RIGHT_INDEX = 2,
        BUTTON_UP_INDEX = 3,
        BUTTON_DOWN_INDEX = 4,
        BUTTON_SQUARE_INDEX = 5,
        BUTTON_CROSS_INDEX = 6,
        BUTTON_CIRCLE_INDEX = 7,
        BUTTON_TRIANGLE_INDEX = 8,
        BUTTON_LEFT_SHOLDER_INDEX = 9,
        BUTTON_RIGHT_SHOLDER_INDEX = 10,
        BUTTON_LEFT_TRIGGER_INDEX = 11,
        BUTTON_RIGHT_TRIGGER_INDEX = 12,
        BUTTON_LEFT_JOYSTICK_INDEX = 13,
        BUTTON_RIGHT_JOYSTICK_INDEX = 14,
        BUTTON_SHARE_INDEX = 15,
        AXIS_LX_INDEX = 16,
        AXIS_LY_INDEX = 17,
        AXIS_RX_INDEX = 18,
        AXIS_RY_INDEX = 19,
        GYRO_X_INDEX = 20,
        GYRO_Y_INDEX = 21,
        GYRO_Z_INDEX = 22,
        ACCEL_X_INDEX = 23,
        ACCEL_Y_INDEX = 24,
        ACCEL_Z_INDEX = 25,
    } Field;
    static Result<Bytes> json_serialize(const Ps4Info&);
    static Result<Ps4Info*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const Ps4Info&);
    static Result<Ps4Info*> cbor_deserialize(const Bytes&);
};

class Ps4Cmd : public Msg {
    MSG(Ps4Cmd);
    public:
    std::optional<int32_t> rumble;
    std::optional<int32_t> led_rgb;
    
    // Field indexes
        typedef enum {
        RUMBLE_INDEX = 1,
        LED_RGB_INDEX = 2,
    } Field;
    static Result<Bytes> json_serialize(const Ps4Cmd&);
    static Result<Ps4Cmd*> json_deserialize(const Bytes&);
    static Result<Bytes> cbor_serialize(const Ps4Cmd&);
    static Result<Ps4Cmd*> cbor_deserialize(const Bytes&);
};

