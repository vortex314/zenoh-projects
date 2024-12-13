/*
SemiHosting logger to debug port of probe
*/

use std::time::SystemTime;

use log::{Level, LevelFilter, Metadata, Record, SetLoggerError};

pub struct LimeroLogger {}

pub static LIMERO_LOGGER: LimeroLogger = LimeroLogger {};

pub fn limero_logger_init() -> Result<(), SetLoggerError> {
    let _res = log::set_logger(&LIMERO_LOGGER).map(|()| log::set_max_level(LevelFilter::Info));
    _res
}

impl log::Log for LimeroLogger {
    fn enabled(&self, metadata: &Metadata<'_>) -> bool {
        metadata.level() >= Level::Info
    }

    fn log(&self, record: &Record<'_>) {
        let ts = SystemTime::now()
            .duration_since(SystemTime::UNIX_EPOCH)
            .unwrap_or(std::time::Duration::from_secs(0))
            .as_millis();
        if self.enabled(record.metadata()) {
            let s = record.args().to_string();
            let file = record
                .file()
                .and_then(|file| file.rsplit_once("/"))
                .map(|(_, b)| b)
                .unwrap_or("");
            println!(
                "[{:1.1}] {:6.6}|{:15.15}:{:4.4}|{}",
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
