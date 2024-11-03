extern crate alloc;
use core::time::Duration;
use std::f128::consts::E;

use anyhow::Error;
use anyhow::Result;

use esp_idf_svc::hal::prelude::Peripherals;
use esp_idf_svc::hal::task::block_on;
use esp_idf_svc::hal::timer::TimerConfig;
use esp_idf_svc::hal::timer::TimerDriver;
use esp_idf_svc::timer::EspTimerService;

use log::error;
use log::info;

use smol::channel::unbounded;
use smol::Executor;

use msg::LogMsg;

mod esp_now_actor;


fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    let exec = Executor::new();

    info!("Starting the main task");

    let _ = exec.spawn(task1());

    info!("Starting the 2nd main task");

    error!("{:?}", block_on(task1()));

    Ok(())
}
async fn task1() -> Result<()> {
    let ts = EspTimerService::new().map_err(Error::msg)?;
    let mut timer_async = ts.timer_async().map_err(Error::msg)?;
    let (sender, receiver) = unbounded();

    let esp_now_actor = EspNowActor::new();

    let mut log_msg = LogMsg::default();
    log_msg.message = "Hello from task1".to_string();

    sender.send(log_msg).await?;
    let _lm = receiver.recv().await?;
    info!("Received message: {}", _lm.message);

    loop {
        let heap_size = unsafe { esp_idf_svc::sys::heap_caps_get_free_size(0) };
        log::info!("heap_size = {} bytes", heap_size);
        timer_async.after(Duration::from_secs(1)).await;
    }
}
