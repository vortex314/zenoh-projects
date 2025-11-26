
use core::fmt::Write;
use std::panic;

use embedded_hal::blocking::delay::DelayUs;
use esp_backtrace as _;
use esp_println::println;
use hal::{
    clock::{ClockControl, CpuClock},
    i2c::I2C,
    peripherals::Peripherals,
    prelude::*,
    timer::TimerGroup,
    Delay, Rtc,
};
use heapless::String;
use lsm303agr::{AccelOutput, Lsm303agr, MagOutput};
use nb::block;
use zenoh_pico::{zenoh::Session, ZResult};

type Lsm303 = Lsm303agr<hal::i2c::I2C<'static, hal::peripherals::I2C0>>;
// At top of main.rs, after other uses
include!(concat!(env!("OUT_DIR"), "/zenoh_bindings.rs"));

// Example usage (replace the old zenoh_session code)
use core::ffi::CStr; // For string handling

fn open_zenoh_session() -> Result<z_owned_session_t, i32> {
    // Config: UDP multicast scouting (for ESP32 Wi-Fi)
    let mut config = z_config_default();
    z_config_insert_json(
        z_config_from_ptr(&mut config),
        CStr::from_bytes_with_nul(b"mode\0").unwrap().as_ptr(),
        CStr::from_bytes_with_nul(b"{\"mode\":\"peer\"}\0")
            .unwrap()
            .as_ptr(),
    );
    z_config_insert_json(
        z_config_from_ptr(&mut config),
        CStr::from_bytes_with_nul(b"connect/timeout\0")
            .unwrap()
            .as_ptr(),
        CStr::from_bytes_with_nul(b"500\0").unwrap().as_ptr(),
    ); // ms

    // Open session
    let mut session = z_owned_session_t::default();
    let ret = unsafe { z_open(&mut session, z_move(config)) };
    if ret == 0 {
        Ok(session)
    } else {
        Err(-1)
    }
}

#[entry]
fn main() -> ! {
    let peripherals = Peripherals::take().unwrap();
    let system = peripherals.SYSTEM.split();
    let clocks = ClockControl::configure(system.clock_control, CpuClock::Clock160MHz).freeze();

    // Initialize logging
    esp_println::logger::init_logger_from_env();

    // Fast RTC for low power (not used here but good practice)
    let mut rtc = Rtc::new(peripherals.RTC_CNTL);
    let mut delay = Delay::new(&clocks);

    // Timer Group0 is used for WiFi/BT
    let timer_group0 = TimerGroup::new(peripherals.TIMG0, &clocks);
    let mut timer_group1 = TimerGroup::new(peripherals.TIMG1, &clocks);

    // Configure Wi-Fi
    let wifi = configure_wifi(peripherals.MODEM, system.radio_clock_control, &clocks);

    // I2C0 on default pins: GPIO21 (SDA), GPIO22 (SCL) for ESP32
    let i2c = I2C::new(
        peripherals.I2C0,
        hal::gpio::GpioPin::<hal::gpio::InputOutput, 21>, // SDA
        hal::gpio::GpioPin::<hal::gpio::InputOutput, 22>, // SCL
        400u32.kHz(),
        &clocks,
    );

    // Initialize LSM303AGR (auto-detects variant)
    let mut lsm303 = Lsm303agr::new_with_i2c(i2c).expect("Failed to init LSM303");
    lsm303.set_accel_mode(lsm303agr::Mode::HighResolution).ok();
    lsm303.set_mag_rate(lsm303agr::MagOutputDataRate::Hz50).ok();
    lsm303
        .set_accel_rate(lsm303agr::AccelOutputDataRate::Hz50)
        .ok();

    // Wait for sensor to be ready
    delay.delay_ms(100u32);

    // Open Zenoh session (peer-to-peer mode, connects to router if available)
    // In your loop (put example):
    let mut session = open_zenoh_session()?;
    let key_expr = b"sensors/compass/heading\0"; // Null-terminated C string
    let mut publisher = z_owned_publisher_t::default();
    unsafe {
        z_declare_publisher(&mut session, key_expr.as_ptr() as *const _, &mut publisher);
    }

    let key_expr = "sensors/compass/heading";

    println!("LSM303 + Zenoh compass publisher running!");

    loop {
        if lsm303.acceleration_status().unwrap().xyz_new_data()
            && lsm303.magnetometer_status().unwrap().xyz_new_data()
        {
            let accel = lsm303.accel_data().unwrap();
            let mag = lsm303.mag_data().unwrap();

            // Simple tilt-compensated compass (X/Y heading only)
            let heading = tilt_compensated_heading(
                mag.x as f32,
                mag.y as f32,
                mag.z as f32,
                accel.x as f32,
                accel.y as f32,
                accel.z as f32,
            );

            // Create JSON payload
            let mut payload: String<128> = String::new();
            write!(
                &mut payload,
                r#"{{"heading": {:.1}, "timestamp": {}}}"#,
                heading,
                esp_hal::time::current_time().as_millis()
            )
            .unwrap();

            // Publish JSON
            let payload = b"{\"heading\": 183.2}\0"; // Your JSON as bytes
            unsafe {
                z_publisher_put(
                    &mut publisher,
                    payload.as_ptr() as *const _,
                    payload.len() as usize,
                );
            }
        }

        delay.delay_ms(100u32);
    }
}

