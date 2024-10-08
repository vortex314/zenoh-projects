

use minicbor::{Decode, Encode};

use crate::fnv;


#[derive(Debug, Clone)]
#[repr(i8)]
pub enum Ps4 {
    Dpad=0,

    ButtonSquare,
    ButtonCross ,
    ButtonCircle,  
    ButtonTriangle,

    ButtonLeftShoulder,
    ButtonRightShoulder,

    ButtonLeftTrigger,
    ButtonRightTrigger,

    ButtonLeftJoystick,
    ButtonRightJoystick,

    ButtonShare,
    StickLeftX ,
    StickLeftY ,
    StickRightX ,
    StickRightY ,

    GyroX ,
    GyroY ,
    GyroZ ,

    AccelX ,
    AccelY ,
    AccelZ ,

    Rumble,
    LedGreen,
    LedRed,
    LedBlue,
}

#[derive(Encode,Decode,Default,Debug,Clone)]
#[cbor(map)]
pub struct Ps4Map {
    #[n(0)] pub dpad: Option<i8>,
    #[n(1)] pub button_square: Option<i8>,
    #[n(2)] pub button_cross: Option<i8>,
    #[n(3)] pub button_circle: Option<i8>,
    #[n(4)] pub button_triangle: Option<i8>,
    #[n(5)] pub button_left_shoulder: Option<i8>,
    #[n(6)] pub button_right_shoulder: Option<i8>,
    #[n(7)] pub button_left_trigger: Option<i8>,
    #[n(8)] pub button_right_trigger: Option<i8>,
    #[n(9)] pub button_left_joystick: Option<i8>,
    #[n(10)] pub button_right_joystick: Option<i8>,
    #[n(11)] pub button_share: Option<i8>,
    #[n(12)] pub stick_left_x: Option<i16>,
    #[n(13)] pub stick_left_y: Option<i16>,
    #[n(14)] pub stick_right_x: Option<i16>,
    #[n(15)] pub stick_right_y: Option<i16>,
    #[n(16)] pub gyro_x: Option<i16>,
    #[n(17)] pub gyro_y: Option<i16>,
    #[n(18)] pub gyro_z: Option<i16>,
    #[n(19)] pub accel_x: Option<i16>,
    #[n(20)] pub accel_y: Option<i16>,
    #[n(21)] pub accel_z: Option<i16>,
    #[n(22)] pub rumble: Option<i8>,
    #[n(23)] pub led_green: Option<i8>,
    #[n(24)] pub led_red: Option<i8>,
    #[n(25)] pub led_blue: Option<i8>,
}
 
pub const PS4_ID: u32 = fnv("ps4");


