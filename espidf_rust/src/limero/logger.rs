use core::str::FromStr;

use embassy_time::Instant;
use esp_println::println;
use log::LevelFilter;

pub fn init_logger(level: log::LevelFilter) {
    unsafe {
        log::set_logger_racy(&LimeroLogger {}).unwrap();
        log::set_max_level_racy(level);
    }
}


struct LimeroLogger;

impl log::Log for LimeroLogger {
    fn enabled(&self, _metadata: &log::Metadata) -> bool {
        true
    }

    #[allow(unused)]
    fn log(&self, record: &log::Record) {
        // check enabled log targets if any

        const RESET: &str = "\u{001B}[0m";
        const RED: &str = "\u{001B}[31m";
        const GREEN: &str = "\u{001B}[32m";
        const YELLOW: &str = "\u{001B}[33m";
        const BLUE: &str = "\u{001B}[34m";
        const CYAN: &str = "\u{001B}[35m";

        #[cfg(feature = "colors")]
        let color = match record.level() {
            log::Level::Error => RED,
            log::Level::Warn => YELLOW,
            log::Level::Info => GREEN,
            log::Level::Debug => BLUE,
            log::Level::Trace => CYAN,
        };
        #[cfg(feature = "colors")]
        let reset = RESET;

        #[cfg(not(feature = "colors"))]
        let color = "";
        #[cfg(not(feature = "colors"))]
        let reset = "";
        let ts = Instant::now().as_millis();
        let sec = ts / 1000;
        let ms = ts % 1000;
        let min = sec / 60;
        let sec = sec % 60;
        let hour = min / 60;
        let min = min % 60;

        println!(
            "{:02}:{:02}:{:02}.{:03} {}{:5}{} {:>15}-{:3} {}",
            hour,
            min,
            sec,
            ms,
            color,
            record.level(),
            reset,
            record.file().unwrap_or(""),
            record.line().unwrap_or(0),
            record.args(),
        );
    }

    fn flush(&self) {}
}
