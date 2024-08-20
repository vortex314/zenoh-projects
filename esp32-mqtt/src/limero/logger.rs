/*
SemiHosting logger to debug port of probe 
*/

use alloc::string::ToString;
use embassy_time::Instant;
use esp_println::println;

use log::{Level, LevelFilter, Metadata, Record, SetLoggerError};

pub struct SemiLogger;

pub static SEMI_LOGGER: SemiLogger = SemiLogger;

pub fn semi_logger_init() -> Result<(), SetLoggerError> {
    log::set_logger(&SEMI_LOGGER).map(|()| log::set_max_level(LevelFilter::Info))
}

impl log::Log for SemiLogger {
    fn enabled(&self, metadata: &Metadata<'_>) -> bool {
        metadata.level() >= Level::Info
    }

    fn log(&self, record: &Record<'_>) {
        if self.enabled(record.metadata()) {
            let s = record.args().to_string();
            println!(
                "[{:1.1}] {:6.6} {:10.10}:{:4.4}|{}",
                record.level().as_str(),
                Instant::now().as_millis(),
                record.file().unwrap_or("unknown"),
                record.line().unwrap_or(0),
                s
            );
        }
    }

    fn flush(&self) {}
}