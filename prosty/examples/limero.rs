use serde::{Serialize, Deserialize};

#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum MessageType {
    SysCmd,
    SysInfo,
    WifiInfo,
    MotorInfo,
    MotorCmd,
} 

#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum Toggle {
    Off,
    On,
} 



#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct SysCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub set_time:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub reboot:Option<bool>,
        
}
impl SysCmd {
    pub const ID: u32 = 51983;
}

#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct SysInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub uptime:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub free_memory:Option<u64>,
        pub total_memory:Vec<u64>,#[serde(skip_serializing_if = "Option::is_none")]
    pub power_on:Option<Toggle>,
        
}
impl SysInfo {
    pub const ID: u32 = 10347;
}

#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct WifiInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ssid:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub bssid:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub rssi:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub ip:Option<String>,
        
}
impl WifiInfo {
    pub const ID: u32 = 15363;
}

#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct MotorInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        
}
impl MotorInfo {
    pub const ID: u32 = 62329;
}

#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct MotorCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        
}
impl MotorCmd {
    pub const ID: u32 = 32797;
}

#[derive(Debug, Clone,Serialize,Deserialize)]
pub struct LpsInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub msg:Option<String>,
        
}
impl LpsInfo {
    pub const ID: u32 = 24957;
}
