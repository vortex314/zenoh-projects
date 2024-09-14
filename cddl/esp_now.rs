enum MsgType {
    Alive = 0,
    Pub0Req ,
    Pub1Req,
    PingReq,
    NameReq,
    DescReq,
    SetReq,
    GetReq,
}

const fn Resp(t: MsgType) -> u32 {
    (t as u32)| 0x80
}

struct EspNowMessage {
    pub dst: Option<u32>,           // always known , can be broadcast = 0
    pub src: Option<u32>,   // 
    pub msg_type: MsgType,
    pub msg_id: Option<u16>,
    pub payload: Option<Vec<u8>>,
}

const FNV_PRIME: u32 = 16777619;
const FNV_OFFSET: u32 = 2166136261;
const FNV_MASK: u32 = 0xFFFFFFFF;

const fn fnv1(h: u32, s: &str) -> u32 {
    if s.is_empty() {
        h
    } else {
        fnv1((h.wrapping_mul(FNV_PRIME)) ^ s[0] as u32, &s[1..])
    }
}

const fn H(s: &str) -> u32 {
    fnv1(FNV_OFFSET, s)
}

struct Ps4Event {
    left_buttons: u8,
    left_axis_x : i16,
    left_axis_y : i16,
    right_axis_x : i16,
    right_axis_y : i16,
    right_buttons : u8,
    misc_buttons : u8,
}

enum Ps4Props {
    LeftButtons=0,
    LeftAxisX,
    LeftAxisY,
    RightAxisX,
    RightAxisY,
    RightButtons,
    MiscButtons,
    RedLed, // =7
    GreenLed,
    BlueLed,
    Rumble,
}

struct Ps4Cmd {
    BtreeMap<Ps4Props, i16>,
}

enum Ps4Event {
    Connected = 0 ,
    Disconnected,
    Message(Ps4Message),
    Oob,
}