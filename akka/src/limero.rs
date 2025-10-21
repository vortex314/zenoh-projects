use serde::{Serialize, Deserialize};
use anyhow::Result;
use crate::value::Value;

pub trait Msg {
    const ID: u32;
    const NAME: &'static str;
    fn serialize(&self) -> Result<Vec<u8>>;
    fn deserialize(v:&Vec<u8>) -> Result<Self> where Self : Sized;
}


#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Alert,
} 

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



#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct ZenohInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub zid:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub what_am_i:Option<String>,
        pub peers:Vec<String>,#[serde(skip_serializing_if = "Option::is_none")]
    pub prefix:Option<String>,
        pub routers:Vec<String>,#[serde(skip_serializing_if = "Option::is_none")]
    pub connect:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub listen:Option<String>,
        

}
impl Msg for ZenohInfo {
     const ID: u32 = 33380;
     const NAME: &'static str = "ZenohInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:ZenohInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LogInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub level:Option<LogLevel>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub message:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub error_code:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub file:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub line:Option<i32>,
        

}
impl Msg for LogInfo {
     const ID: u32 = 34678;
     const NAME: &'static str = "LogInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LogInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct SysCmd {
    
    pub src:String,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub set_time:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub reboot:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub console:Option<String>,
        

}
impl Msg for SysCmd {
     const ID: u32 = 51983;
     const NAME: &'static str = "SysCmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:SysCmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct SysInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub utc:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub uptime:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub free_heap:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub flash:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub cpu_board:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub build_date:Option<String>,
        

}
impl Msg for SysInfo {
     const ID: u32 = 10347;
     const NAME: &'static str = "SysInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:SysInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct WifiInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ssid:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub bssid:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub rssi:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub ip:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub mac:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub channel:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub gateway:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub netmask:Option<String>,
        

}
impl Msg for WifiInfo {
     const ID: u32 = 15363;
     const NAME: &'static str = "WifiInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:WifiInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct MulticastInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub group:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub port:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub mtu:Option<u32>,
        

}
impl Msg for MulticastInfo {
     const ID: u32 = 61310;
     const NAME: &'static str = "MulticastInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:MulticastInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct HoverboardInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub currentA:Option<i32>,
        

}
impl Msg for HoverboardInfo {
     const ID: u32 = 59150;
     const NAME: &'static str = "HoverboardInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:HoverboardInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct HoverboardCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub src:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        

}
impl Msg for HoverboardCmd {
     const ID: u32 = 58218;
     const NAME: &'static str = "HoverboardCmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:HoverboardCmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LpsInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub msg:Option<String>,
        

}
impl Msg for LpsInfo {
     const ID: u32 = 24957;
     const NAME: &'static str = "LpsInfo";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LpsInfo = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    
