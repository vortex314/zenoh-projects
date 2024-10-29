use anyhow::Ok;
use anyhow::Result;
use embassy_futures::select::select; // Import the select! macro
use embassy_futures::select::Either::First;
use embassy_futures::select::Either::Second;
use esp_idf_hal::peripherals::*;
use esp_idf_hal::task::*;
use esp_idf_hal::timer::*;
use esp_idf_svc::timer::Task;

use embassy_executor::raw::Executor;
use embassy_time as _;

fn main() -> Result<()> {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_hal::sys::link_patches();

    let peripherals = Peripherals::take() ? ;
    let system = peripherals.SYSTEM;
    let clocks = ClockControl::boot_defaults(system.clock_control).freeze();

    let timer_group0 = TimerGroup::new(peripherals.TIMG0, &clocks);
    embassy::init(&clocks, timer_group0.timer0);

    let mut _timer = TimerDriver::new(peripherals.timer00, &TimerConfig::new())?;

    block_on(run());
    Ok(())
}

async fn run() {
    loop {
        match select(async_wait_millis(1000), async_wait_millis(2000)).await {
            First(_) => {
                println!("Tick");
            }
            Second(_) => {
                println!("Tock");
            }
        }
    }
}

pub async fn async_wait_millis(millis: u32) -> () {
    embassy_time::Timer::after_millis(millis as u64).await;
}
