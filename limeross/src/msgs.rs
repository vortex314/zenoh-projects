use serde::{Serialize, Deserialize};
use serde::de::DeserializeOwned;
use anyhow::Result;
use minicbor::{Encode, Decode};

pub trait TypedMessage : DeserializeOwned + Send + Sync +'static{
    const ID: u32;
    const MSG_TYPE: &'static str;
}
pub trait Msg  : Send + Sync {
    fn type_name(&self) -> &'static str ;
    fn type_id(&self) -> u32 ;
    fn cbor_serialize(&self) -> Result<Vec<u8>>;
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized;
    fn json_serialize(&self) -> Result<Vec<u8>>;
    fn json_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized;
}


#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum LogLevel {
    #[n(1)]
    Debug,
    #[n(2)]
    Info,
    #[n(3)]
    Warn,
    #[n(4)]
    Error,
    #[n(5)]
    Fatal,
    #[n(6)]
    Alert,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum MessageType {
    #[n(1)]
    SysCmd,
    #[n(2)]
    SysInfo,
    #[n(3)]
    WifiInfo,
    #[n(4)]
    MotorInfo,
    #[n(5)]
    MotorCmd,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum Toggle {
    #[n(0)]
    Off,
    #[n(1)]
    On,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum CtrlMod {
    #[n(1)]
    Voltage,
    #[n(2)]
    Speed,
    #[n(3)]
    Torque,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum CtrlTyp {
    #[n(0)]
    Commutation,
    #[n(1)]
    Sinusoidal,
    #[n(2)]
    Foc,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum InTyp {
    #[n(0)]
    Disabled,
    #[n(1)]
    NormalPot,
    #[n(2)]
    MiddleRestingPot,
    #[n(3)]
    AutoDetect,
}

#[derive(Debug, Clone, Serialize, Deserialize,Encode, Decode)]
pub enum LawnmowerMode {
    #[n(0)]
    Manual,
    #[n(1)]
    Auto,
    #[n(2)]
    Paused,
    #[n(3)]
    EmergencyStop,
}



#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct Alive {
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub subscribe: Option<Vec<String>>,
}

impl TypedMessage for Alive {
    const ID: u32 = 57419;
    const MSG_TYPE: &'static str = "Alive";
}

impl Msg for Alive {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct UdpMessage {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub dst: Option<String>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub src: Option<String>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub msg_type: Option<String>,
    #[cbor(n(4), with = "minicbor::bytes")]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub payload: Option<Vec<u8>>,
}

impl TypedMessage for UdpMessage {
    const ID: u32 = 61718;
    const MSG_TYPE: &'static str = "UdpMessage";
}

impl Msg for UdpMessage {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct UdpMessageCbor {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub dst: Option<u32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub src: Option<u32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub msg_type: Option<u32>,
    #[cbor(n(4), with = "minicbor::bytes")]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub payload: Option<Vec<u8>>,
}

impl TypedMessage for UdpMessageCbor {
    const ID: u32 = 65322;
    const MSG_TYPE: &'static str = "UdpMessageCbor";
}

impl Msg for UdpMessageCbor {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct ZenohEvent {
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub zid: Option<String>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub what_am_i: Option<String>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub peers: Option<Vec<String>>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub prefix: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub routers: Option<Vec<String>>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub connect: Option<String>,
    #[n(8)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub listen: Option<String>,
}

impl TypedMessage for ZenohEvent {
    const ID: u32 = 48902;
    const MSG_TYPE: &'static str = "ZenohEvent";
}

impl Msg for ZenohEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LogEvent {
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub level: Option<LogLevel>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message: Option<String>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error_code: Option<i32>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub file: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub line: Option<i32>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub timestamp: Option<u64>,
}

impl TypedMessage for LogEvent {
    const ID: u32 = 29204;
    const MSG_TYPE: &'static str = "LogEvent";
}

impl Msg for LogEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct SysCmd {
    #[n(2)]
    
    pub src: String,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub set_time: Option<u64>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub reboot: Option<bool>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub console: Option<String>,
}

impl TypedMessage for SysCmd {
    const ID: u32 = 51983;
    const MSG_TYPE: &'static str = "SysCmd";
}

impl Msg for SysCmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct SysEvent {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub utc: Option<u64>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub uptime: Option<u64>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub free_heap: Option<u64>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub flash: Option<u64>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub cpu_board: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub build_date: Option<String>,
}

impl TypedMessage for SysEvent {
    const ID: u32 = 23049;
    const MSG_TYPE: &'static str = "SysEvent";
}

impl Msg for SysEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct WifiEvent {
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ssid: Option<String>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub bssid: Option<String>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rssi: Option<i32>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ip: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mac: Option<String>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub channel: Option<i32>,
    #[n(8)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub gateway: Option<String>,
    #[n(9)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub netmask: Option<String>,
}

impl TypedMessage for WifiEvent {
    const ID: u32 = 54881;
    const MSG_TYPE: &'static str = "WifiEvent";
}

impl Msg for WifiEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct MulticastEvent {
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub group: Option<String>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub port: Option<i32>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mtu: Option<u32>,
}

impl TypedMessage for MulticastEvent {
    const ID: u32 = 53788;
    const MSG_TYPE: &'static str = "MulticastEvent";
}

impl Msg for MulticastEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct PingReq {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub number: Option<u32>,
}

impl TypedMessage for PingReq {
    const ID: u32 = 27754;
    const MSG_TYPE: &'static str = "PingReq";
}

impl Msg for PingReq {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct PingRep {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub number: Option<u32>,
}

impl TypedMessage for PingRep {
    const ID: u32 = 28011;
    const MSG_TYPE: &'static str = "PingRep";
}

impl Msg for PingRep {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct HoverboardEvent {
    #[n(0)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ctrl_mod: Option<CtrlMod>,
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ctrl_typ: Option<CtrlTyp>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub cur_mot_max: Option<i32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rpm_mot_max: Option<i32>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_ena: Option<i32>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_hi: Option<i32>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_lo: Option<i32>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_max: Option<i32>,
    #[n(8)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub phase_adv_max_deg: Option<i32>,
    #[n(9)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_raw: Option<i32>,
    #[n(10)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_typ: Option<InTyp>,
    #[n(11)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_min: Option<i32>,
    #[n(12)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_mid: Option<i32>,
    #[n(13)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_max: Option<i32>,
    #[n(14)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_cmd: Option<i32>,
    #[n(15)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_raw: Option<i32>,
    #[n(16)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_typ: Option<InTyp>,
    #[n(17)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_min: Option<i32>,
    #[n(18)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_mid: Option<i32>,
    #[n(19)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_max: Option<i32>,
    #[n(20)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_cmd: Option<i32>,
    #[n(21)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_raw: Option<i32>,
    #[n(22)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_typ: Option<InTyp>,
    #[n(23)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_min: Option<i32>,
    #[n(24)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_mid: Option<i32>,
    #[n(25)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_max: Option<i32>,
    #[n(26)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_cmd: Option<i32>,
    #[n(27)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_raw: Option<i32>,
    #[n(28)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_typ: Option<InTyp>,
    #[n(29)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_min: Option<i32>,
    #[n(30)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_mid: Option<i32>,
    #[n(31)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_max: Option<i32>,
    #[n(32)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_cmd: Option<i32>,
    #[n(33)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub dc_curr: Option<i32>,
    #[n(34)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rdc_curr: Option<i32>,
    #[n(35)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ldc_curr: Option<i32>,
    #[n(36)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub cmdl: Option<i32>,
    #[n(37)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub cmdr: Option<i32>,
    #[n(38)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub spd_avg: Option<i32>,
    #[n(39)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub spdl: Option<i32>,
    #[n(40)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub spdr: Option<i32>,
    #[n(41)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub filter_rate: Option<i32>,
    #[n(42)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub spd_coef: Option<i32>,
    #[n(43)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub str_coef: Option<i32>,
    #[n(44)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub batv: Option<i32>,
    #[n(45)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub temp: Option<i32>,
}

impl TypedMessage for HoverboardEvent {
    const ID: u32 = 31340;
    const MSG_TYPE: &'static str = "HoverboardEvent";
}

impl Msg for HoverboardEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct HoverboardCmd {
    #[n(0)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed: Option<i32>,
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub steer: Option<i32>,
}

impl TypedMessage for HoverboardCmd {
    const ID: u32 = 58218;
    const MSG_TYPE: &'static str = "HoverboardCmd";
}

impl Msg for HoverboardCmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct HoverboardReply {
    #[n(0)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error_code: Option<i32>,
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message: Option<String>,
}

impl TypedMessage for HoverboardReply {
    const ID: u32 = 30066;
    const MSG_TYPE: &'static str = "HoverboardReply";
}

impl Msg for HoverboardReply {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct TouchPoint {
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub active: Option<bool>,
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub id: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub x: Option<i32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub y: Option<i32>,
}

impl TypedMessage for TouchPoint {
    const ID: u32 = 49173;
    const MSG_TYPE: &'static str = "TouchPoint";
}

impl Msg for TouchPoint {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct Ps4Event {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left: Option<bool>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right: Option<bool>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_up: Option<bool>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_down: Option<bool>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_square: Option<bool>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_cross: Option<bool>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_circle: Option<bool>,
    #[n(8)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_triangle: Option<bool>,
    #[n(9)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_shoulder: Option<bool>,
    #[n(10)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_shoulder: Option<bool>,
    #[n(11)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_trigger: Option<bool>,
    #[n(12)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_trigger: Option<bool>,
    #[n(13)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_joystick: Option<bool>,
    #[n(14)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_joystick: Option<bool>,
    #[n(15)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_share: Option<bool>,
    #[n(16)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_options: Option<bool>,
    #[n(33)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_touchpad: Option<bool>,
    #[n(34)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_ps: Option<bool>,
    #[n(17)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_lx: Option<i32>,
    #[n(18)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_ly: Option<i32>,
    #[n(19)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_rx: Option<i32>,
    #[n(20)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_ry: Option<i32>,
    #[n(21)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_x: Option<i32>,
    #[n(22)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_y: Option<i32>,
    #[n(23)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_z: Option<i32>,
    #[n(24)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_x: Option<i32>,
    #[n(25)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_y: Option<i32>,
    #[n(26)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_z: Option<i32>,
    #[n(27)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub connected: Option<bool>,
    #[n(28)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub battery_level: Option<i32>,
    #[n(30)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub bluetooth: Option<bool>,
    #[n(31)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub debug: Option<String>,
    #[n(32)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub temp: Option<i32>,
}

impl TypedMessage for Ps4Event {
    const ID: u32 = 29767;
    const MSG_TYPE: &'static str = "Ps4Event";
}

impl Msg for Ps4Event {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct Ps4Cmd {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rumble_small: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rumble_large: Option<i32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led_red: Option<i32>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led_green: Option<i32>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led_blue: Option<i32>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led_flash_on: Option<i32>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led_flash_off: Option<i32>,
}

impl TypedMessage for Ps4Cmd {
    const ID: u32 = 50497;
    const MSG_TYPE: &'static str = "Ps4Cmd";
}

impl Msg for Ps4Cmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct CameraEvent {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub width: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub height: Option<i32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub format: Option<String>,
    #[cbor(n(4), with = "minicbor::bytes")]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub data: Option<Vec<u8>>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led: Option<bool>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub quality: Option<i32>,
}

impl TypedMessage for CameraEvent {
    const ID: u32 = 32617;
    const MSG_TYPE: &'static str = "CameraEvent";
}

impl Msg for CameraEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct CameraCmd {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led: Option<bool>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub capture_tcp_destination: Option<String>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub quality: Option<i32>,
}

impl TypedMessage for CameraCmd {
    const ID: u32 = 61551;
    const MSG_TYPE: &'static str = "CameraCmd";
}

impl Msg for CameraCmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct CameraReply {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error_code: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message: Option<String>,
    #[cbor(n(3), with = "minicbor::bytes")]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub data: Option<Vec<u8>>,
}

impl TypedMessage for CameraReply {
    const ID: u32 = 32887;
    const MSG_TYPE: &'static str = "CameraReply";
}

impl Msg for CameraReply {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerManualEvent {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub steering: Option<i32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub blade: Option<bool>,
}

impl TypedMessage for LawnmowerManualEvent {
    const ID: u32 = 24124;
    const MSG_TYPE: &'static str = "LawnmowerManualEvent";
}

impl Msg for LawnmowerManualEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerManualCmd {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed: Option<f32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub steer: Option<f32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub blade: Option<bool>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub start_manual_control: Option<bool>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stop_manual_control: Option<bool>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub emergency_stop: Option<bool>,
    #[n(7)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub start_auto_mode: Option<bool>,
    #[n(8)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stop_auto_mode: Option<bool>,
}

impl TypedMessage for LawnmowerManualCmd {
    const ID: u32 = 1850;
    const MSG_TYPE: &'static str = "LawnmowerManualCmd";
}

impl Msg for LawnmowerManualCmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerManualReply {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error_code: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message: Option<String>,
}

impl TypedMessage for LawnmowerManualReply {
    const ID: u32 = 22818;
    const MSG_TYPE: &'static str = "LawnmowerManualReply";
}

impl Msg for LawnmowerManualReply {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerAutoEvent {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub started: Option<bool>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stopped: Option<bool>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub paused: Option<bool>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub resumed: Option<bool>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mode: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub path: Option<String>,
}

impl TypedMessage for LawnmowerAutoEvent {
    const ID: u32 = 58665;
    const MSG_TYPE: &'static str = "LawnmowerAutoEvent";
}

impl Msg for LawnmowerAutoEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerAutoCmd {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub start: Option<bool>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stop: Option<bool>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub pause: Option<bool>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub resume: Option<bool>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mode: Option<String>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub path: Option<String>,
}

impl TypedMessage for LawnmowerAutoCmd {
    const ID: u32 = 22063;
    const MSG_TYPE: &'static str = "LawnmowerAutoCmd";
}

impl Msg for LawnmowerAutoCmd {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct LawnmowerStatus {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub battery_level: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub blade_status: Option<bool>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub current_mode: Option<String>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error_message: Option<String>,
}

impl TypedMessage for LawnmowerStatus {
    const ID: u32 = 21374;
    const MSG_TYPE: &'static str = "LawnmowerStatus";
}

impl Msg for LawnmowerStatus {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

#[derive(Debug, Clone, Serialize, Deserialize, Default,Encode, Decode)]
#[cbor(map)]
pub struct MotorEvent {
    #[n(1)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub motor_id: Option<i32>,
    #[n(2)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub temperature: Option<f32>,
    #[n(3)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub voltage: Option<f32>,
    #[n(4)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub current: Option<f32>,
    #[n(5)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed: Option<f32>,
    #[n(6)]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub position: Option<f32>,
}

impl TypedMessage for MotorEvent {
    const ID: u32 = 55067;
    const MSG_TYPE: &'static str = "MotorEvent";
}

impl Msg for MotorEvent {
    fn type_name(&self) -> &'static str {<Self as TypedMessage>::MSG_TYPE}
    fn type_id(&self) -> u32 {<Self as TypedMessage>::ID}
    fn cbor_serialize(&self) -> Result<Vec<u8>> {Ok(minicbor::to_vec(self)?)}
    fn cbor_deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized {Ok(minicbor::decode::<Self>(v.as_slice())?)}
    fn json_serialize(&self) -> Result<Vec<u8>> {Ok(serde_json::to_vec(self) ?)}
    fn json_deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {Ok(serde_json::from_slice(v.as_slice()) ?)}
}
    

