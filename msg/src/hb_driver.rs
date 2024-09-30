struct HbCmd {
    speed:i16,
    steer:i16,
}

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

/*
Field	Ouput
in1	Input1
in2	Input2
cmdR	Right Wheel Speed Command (not the measured speed)
cmdL	Left Wheel Speed Command (not the measured speed)
BatADC	Battery adc-value measured by mainboard
BatV	Battery calibrated voltage multiplied by 100 for verifying battery voltage calibration
TempADC	For board temperature calibration
Temp	Temperature in celcius for verifying board temperature calibration
*/

struct HbEvent {
    cmd1:i16,
    cmd2:i16,
    cmdR:i16,
    cmdL:i16,
    battery_ADC:i16,
    battery_voltage:i16,
    temperature_ADC:i16,
    temperature:i16,
}

enum VariableIndex {
    CTRL_MOD = 0,
    CTRL_TYP = 1,
    I_MOT_MAX = 2,
    N_MOT_MAX = 3,
    FI_WEAK_ENA = 4,
    FI_WEAK_HI = 5,
    FI_WEAK_LO = 6,
    FI_WEAK_MAX = 7,
    PHA_ADV_MAX = 8,
    IN1_RAW = 9,
    IN1_TYP = 10,
    IN1_MIN = 11,
    IN1_MID = 12,
    IN1_MAX = 13,
    IN1_CMD = 14,
    IN2_RAW = 15,
    IN2_TYP = 16,
    IN2_MIN = 17,
    IN2_MID = 18,
    IN2_MAX = 19,
    IN2_CMD = 20,
    DC_CURR = 21,
    LDC_CURR = 22,
    RDC_CURR = 23,

}

struct HbVariable {
    ctrl_mod:u8,
    ctrl_typ:u8,
    i_mot_max:u16,
    n_mot_max:u16,
    fi_weak_ena:u8,
    fi_weak_hi:u16,
    fi_weak_lo:u16,
    fi_weak_max:u16,
    pha_adv_max:u16,
    in1_raw:u16,
    in1_typ:u8,
    in1_min:u16,
    in1_mid:u16,
    in1_max:u16,
    in1_cmd:u16,
    in2_raw:u16,
    in2_typ:u8,
    in2_min:u16,
    in2_mid:u16,
    in2_max:u16,
    in2_cmd:u16,
    dc_curr:u16,
    ldc_curr:u16,
    rdc_curr:u16,
    cmdl:u16,
    cmdr:u16,
    spd_avg:u16,
    spdl:u16,
    spdr:u16,
    rate:u16,
    spd_coef:u16,
    str_coef:u16,
    batv:u16,
    temp:u16,
}

struct Serialized {
    vars : BtreeMap<u8,i32> ,
}

impl Serialized {
    fn new() -> Serialized {
        Serialized {
            vars: BTreeMap::new(),
        }
    }

    fn push(idx:u8,value:i32){
        
    }
}

