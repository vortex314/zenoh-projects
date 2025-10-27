

use minicbor::{Decode, Encode};

use crate::fnv;


#[derive(Debug, Clone)]
#[repr(i8)]
pub enum Sys {
    DeviceName=0,
    Build,
    Mac,
    Ip,
    Hostname,
    Build,
    Version,
    Uptime,
    FreeHeap,
    CpuFree,

    
}

use crate::InfoMap;



#[derive(Encode,Decode,Default,Debug,Clone)]
#[cbor(map)]
pub struct SysMap {
    #[n(0)] pub device_name: Option<String>,
    #[n(1)] pub build : Option<String>,
}

const SYS_PROPS:[InfoMap] = [
    InfoMap { id: 0, name: Some("device_name"), desc: Some("Device Name"), prop_type: PropType::STR, prop_mode: PropMode::Read .. Default()}    
    InfoMap { id: 1, name: Some("build"), desc: Some("Build"), prop_type: PropType::STR, prop_mode: PropMode::Read .. Default()}
];
 
pub const SYS_ID: u32 = fnv("sys");


