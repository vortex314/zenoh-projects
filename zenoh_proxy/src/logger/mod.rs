use std::io::Write;
use std::thread;

pub fn init() {
    println!("init logger");
    let mut builder = env_logger::Builder::from_default_env();
    builder
        .format(|buf, record| {
            let thread_name = thread::current();
            let name = thread_name.name().unwrap_or("unknown");
            writeln!(
                buf,
                "[{}] {} {:10.10} | {:12.12}:{:3}|  {}",
                chrono::Local::now().format("%H:%M:%S.%3f"),
                record.level(),
                name,
                record.file().unwrap_or("unknown").rsplit_once('/').unwrap().1,
                record.line().unwrap_or(0),
                record.args()
            )
        })
        .filter(None, log::LevelFilter::Info)
        .init();
}