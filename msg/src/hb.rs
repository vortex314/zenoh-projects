
// ignore dead code
#![allow(dead_code)]
// allow unusde variables
#![allow(unused_variables)]

use minicbor::{Decode, Decoder, Encode};
use anyhow::Result;
use minicbor::Encoder;
use alloc::vec::Vec;
use alloc::collections::VecDeque;

pub enum HbProp {
    CtrlMod = 0,
    CtrlTyp,
    CurMotMax,
    RpmMotMax,
    FiWeakEna,
    FiWeakHi,
    FiWeakLo,
    FiWeakMax,
    PhaseAdvMaxDeg,
    In1Raw,
    In1Typ,
    In1Min,
    In1Mid,
    In1Max,
    In1Cmd,
    In2Raw,
    In2Typ,
    In2Min,
    In2Mid,
    In2Max,
    In2Cmd,
    AuxIn1Raw,
    AuxIn1Typ,
    AuxIn1Min,
    AuxIn1Mid,
    AuxIn1Max,
    AuxIn1Cmd,
    AuxIn2Raw,
    AuxIn2Typ,
    AuxIn2Min,
    AuxIn2Mid,
    AuxIn2Max,
    AuxIn2Cmd,
    DcCurr,
    RdcCurr,
    LdcCurr,
    CmdL,
    CmdR,
    SpdAvg,
    SPDL,
    SPDR,
    FilterRate,
    SpeedCoef,
    SteerCoef,
    BatVoltage,
    Temp=45,
    Speed,
    Steer,
}

#[derive(Encode,Decode,Default,Debug,Clone)]
#[cbor(map)]
struct HbMap {
#[n(0)] pub ctrl_mod: Option<u8>,
#[n(1)] pub ctrl_typ: Option<u8>,
#[n(2)] pub cur_mot_max: Option<u16>,
#[n(3)] pub rpm_mot_max: Option<u16>,
#[n(4)] pub fi_weak_ena: Option<u8>,
#[n(5)] pub fi_weak_hi: Option<u16>,
#[n(6)] pub fi_weak_lo: Option<u16>,
#[n(7)] pub fi_weak_max: Option<u16>,
#[n(8)] pub phase_adv_max_deg: Option<u16>,

#[n(9)] pub in1_raw: Option<u16>,
#[n(10)] pub in1_typ: Option<u8>,
#[n(11)] pub in1_min: Option<u16>,
#[n(12)] pub in1_mid: Option<u16>,
#[n(13)] pub in1_max: Option<u16>,
#[n(14)] pub in1_cmd: Option<u16>,

#[n(15)] pub in2_raw: Option<u16>,
#[n(16)] pub in2_typ: Option<u8>,
#[n(17)] pub in2_min: Option<u16>,
#[n(18)] pub in2_mid: Option<u16>,
#[n(19)] pub in2_max: Option<u16>,
#[n(20)] pub in2_cmd: Option<u16>,

#[n(21)] pub aux_in1_raw: Option<u16>,
#[n(22)] pub aux_in1_typ: Option<u8>,
#[n(23)] pub aux_in1_min: Option<u16>,
#[n(24)] pub aux_in1_mid: Option<u16>,
#[n(25)] pub aux_in1_max: Option<u16>,
#[n(26)] pub aux_in1_cmd: Option<u16>,

#[n(27)] pub aux_in2_raw: Option<u16>,
#[n(28)] pub aux_in2_typ: Option<u8>,
#[n(29)] pub aux_in2_min: Option<u16>,
#[n(30)] pub aux_in2_mid: Option<u16>,
#[n(31)] pub aux_in2_max: Option<u16>,
#[n(32)] pub aux_in2_cmd: Option<u16>,

#[n(33)] pub dc_curr: Option<u16>,
#[n(34)] pub rdc_curr: Option<u16>,
#[n(35)] pub ldc_curr: Option<u16>,
#[n(36)] pub cmd_l: Option<u16>,
#[n(37)] pub cmd_r: Option<u16>,
#[n(38)] pub spd_avg: Option<u16>,
#[n(39)] pub spdl: Option<u16>,
#[n(40)] pub spdr: Option<u16>,
#[n(41)] pub filter_rate: Option<u16>,
#[n(42)] pub speed_coef: Option<u16>,
#[n(43)] pub steer_coef: Option<u16>,
#[n(44)] pub bat_voltage: Option<u16>,
#[n(45)] pub temp: Option<u16>,
#[n(46)] pub speed: Option<i16>,
#[n(47)] pub steer: Option<i16>,
}

pub const START_FRAME: u16 = 0xABCD;


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

