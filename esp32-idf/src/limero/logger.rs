/*
SemiHosting logger to debug port of probe 
*/


use core::time::Duration;
use std::time::Instant; 
use log::{Level, LevelFilter, Metadata, Record, SetLoggerError};

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
        let ts = std::time::UNIX_EPOCH.elapsed().unwrap().as_millis();
        if self.enabled(record.metadata()) {
            let s = record.args().to_string();
            println!(
                "[{:1.1}] {:6.6} {:10.10}:{:4.4}|{}",
                record.level().as_str(),
                ts,
                record.file().unwrap_or("unknown"),
                record.line().unwrap_or(0),
                s
            );
        }
    }

    fn flush(&self) {}
}