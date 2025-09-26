use serde::{Serialize, Deserialize};
use crate::value::*;
use anyhow::Result;

pub trait Msg {
    const ID: u32;
    const NAME: &'static str;
}

pub trait SerdesValue<T> {
    fn from_value(v:&Value)->Result<T>;
    fn to_value(t:&T) -> Result<Value>;
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
pub struct SysCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub set_time:Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub reboot:Option<bool>,
}

impl Msg for SysCmd {
     const ID: u32 = 51983;
     const NAME: &'static str = "SysCmd";
}

impl SerdesValue<SysCmd> for SysCmd {
    fn from_value(v:&Value)->Result<SysCmd> {
        let mut sys_cmd = SysCmd::default();
        v["set_time"].set_if_opt(&mut sys_cmd.set_time);
        v["reboot"].set_if_opt(&mut sys_cmd.reboot);
        Ok(SysCmd::default())
    }
    fn to_value(sys_cmd:&SysCmd) -> Result<Value> {
        let mut value = Value::object();
        v.get_if_opt("set_time",sys_cmd.set_time);
        
        if let Some(set_time) = sys_cmd.set_time {
            v["set_time"]=sys_cmd.set_time.unwrap();
        }

        Ok(Value::Null)
    }
}




#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct SysInfo {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub uptime:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub free_heap:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub flash:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub cpu_board:Option<String>,
        
}
impl Msg for SysInfo {
     const ID: u32 = 10347;
     const NAME: &'static str = "SysInfo";

     fn  from_value(Value) -> Result<SysInfo> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
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

     fn  from_value(Value) -> Result<WifiInfo> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
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

     fn  from_value(Value) -> Result<MulticastInfo> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
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

     fn  from_value(Value) -> Result<HoverboardInfo> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
}


#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct HoverboardCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub direction:Option<i32>,
        
}
impl Msg for HoverboardCmd {
     const ID: u32 = 58218;
     const NAME: &'static str = "HoverboardCmd";

     fn  from_value(Value) -> Result<HoverboardCmd> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
}


#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct Msg {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub src:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub dst:Option<String>,
        
}
impl Msg for Msg {
     const ID: u32 = 14661;
     const NAME: &'static str = "Msg";

     fn  from_value(Value) -> Result<Msg> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
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

     fn  from_value(Value) -> Result<LpsInfo> {

        serde_json::from_value(Value)
     }

     fn  to_value(&self) -> Result<Value>  {
        serde_json::to_value(self)
     }

}

fn json_to_value(json:&String) -> Result<Value> {
    // to value 
}
fn value_to_json(value:&Value) -> Result<String> {
    
}