// Simple tilt-compensated compass heading in degrees
fn tilt_compensated_heading(mx: f32, my: f32, mz: f32, ax: f32, ay: f32, az: f32) -> f32 {
    // Normalize accelerometer
    let norm = (ax * ax + ay * ay + az * az).sqrt();
    if norm == 0.0 {
        return 0.0;
    }
    let ax = ax / norm;
    let ay = ay / norm;
    let az = az / norm;

    // Tilt-compensated magnetic field
    let xh = my * az - mz * ay;
    let yh = mz * ax - mx * az;
    // let zh = mx * ay - my * ax; // not needed for heading

    let mut heading = xh.atan2(yh) * 180.0 / core::f32::consts::PI;

    // Convert to 0–360°
    if heading < 0.0 {
        heading += 360.0;
    }

    heading
}

// Wi-Fi configuration (replace with your SSID/password)
fn configure_wifi(
    modem: hal::peripherals::MODEM,
    radio_clock_control: hal::radio_clock::RadioClockControl,
    clocks: &hal::clock::Clocks,
) -> esp_wifi::wifi::WifiStaDevice {
    use esp_wifi::{wifi::*, EspWifiInitFor};

    let timer = TimerGroup::new(peripherals.TIMG0, clocks).timer0;
    let init =
        esp_wifi::initialize(EspWifiInitFor::Wifi, timer, radio_clock_control, clocks).unwrap();

    let wifi = peripherals.WIFI;
    let mut wifi = WifiController::new(esp_wifi::wifi::WifiDriver::new(modem, wifi, init).unwrap());

    let config = Configuration::Client(ClientConfiguration {
        ssid: "YOUR_SSID".into(),
        password: "YOUR_PASSWORD".into(),
        ..Default::default()
    });

    wifi.set_configuration(&config).unwrap();
    wifi.start().unwrap();
    wifi.connect().unwrap();

    println!("Waiting for Wi-Fi association...");
    while !wifi.is_connected().unwrap() {
        let status = wifi.get_status();
        println!("Status: {:?}", status);
        hal::delay::Delay::new(clocks).delay_ms(500u32);
    }
    println!("Wi-Fi connected!");

    wifi.sta_device()
}

// Open Zenoh-pico session (scouting mode)
fn open_zenoh_session() -> ZResult<Session> {
    use zenoh_pico::*;

    let config = zenoh::Config::default()
        .mode(zenoh::Scouting) // auto-discover Zenoh routers
        .connect_endpoints(vec![]); // empty = scout

    Session::open(config)
}
