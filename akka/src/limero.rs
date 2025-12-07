use serde::{Serialize, Deserialize};
use anyhow::Result;

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

#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum CtrlMod {
    Voltage,
    Speed,
    Torque,
} 

#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum CtrlTyp {
    Commutation,
    Sinusoidal,
    Foc,
} 

#[derive(Debug, Clone,Serialize,Deserialize)] 
pub enum InTyp {
    Disabled,
    NormalPot,
    MiddleRestingPot,
    AutoDetect,
} 



#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct Sample {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub flag:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub identifier:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub name:Option<String>,
        pub values:Vec<f32>,#[serde(skip_serializing_if = "Option::is_none")]
    pub f:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub d:Option<f64>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub data:Option<Vec<u8>>,
        

}
impl Msg for Sample {
     const ID: u32 = 3386;
     const NAME: &'static str = "Sample";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:Sample = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct ZenohEvent {
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
impl Msg for ZenohEvent {
     const ID: u32 = 48902;
     const NAME: &'static str = "ZenohEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:ZenohEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LogEvent {
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
        #[serde(skip_serializing_if = "Option::is_none")]
    pub timestamp:Option<u64>,
        

}
impl Msg for LogEvent {
     const ID: u32 = 29204;
     const NAME: &'static str = "LogEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LogEvent = serde_json::from_slice(v.as_slice()) ?;
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
pub struct SysEvent {
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
impl Msg for SysEvent {
     const ID: u32 = 23049;
     const NAME: &'static str = "SysEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:SysEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct WifiEvent {
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
impl Msg for WifiEvent {
     const ID: u32 = 54881;
     const NAME: &'static str = "WifiEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:WifiEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct MulticastEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub group:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub port:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub mtu:Option<u32>,
        

}
impl Msg for MulticastEvent {
     const ID: u32 = 53788;
     const NAME: &'static str = "MulticastEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:MulticastEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct HoverboardEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub ctrl_mod:Option<CtrlMod>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub ctrl_typ:Option<CtrlTyp>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub cur_mot_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub rpm_mot_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_ena:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_hi:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_lo:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub fi_weak_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub phase_adv_max_deg:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_raw:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_typ:Option<InTyp>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_min:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_mid:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input1_cmd:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_raw:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_typ:Option<InTyp>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_min:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_mid:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub input2_cmd:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_raw:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_typ:Option<InTyp>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_min:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_mid:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input1_cmd:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_raw:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_typ:Option<InTyp>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_min:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_mid:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_max:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub aux_input2_cmd:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub dc_curr:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub rdc_curr:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub ldc_curr:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub cmdl:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub cmdr:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub spd_avg:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub spdl:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub spdr:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub filter_rate:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub spd_coef:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub str_coef:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub batv:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub temp:Option<i32>,
        

}
impl Msg for HoverboardEvent {
     const ID: u32 = 31340;
     const NAME: &'static str = "HoverboardEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:HoverboardEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct HoverboardCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub steer:Option<i32>,
        

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
pub struct TouchPoint {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub active:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub id:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub x:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub y:Option<i32>,
        

}
impl Msg for TouchPoint {
     const ID: u32 = 49173;
     const NAME: &'static str = "TouchPoint";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:TouchPoint = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct Ps4Event {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_up:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_down:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_square:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_cross:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_circle:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_triangle:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_shoulder:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_shoulder:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_trigger:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_trigger:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_left_joystick:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_right_joystick:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_share:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_options:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_touchpad:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub button_ps:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_lx:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_ly:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_rx:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub axis_ry:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_x:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_y:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub gyro_z:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_x:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_y:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub accel_z:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub connected:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub battery_level:Option<i32>,
        pub touch_points:Vec<TouchPoint>,#[serde(skip_serializing_if = "Option::is_none")]
    pub bluetooth:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub debug:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub temp:Option<i32>,
        

}
impl Msg for Ps4Event {
     const ID: u32 = 29767;
     const NAME: &'static str = "Ps4Event";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:Ps4Event = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct Ps4Cmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rumble_small:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub rumble_large:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led_red:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led_green:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led_blue:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led_flash_on:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led_flash_off:Option<i32>,
        

}
impl Msg for Ps4Cmd {
     const ID: u32 = 50497;
     const NAME: &'static str = "Ps4Cmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:Ps4Cmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct CameraEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub width:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub height:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub format:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub data:Option<Vec<u8>>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub led:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub quality:Option<i32>,
        

}
impl Msg for CameraEvent {
     const ID: u32 = 32617;
     const NAME: &'static str = "CameraEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:CameraEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct CameraCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub led:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub quality:Option<i32>,
        

}
impl Msg for CameraCmd {
     const ID: u32 = 61551;
     const NAME: &'static str = "CameraCmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:CameraCmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LawnmowerManualEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub steering:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub blade:Option<bool>,
        

}
impl Msg for LawnmowerManualEvent {
     const ID: u32 = 24124;
     const NAME: &'static str = "LawnmowerManualEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LawnmowerManualEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LawnmowerManualCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub steer:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub blade:Option<bool>,
        

}
impl Msg for LawnmowerManualCmd {
     const ID: u32 = 1850;
     const NAME: &'static str = "LawnmowerManualCmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LawnmowerManualCmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LawnmowerAutoEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub started:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub stopped:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub paused:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub resumed:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub mode:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub path:Option<String>,
        

}
impl Msg for LawnmowerAutoEvent {
     const ID: u32 = 58665;
     const NAME: &'static str = "LawnmowerAutoEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LawnmowerAutoEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LawnmowerAutoCmd {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub start:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub stop:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub pause:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub resume:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub mode:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub path:Option<String>,
        

}
impl Msg for LawnmowerAutoCmd {
     const ID: u32 = 22063;
     const NAME: &'static str = "LawnmowerAutoCmd";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LawnmowerAutoCmd = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct LawnmowerStatus {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub battery_level:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub blade_status:Option<bool>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub current_mode:Option<String>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub error_message:Option<String>,
        

}
impl Msg for LawnmowerStatus {
     const ID: u32 = 21374;
     const NAME: &'static str = "LawnmowerStatus";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:LawnmowerStatus = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    

#[derive(Debug, Clone,Serialize,Deserialize,Default)]
pub struct MotorEvent {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub motor_id:Option<i32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub temperature:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub voltage:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub current:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub speed:Option<f32>,
        #[serde(skip_serializing_if = "Option::is_none")]
    pub position:Option<f32>,
        

}
impl Msg for MotorEvent {
     const ID: u32 = 55067;
     const NAME: &'static str = "MotorEvent";

    fn serialize(&self) -> Result<Vec<u8>> {
        let s = serde_json::to_vec(self) ?;
        Ok(s)
    }
     
    fn deserialize(v:& Vec<u8>) -> Result<Self> where Self : Sized {
        let m:MotorEvent = serde_json::from_slice(v.as_slice()) ?;
        Ok(m)
        }
}

    
