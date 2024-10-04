
/*
Type	Name	Description	Can be Set	Can be saved to EEPROM
Parameter	CTRL_MOD	Ctrl mode 1:Voltage 2:Speed 3:Torque	Yes	No
Parameter	CTRL_TYP	Ctrl type 0:Commutation 1:Sinusoidal 2:FOC	Yes	No
Parameter	I_MOT_MAX	Max phase current A	Yes	Yes
Parameter	N_MOT_MAX	Max motor RPM	Yes	Yes
Parameter	FI_WEAK_ENA	Enable field weak 0:OFF 1:ON	Yes	No
Parameter	FI_WEAK_HI	Field weak high RPM	Yes	No
Parameter	FI_WEAK_LO	Field weak low RPM	Yes	No
Parameter	FI_WEAK_MAX	Field weak max current A(FOC only)	Yes	No
Parameter	PHA_ADV_MAX	Max Phase Adv angle Deg(SIN only)	Yes	No
Variable	IN1_RAW	Input1 raw value	No	No
Parameter	IN1_TYP	Input1 type 0:Disabled, 1:Normal Pot, 2:Middle Resting Pot, 3:Auto-detect	Yes	Yes
Parameter	IN1_MIN	Input1 minimum value	Yes	Yes
Parameter	IN1_MID	Input1 middle value	Yes	Yes
Parameter	IN1_MAX	Input1 maximum value	Yes	Yes
Variable	IN1_CMD	Input1 command value	No	No
Variable	IN2_RAW	Input2 raw value	No	No
Parameter	IN2_TYP	Input2 type 0:Disabled, 1:Normal Pot, 2:Middle Resting Pot, 3:Auto-detect	Yes	Yes
Parameter	IN2_MIN	Input2 minimum value	Yes	Yes
Parameter	IN2_MID	Input2 middle value	Yes	Yes
Parameter	IN2_MAX	Input2 maximum value	Yes	Yes
Variable	IN2_CMD	Input2 command value	No	No
Variable	DC_CURR	Total DC Link current A *100	No	No
Variable	LDC_CURR	Left DC Link current A *100	No	No
Variable	RDC_CURR	Right DC Link current A *100	No	No
Variable	CMDL	Left Motor Command RPM	No	No
Variable	CMDR	Right Motor Command RPM	No	No
Variable	SPD_AVG	Motor Measured Avg RPM	No	No
Variable	SPDL	Left Motor Measured RPM	No	No
Variable	SPDR	Right Motor Measured RPM	No	No
Variable	RATE	Rate *10	No	No
Variable	SPD_COEF	Speed Coefficient *10	No	No
Variable	STR_COEF	Steer Coefficient *10	No	No
Variable	BATV	Calibrated Battery Voltage *100	No	No
Variable	TEMP	Calibrated Temperature °C *10	No	No
*/
#include <stdint.h>
typedef enum {
    CTRL_MOD = 0,
    CTRL_TYP,
    I_MOT_MAX,
    N_MOT_MAX,
    FI_WEAK_ENA,
    FI_WEAK_HI,
    FI_WEAK_LO,
    FI_WEAK_MAX,
    PHA_ADV_MAX,
    IN1_RAW,
    IN1_TYP,
    IN1_MIN,
    IN1_MID,
    IN1_MAX,
    IN1_CMD,
    IN2_RAW,
    IN2_TYP,
    IN2_MIN,
    IN2_MID,
    IN2_MAX,
    IN2_CMD,
    DC_CURR,
    LDC_CURR,
    RDC_CURR,
    CMDL,
    CMDR,
    SPD_AVG,
    SPDL,
    SPDR,
    RATE,
    SPD_COEF,
    STR_COEF,
    BATV,
    TEMP,
} HoverboardProperty;

const uint32_t LM1_DRIVE = FNV("lm1/drive");
const uint32_t LM1_CUTTER = FNV("lm1/cutter");
const uint32_t LM1_NAVI = FNV("lm1/compass");
const uint32_t LM1_LIGHT = FNV("lm1/light");
const uint32_t LM1_HORN = FNV("lm1/horn");
const uint32_t LM1_POWER = FNV("lm1/brake");

enum InfoProperty {
    NAME = 0,
    DESCRIPTION,
    MODE,
    TYPE,
    IDX,
    MAX,
    MIN,
};

typedef struct {
    uint8_t idx;
    uint8_t type;
    uint8_t mode;
    uint8_t max;
    uint8_t min;
    const char* name;
    const char* description;
} Info;

void func(obj_id : uint32_t, prop : uint8_t, value : uint32_t) {
    Info info;
    switch (prop) {
        case LM1_DRIVE:
            info = {0, 0, 0, 100, 0, "Drive", "Drive the robot"};
            break;
        case LM1_CUTTER:
            info = {1, 0, 0, 100, 0, "Cutter", "Cut the grass"};
            break;
        case LM1_NAVI:
            info = {2, 0, 0, 100, 0, "Compass", "Navigate"};
            break;
    }
}

