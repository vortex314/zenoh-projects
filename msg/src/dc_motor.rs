use minicbor::{Decode, Encode};

use crate::fnv;


#[derive(Encode,Decode,Default,Debug,Clone)]
#[cbor(map)]
pub struct DcMotorMap {
    #[n(0)] pub target_rpm: Option<i16>,
    #[n(1)] pub measured_rpm: Option<i16>,
    #[n(2)] pub current: Option<f32>,

}

pub const DC_MOTOR_ID : u32 = fnv("dc_motor");