#[derive(Debug, Clone)]
pub struct EspNowHeader {
    pub dst: Option<u32>, // always known , can be broadcast = 0
    pub src: Option<u32>, //
    pub msg_type: u8,
    pub msg_id: Option<u16>,
}

impl EspNowHeader {
    /*pub fn new() -> EspNowHeader {
        EspNowHeader {
            dst: None,
            src: None,
            msg_type: 0,
            msg_id: None,
        }
    }*/
    pub fn from_decoder(decoder: &mut Decoder) -> Result<EspNowHeader> {
        let _length = decoder.array().map_err(anyhow::Error::msg)?;
        if _length.is_some() {
            return Err(anyhow::Error::msg("Invalid array length"));
        }
        let dst: Option<u32> = decoder.decode().map_err(anyhow::Error::msg)?;
        let src: Option<u32> = decoder.decode().map_err(anyhow::Error::msg)?;
        let msg_type: u8 = decoder.decode().map_err(anyhow::Error::msg)?;
        let msg_id: Option<u16> = decoder.decode().map_err(anyhow::Error::msg)?;
        Ok(EspNowHeader {
            dst,
            src,
            msg_type,
            msg_id,
        })
    }
    pub fn encode(&self, encoder: &mut Encoder<Vec<u8>>) -> Result<()> {
        encoder.begin_array().map_err(anyhow::Error::msg)?;
        encoder.encode(&self.dst).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.src).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.msg_type).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.msg_id).map_err(anyhow::Error::msg)?;
        Ok(())
    }
}*/

#[derive(Debug, Clone)]
pub struct MotorCmd {
    pub speed: i16,
    pub steer: i16,
}

impl MotorCmd {
    pub fn encode(&self) -> Vec<u8> {
        let mut v = Vec::new();
        v.push((START_FRAME & 0xFF) as u8);
        v.push((START_FRAME >> 8) as u8);
        v.push((self.steer & 0xFF) as u8);
        v.push((self.steer >> 8) as u8);
        v.push((self.speed & 0xFF) as u8);
        v.push((self.speed >> 8) as u8);
        self.add_crc(&mut v);
        v
    }

    fn crc(&self) -> u16 {
        let mut crc = 0;
        crc = crc ^ (START_FRAME) as u16;
        crc = crc ^ self.steer as u16;
        crc = crc ^ self.speed as u16;
        crc
    }

    fn add_crc(&self, vec: &mut Vec<u8>) {
        let crc = self.crc();
        vec.push((crc & 0xFF) as u8);
        vec.push((crc >> 8) as u8);
    }
}
/* 
#[derive(Debug, Clone,PartialEq)]
pub struct MotorEvent {
    frame: u16,
    pub cmd1: i16,
    pub cmd2: i16,
    pub speed_right: i16,
    pub speed_left: i16,
    pub battery_voltage: i16,
    pub board_temperature: i16,
    pub cmd_led: u16,
    crc: u16,
}

impl MotorEvent {
    pub fn new() -> MotorEvent {
        MotorEvent {
            frame: 0,
            cmd1: 0,
            cmd2: 0,
            speed_right: 0,
            speed_left: 0,
            battery_voltage: 0,
            board_temperature: 0,
            cmd_led: 0,
            crc: 0,
        }
    }
    pub fn decode(&mut self, data: &mut VecDeque<u8>) {
        self.frame = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
        self.cmd1 = data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.cmd2 = data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.speed_right =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.speed_left =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.battery_voltage =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.board_temperature =
            data.pop_front().unwrap() as i16 | (data.pop_front().unwrap() as i16) << 8;
        self.cmd_led = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
        self.crc = data.pop_front().unwrap() as u16 | (data.pop_front().unwrap() as u16) << 8;
    }

    fn crc(&self) -> u16 {
        let mut crc = 0;
        crc = crc ^ self.frame as u16;
        crc = crc ^ self.cmd1 as u16;
        crc = crc ^ self.cmd2 as u16;
        crc = crc ^ self.speed_right as u16;
        crc = crc ^ self.speed_left as u16;
        crc = crc ^ self.battery_voltage as u16;
        crc = crc ^ self.board_temperature as u16;
        crc = crc ^ self.cmd_led as u16;
        crc
    }

    pub fn encode(&self,encoder: &mut Encoder<Vec<u8>>) -> Result<()> {
        encoder.begin_array().map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd1).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd2).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.speed_right).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.speed_left).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.battery_voltage).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.board_temperature).map_err(anyhow::Error::msg)?;
        encoder.encode(&self.cmd_led).map_err(anyhow::Error::msg)?;
        encoder.end().map_err(anyhow::Error::msg)?;
        Ok(())
    }
}*/
