use minicbor::{Decode, Encode};
use minicbor_derive::*;
use std::fmt;

#[derive(Encode, Decode, Default, Debug, Clone)]
#[cbor(map)]
pub struct Ps4Map {
    #[n(0)]
    pub dpad: Option<i8>,
    #[n(1)]
    pub button_square: Option<i8>,
    #[n(2)]
    pub button_cross: Option<i8>,
    #[n(3)]
    pub button_circle: Option<i8>,
    #[n(4)]
    pub button_triangle: Option<i8>,
    #[n(5)]
    pub button_left_shoulder: Option<i8>,
    #[n(6)]
    pub button_right_shoulder: Option<i8>,
    #[n(7)]
    pub button_left_trigger: Option<i8>,
    #[n(8)]
    pub button_right_trigger: Option<i8>,
    #[n(9)]
    pub button_left_joystick: Option<i8>,
    #[n(10)]
    pub button_right_joystick: Option<i8>,
    #[n(11)]
    pub button_share: Option<i8>,
    #[n(12)]
    pub stick_left_x: Option<i32>,
    #[n(13)]
    pub stick_left_y: Option<i32>,
    #[n(14)]
    pub stick_right_x: Option<i32>,
    #[n(15)]
    pub stick_right_y: Option<i32>,
    #[n(16)]
    pub gyro_x: Option<i32>,
    #[n(17)]
    pub gyro_y: Option<i32>,
    #[n(18)]
    pub gyro_z: Option<i32>,
    #[n(19)]
    pub accel_x: Option<i32>,
    #[n(20)]
    pub accel_y: Option<i32>,
    #[n(21)]
    pub accel_z: Option<i32>,
    #[n(22)]
    pub rumble: Option<u32>,
    #[n(23)]
    pub lightbar_rgb: Option<u32>,
    #[n(24)]
    pub connected: Option<bool>,
}

/*

typedef enum {
    OTA_BEGIN =0,
    OTA_END,
    OTA_WRITE,

} OtaOperation;


struct OtaMsg : public Serializable
{
    std::optional<OtaOperation> operation;
    std::optional<uint32_t> offset;
    std::optional<Bytes> image = std::nullopt;
    std::optional<int32_t> rc = std::nullopt;
    std::optional<std::string> message = std::nullopt;
    std::optional<std::string> reply_to = std::nullopt;

    Res serialize(Serializer &ser);
    Res deserialize(Deserializer &des);
};
*/

#[derive(Decode,Encode,Debug, Clone)]
#[cbor(index_only)]
pub enum OtaOperation {
    #[n(0)] OtaBegin = 0,
    #[n(1)] OtaEnd,
    #[n(2)] OtaWrite,
}

#[derive(Encode, Decode, Default, Debug, Clone)]
#[cbor(map)]
pub struct OtaMsg {
    #[n(0)]
    pub operation: Option<OtaOperation>,
    #[n(1)]
    pub offset: Option<u32>,
    #[n(2)]
    pub image: Option<Vec<u8>>,
    #[n(3)]
    pub rc: Option<i32>,
    #[n(4)]
    pub message: Option<String>,
    #[n(5)]
    pub reply_to: Option<String>,
}
