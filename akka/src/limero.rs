use serde::{Serialize, Deserialize};
use anyhow::Result;
use crate::value::Value;

pub trait Msg {
    const ID: u32;
    const NAME: &'static str;
}

pub trait Convert<T> {
    fn to_value(&self) -> Result<Value>;
    fn from_value(value:&Value) -> Result<T>;
    fn exist_in_value(value:&Value) -> bool ;
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
}
impl Convert<ZenohInfo> for ZenohInfo {

     fn  from_value(v:&Value) -> Result<ZenohInfo> {
         let mut m = ZenohInfo::default();
         
         m.zid = v["zid"].as_::<String>().clone().cloned();
         m.what_am_i = v["what_am_i"].as_::<String>().clone().cloned();v.get_array("peers").map(|arr|{
             m.peers = arr.iter().map(|v|v.as_::String().unwrap()).collect();
         });
         m.prefix = v["prefix"].as_::<String>().clone().cloned();v.get_array("routers").map(|arr|{
             m.routers = arr.iter().map(|v|v.as_::String().unwrap()).collect();
         });
         m.connect = v["connect"].as_::<String>().clone().cloned();
         m.listen = v["listen"].as_::<String>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.zid.as_ref().map(|v| value.set("zid", Value::from(v.clone())));
        self.what_am_i.as_ref().map(|v| value.set("what_am_i", Value::from(v.clone())));let arr = self.peers.iter().map(|v|Value::from(v.clone())).collect();
        value.set_array("peers", arr);
        self.prefix.as_ref().map(|v| value.set("prefix", Value::from(v.clone())));let arr = self.routers.iter().map(|v|Value::from(v.clone())).collect();
        value.set_array("routers", arr);
        self.connect.as_ref().map(|v| value.set("connect", Value::from(v.clone())));
        self.listen.as_ref().map(|v| value.set("listen", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<LogInfo> for LogInfo {

     fn  from_value(v:&Value) -> Result<LogInfo> {
         let mut m = LogInfo::default();
         
         m.level = v["level"].as_::<LogLevel>().clone().cloned();
         m.message = v["message"].as_::<String>().clone().cloned();
         m.error_code = v["error_code"].as_::<i32>().clone().cloned();
         m.file = v["file"].as_::<String>().clone().cloned();
         m.line = v["line"].as_::<i32>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.level.as_ref().map(|v| value.set("level", Value::from(v.clone())));
        self.message.as_ref().map(|v| value.set("message", Value::from(v.clone())));
        self.error_code.as_ref().map(|v| value.set("error_code", Value::from(v.clone())));
        self.file.as_ref().map(|v| value.set("file", Value::from(v.clone())));
        self.line.as_ref().map(|v| value.set("line", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
    }
}
    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct SysCmd {
    
    pub src:String,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub set_time:Option<u64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub reboot:Option<bool>,
        

}
impl Msg for SysCmd {
     const ID: u32 = 51983;
     const NAME: &'static str = "SysCmd";
}
impl Convert<SysCmd> for SysCmd {

     fn  from_value(v:&Value) -> Result<SysCmd> {
         let mut m = SysCmd::default();
         m.src = v.get("src").and_then(|v|v.as_<String>()).unwrap();
         m.set_time = v["set_time"].as_::<u64>().clone().cloned();
         m.reboot = v["reboot"].as_::<bool>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        value.set("src", Value::from(&self.src));
        
        self.set_time.as_ref().map(|v| value.set("set_time", Value::from(v.clone())));
        self.reboot.as_ref().map(|v| value.set("reboot", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
        

}
impl Msg for SysInfo {
     const ID: u32 = 10347;
     const NAME: &'static str = "SysInfo";
}
impl Convert<SysInfo> for SysInfo {

     fn  from_value(v:&Value) -> Result<SysInfo> {
         let mut m = SysInfo::default();
         
         m.utc = v["utc"].as_::<u64>().clone().cloned();
         m.uptime = v["uptime"].as_::<u64>().clone().cloned();
         m.free_heap = v["free_heap"].as_::<u64>().clone().cloned();
         m.flash = v["flash"].as_::<u64>().clone().cloned();
         m.cpu_board = v["cpu_board"].as_::<String>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.utc.as_ref().map(|v| value.set("utc", Value::from(v.clone())));
        self.uptime.as_ref().map(|v| value.set("uptime", Value::from(v.clone())));
        self.free_heap.as_ref().map(|v| value.set("free_heap", Value::from(v.clone())));
        self.flash.as_ref().map(|v| value.set("flash", Value::from(v.clone())));
        self.cpu_board.as_ref().map(|v| value.set("cpu_board", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<WifiInfo> for WifiInfo {

     fn  from_value(v:&Value) -> Result<WifiInfo> {
         let mut m = WifiInfo::default();
         
         m.ssid = v["ssid"].as_::<String>().clone().cloned();
         m.bssid = v["bssid"].as_::<String>().clone().cloned();
         m.rssi = v["rssi"].as_::<i32>().clone().cloned();
         m.ip = v["ip"].as_::<String>().clone().cloned();
         m.mac = v["mac"].as_::<String>().clone().cloned();
         m.channel = v["channel"].as_::<i32>().clone().cloned();
         m.gateway = v["gateway"].as_::<String>().clone().cloned();
         m.netmask = v["netmask"].as_::<String>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.ssid.as_ref().map(|v| value.set("ssid", Value::from(v.clone())));
        self.bssid.as_ref().map(|v| value.set("bssid", Value::from(v.clone())));
        self.rssi.as_ref().map(|v| value.set("rssi", Value::from(v.clone())));
        self.ip.as_ref().map(|v| value.set("ip", Value::from(v.clone())));
        self.mac.as_ref().map(|v| value.set("mac", Value::from(v.clone())));
        self.channel.as_ref().map(|v| value.set("channel", Value::from(v.clone())));
        self.gateway.as_ref().map(|v| value.set("gateway", Value::from(v.clone())));
        self.netmask.as_ref().map(|v| value.set("netmask", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<MulticastInfo> for MulticastInfo {

     fn  from_value(v:&Value) -> Result<MulticastInfo> {
         let mut m = MulticastInfo::default();
         
         m.group = v["group"].as_::<String>().clone().cloned();
         m.port = v["port"].as_::<i32>().clone().cloned();
         m.mtu = v["mtu"].as_::<u32>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.group.as_ref().map(|v| value.set("group", Value::from(v.clone())));
        self.port.as_ref().map(|v| value.set("port", Value::from(v.clone())));
        self.mtu.as_ref().map(|v| value.set("mtu", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<HoverboardInfo> for HoverboardInfo {

     fn  from_value(v:&Value) -> Result<HoverboardInfo> {
         let mut m = HoverboardInfo::default();
         
         m.speed = v["speed"].as_::<i32>().clone().cloned();
         m.direction = v["direction"].as_::<i32>().clone().cloned();
         m.currentA = v["currentA"].as_::<i32>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.speed.as_ref().map(|v| value.set("speed", Value::from(v.clone())));
        self.direction.as_ref().map(|v| value.set("direction", Value::from(v.clone())));
        self.currentA.as_ref().map(|v| value.set("currentA", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<HoverboardCmd> for HoverboardCmd {

     fn  from_value(v:&Value) -> Result<HoverboardCmd> {
         let mut m = HoverboardCmd::default();
         
         m.src = v["src"].as_::<String>().clone().cloned();
         m.speed = v["speed"].as_::<i32>().clone().cloned();
         m.direction = v["direction"].as_::<i32>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.src.as_ref().map(|v| value.set("src", Value::from(v.clone())));
        self.speed.as_ref().map(|v| value.set("speed", Value::from(v.clone())));
        self.direction.as_ref().map(|v| value.set("direction", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
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
}
impl Convert<LpsInfo> for LpsInfo {

     fn  from_value(v:&Value) -> Result<LpsInfo> {
         let mut m = LpsInfo::default();
         
         m.direction = v["direction"].as_::<i32>().clone().cloned();
         m.msg = v["msg"].as_::<String>().clone().cloned();
         Ok(m)
     }

     fn  to_value(&self) -> Result<Value>  {
        let mut value = Value::object();
        
        self.direction.as_ref().map(|v| value.set("direction", Value::from(v.clone())));
        self.msg.as_ref().map(|v| value.set("msg", Value::from(v.clone())));Ok(value)
     }

    fn exist_in_value(value:&Value) -> bool {
        value.has_field("MulticastInfo")
    }
}
    
