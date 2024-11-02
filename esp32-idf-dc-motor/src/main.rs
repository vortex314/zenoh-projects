#![no_std]
#![no_main]

extern crate alloc;
use core::time::Duration;

use alloc::format;
use anyhow::Error;
use anyhow::Result;

use edge_executor::LocalExecutor;
use esp_idf_svc::hal::prelude::Peripherals;
use esp_idf_svc::hal::task::block_on;
use esp_idf_svc::hal::timer::TimerConfig;
use esp_idf_svc::hal::timer::TimerDriver;
use esp_idf_svc::timer::EspTimerService;

use log::error;
use log::info;

#[no_mangle]
fn main() {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    //   let local_ex: LocalExecutor = Default::default();

    info!("Starting the main task");

    //  local_ex.spawn(task1());

    //   local_ex.run(fut)

    //   log::info!("Task completed");
    error!("{:?}", block_on(task1()));
}
async fn task1() -> Result<()> {
    let ts = EspTimerService::new().map_err(Error::msg)?;
    let mut timer_async = ts.timer_async().map_err(Error::msg)?;

    loop {
        let heap_size = unsafe { esp_idf_svc::sys::heap_caps_get_free_size(0) };
        log::info!("heap_size = {} bytes", heap_size);
        timer_async.after(Duration::from_secs(1)).await;
    }
}
