use anyhow::Result;
use embassy_futures::select::select; // Import the select! macro
use embassy_futures::select::Either::First;
use embassy_futures::select::Either::Second;
use esp_idf_hal::peripherals::*;
use esp_idf_hal::task::*;
use esp_idf_hal::timer::*;
use esp_idf_hal::gpio::PinDriver;

fn main() -> Result<()> {
    esp_idf_hal::sys::link_patches();

    let peripherals = Peripherals::take()?;

    let mut led = PinDriver::output(peripherals.pins.gpio2)?;
    let mut timer = TimerDriver::new(peripherals.timer00, &TimerConfig::new())?;

    block_on(run());

    block_on(async {
        loop {
            led.set_high()?;

            timer.delay(timer.tick_hz()).await?;

            led.set_low()?;

            timer.delay(timer.tick_hz()).await?;
        }
    })
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
