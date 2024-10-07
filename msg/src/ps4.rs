

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

pub const PS4_ID: u32 = fnv("ps4");


