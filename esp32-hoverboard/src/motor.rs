
use alloc::vec::Vec;
pub const START_FRAME: u16 = 0xABCD;

#[derive(Debug, Clone)]
pub struct MotorCmd {
    pub speed: i16,
    pub steer: i16,
}

impl MotorCmd {
    pub fn encode(&self) -> Vec<u8> {
        let mut v = Vec::new();
        v.push((START_FRAME & 0xFF) as u8);
        v.push((START_FRAME >> 8) as u8);
        v.push((self.steer & 0xFF) as u8);
        v.push((self.steer >> 8) as u8);
        v.push((self.speed & 0xFF) as u8);
        v.push((self.speed >> 8) as u8);
        self.add_crc(&mut v);
        v
    }

    fn crc(&self) -> u16 {
        let mut crc = 0;
        crc = crc ^ (START_FRAME) as u16;
        crc = crc ^ self.steer as u16;
        crc = crc ^ self.speed as u16;
        crc
    }

    fn add_crc(&self, vec: &mut Vec<u8>) {
        let crc = self.crc();
        vec.push((crc & 0xFF) as u8);
        vec.push((crc >> 8) as u8);
    }
}
