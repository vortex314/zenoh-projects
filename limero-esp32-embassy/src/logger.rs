/*
SemiHosting logger to debug port of probe 
*/


use embassy_time::Duration;
use embassy_time::Instant;

use log::{Level, LevelFilter, Metadata, Record, SetLoggerError};
use esp_println::println;
use crate::alloc::string::ToString;
pub struct SemiLogger;

pub static SEMI_LOGGER: SemiLogger = SemiLogger;

pub fn semi_logger_init() -> Result<(), SetLoggerError> {
    println!("semi_logger_init called");
    let _res  = log::set_logger(&SEMI_LOGGER).map(|()| log::set_max_level(LevelFilter::Info));
    println!("semi_logger_init done {:?}",_res);
    _res
}

impl log::Log for SemiLogger {
    fn enabled(&self, metadata: &Metadata<'_>) -> bool {
        metadata.level() >= Level::Info
    }

    fn log(&self, record: &Record<'_>) {
        let ts = Instant::now().as_millis();
        if self.enabled(record.metadata()) {
            let s = record.args().to_string();
         let (_,file) = record.file().unwrap_or("/").rsplit_once("/").unwrap_or(("/","/"));
            println!(
                "[{:1.1}] {:6.6} {:15.15}:{:4.4}|{}",
                record.level().as_str(),
                ts,
                file,
                record.line().unwrap_or(0),
                s
            );
        }
    }

    fn flush(&self) {}
}