
use core::fmt::Write;

use esp_backtrace as _;
use esp_println::println;
use esp_idf_hal::{
    delay::Delay,
    i2c::*,
    peripherals::Peripherals,
    prelude::*,
};
use heapless::String;
use lsm303agr::Lsm303agr;

// TODO: Re-enable zenoh once zenoh-pico C bindings are properly set up
// use zenoh_pico::{zenoh::Session, ZResult};

/*
// Zenoh bindings disabled until zenoh-pico is properly configured
// include!(concat!(env!("OUT_DIR"), "/zenoh_bindings.rs"));

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
*/

fn main() -> anyhow::Result<()> {
    esp_idf_hal::sys::link_patches();
    
    let peripherals = Peripherals::take()?;

    // Initialize logging
    esp_println::logger::init_logger_from_env();

    let mut delay = Delay::new_default();

    // I2C0 on default pins: GPIO21 (SDA), GPIO22 (SCL) for ESP32
    let sda = peripherals.pins.gpio21;
    let scl = peripherals.pins.gpio22;
    
    let config = I2cConfig::new().baudrate(400.kHz().into());
    let i2c = I2cDriver::new(peripherals.i2c0, sda, scl, &config)?;

    // Initialize LSM303AGR (auto-detects variant)
    let mut lsm303 = Lsm303agr::new_with_i2c(i2c);
    lsm303.init().map_err(|_| anyhow::anyhow!("Failed to initialize LSM303"))?;
    lsm303.set_accel_mode_and_odr(
        &mut delay,
        lsm303agr::AccelMode::Normal,
        lsm303agr::AccelOutputDataRate::Hz50,
    ).map_err(|_| anyhow::anyhow!("Failed to set accel mode"))?;
    lsm303.set_mag_mode_and_odr(
        &mut delay,
        lsm303agr::MagMode::HighResolution,
        lsm303agr::MagOutputDataRate::Hz50,
    ).map_err(|_| anyhow::anyhow!("Failed to set mag mode"))?;

    // Wait for sensor to be ready
    delay.delay_ms(100);

    // TODO: Open Zenoh session once zenoh-pico is properly configured
    // let mut session = open_zenoh_session()?;
    // let key_expr = b"sensors/compass/heading\0"; // Null-terminated C string
    // let mut publisher = z_owned_session_t::default();
    // unsafe {
    //     z_declare_publisher(&mut session, key_expr.as_ptr() as *const _, &mut publisher);
    // }

    let _key_expr = "sensors/compass/heading";

    println!("LSM303 compass publisher running (Zenoh disabled for now)!");

    loop {
        // Check if new accelerometer and magnetometer data is available
        if lsm303.accel_status().ok().map_or(false, |s| s.xyz_new_data()) 
            && lsm303.mag_status().ok().map_or(false, |s| s.xyz_new_data()) 
        {
            if let (Ok(accel), Ok(mag)) = (lsm303.acceleration(), lsm303.magnetic_field()) {
                // Simple tilt-compensated compass (X/Y heading only)
                let heading = tilt_compensated_heading(
                    mag.x_nt() as f32,
                    mag.y_nt() as f32,
                    mag.z_nt() as f32,
                    accel.x_mg() as f32 / 1000.0, // Convert mg to g
                    accel.y_mg() as f32 / 1000.0,
                    accel.z_mg() as f32 / 1000.0,
                );

                // Create JSON payload
                let mut payload: String<128> = String::new();
                write!(
                    &mut payload,
                    r#"{{"heading": {:.1}}}"#,
                    heading
                )
                .ok();

                // Print to console (TODO: publish via Zenoh once configured)
                println!("{}", payload);
                
                // TODO: Publish JSON via Zenoh
                // let payload = b"{\"heading\": 183.2}\0"; // Your JSON as bytes
                // unsafe {
                //     z_publisher_put(
                //         &mut publisher,
                //         payload.as_ptr() as *const _,
                //         payload.len() as usize,
                //     );
                // }
            }
        }

        delay.delay_ms(100);
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

// Wi-Fi configuration is disabled for now (needs esp-wifi crate)
// TODO: Add esp-wifi to dependencies and uncomment to enable WiFi
/*
fn configure_wifi(
    modem: esp_idf_hal::peripherals::MODEM,
    ...
) -> ... {
    use esp_wifi::{wifi::*, EspWifiInitFor};
    // WiFi configuration code here
}
*/

// TODO: Open Zenoh-pico session (scouting mode) once zenoh-pico is configured
// fn open_zenoh_session() -> ZResult<Session> {
//     use zenoh_pico::*;
//
//     let config = zenoh::Config::default()
//         .mode(zenoh::Scouting) // auto-discover Zenoh routers
//         .connect_endpoints(vec![]); // empty = scout
//
//     Session::open(config)
// }
