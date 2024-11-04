extern crate alloc;
use core::time::Duration;

use anyhow::Error;
use anyhow::Result;

use log::error;
use log::info;

use smol::channel::unbounded;
use smol::Executor;

use msg::LogMsg;
use esp32_limero_std::Actor;

mod esp_now_actor;
use crate::esp_now_actor::EspNowActor;
use esp_idf_svc::timer::EspTimerService;
use futures::executor::block_on;
use esp_idf_svc::espnow::EspNow;
use esp_idf_svc::hal::prelude::Peripherals;
use esp_idf_svc::eventloop::EspSystemEventLoop;
use esp_idf_svc::nvs::EspDefaultNvsPartition;
use esp_idf_svc::wifi::BlockingWifi;
use esp_idf_svc::wifi::EspWifi;

fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    let exec = Executor::new();

    info!("Starting the main task");

    let _ = exec.spawn(task1());

    info!("Starting the 2nd main task");
    let peripherals = Peripherals::take()?;
    let sys_loop = EspSystemEventLoop::take()?;
    let nvs = EspDefaultNvsPartition::take()?;

    let mut wifi = BlockingWifi::wrap(
        EspWifi::new(peripherals.modem, sys_loop.clone(), Some(nvs))?,
        sys_loop,
    )?;


    error!("{:?}", block_on(task1()));

    Ok(())
}
async fn task1() -> Result<()> {
    let ts = EspTimerService::new().map_err(Error::msg)?;
    let mut timer_async = ts.timer_async().map_err(Error::msg)?;
    let (sender, receiver) = unbounded();

    let mut _esp_now_actor = EspNowActor::new()?;

    let mut log_msg = LogMsg::default();
    log_msg.message = "Hello from task1".to_string();

    sender.send(log_msg).await?;
    let _lm = receiver.recv().await?;
    info!("Received message: {}", _lm.message);

    loop {
        let heap_size = unsafe { esp_idf_svc::sys::heap_caps_get_free_size(0) };
        log::info!("heap_size = {} bytes", heap_size);
        timer_async.after(Duration::from_secs(1)).await;
        _esp_now_actor.run().await;
    }
}
