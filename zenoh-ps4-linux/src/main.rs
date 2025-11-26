// src/main.rs
use hidapi::{HidApi, HidDevice};
use serde::{Deserialize, Serialize};
use std::time::Duration;
use zenoh::*;

mod limero;
mod logger;
use log::info;
use log::warn;

use limero::Ps4Cmd;
use limero::Ps4Event;

pub trait MapEvent<EI, EO> {
    fn map(input: &EI) -> Option<EO>;
}

#[derive(Serialize, Deserialize, Debug, Clone)]
struct TouchPoint {
    active: bool,
    id: u8,
    x: u16,
    y: u16,
}

#[derive(Serialize, Deserialize, Debug)]
struct ControllerCommand {
    rumble_small: u8, // 0–255
    rumble_large: u8, // 0–255
    led_r: u8,        // 0–255
    led_g: u8,
    led_b: u8,
    flash_on: u8,  // 0–255 (duration of "on" in 25ms units)
    flash_off: u8, // 0–255 (duration of "off" in 25ms units)
}

const VID: u16 = 0x054C;
const PID_V1: u16 = 0x05C4; // DS4 v1 (USB)
const PID_V2: u16 = 0x09CC; // DS4 v2 (USB/Bluetooth)

fn stringify(x: Box<dyn std::error::Error + Send + Sync>) -> String {
    format!("error code: {x}")
}

#[tokio::main]
async fn main() -> Result<()> {
    logger::init();
    // Open Zenoh session?
    let session = zenoh::open(zenoh::Config::default())
        .await
        .map_err(stringify)?;
    info!("Zenoh session opened");

    let (mut cmd_sender, mut cmd_receiver) = tokio::sync::mpsc::unbounded_channel::<Ps4Cmd>();

    // Zenoh topics
    let event_topic = "src/brain/ps4/Ps4Event";
    let command_topic = "dst/brain/ps4/Ps4Cmd";
    let event_publisher = session.declare_publisher(event_topic).await?;
    let command_subscriber = session.declare_subscriber(command_topic).await?;
    // HID setup
    let api = HidApi::new()?;
    let device = find_ps4_controller(&api)?;
    info!(
        "PS4 Controller found: {}",
        device
            .get_product_string()?
            .unwrap_or("Unknown".to_string())
    );

    // For Bluetooth we need output report 0x05 (USB) or 0x11 (Bluetooth with CRC)
    // We'll detect mode by checking report ID
    let mut is_bluetooth = false;

    // Command receiver task

    tokio::spawn(async move {
        while let Ok(sample) = command_subscriber.recv_async().await {
            info!("Received command");
            let x = sample.payload().to_bytes();
            if let Ok(cmd) = serde_json::from_slice::<Ps4Cmd>(&x) {
                cmd_sender.send(cmd.clone()).unwrap();
            } else {
                warn!("Failed to parse command");
            }
        }
    });

    // Main loop: read input reports and publish states
    let mut buf = [0u8; 64];
    let mut ps4_prev = Ps4Event::default();
    let mut counter = 0;
    loop {
        // in recv loop check for commands
        if let Ok(cmd) = cmd_receiver.try_recv() {
            send_command(&device, &cmd, is_bluetooth)
                .unwrap_or_else(|e| warn!("Send error: {}", e));
        }
        match device.read_timeout(&mut buf, 100) {
            Ok(n) if n > 0 => {
                if buf[0] == 0x01 || buf[0] == 0x11 {
                    // Valid input report
                    is_bluetooth = buf[0] == 0x11;

                    counter += 1;
                    if counter % 50 == 0 {
                        let state = parse_input_report(&buf, is_bluetooth);
                        let changes = find_changes(&ps4_prev, &state);
                        ps4_prev = state.clone();
                        let json = serde_json::to_string(&state).unwrap();

                        event_publisher
                            .put(json)
                            .await
                            .unwrap_or_else(|e| warn!("Publish error: {}", e));
                    }
                }
            }
            Ok(_) => std::thread::sleep(Duration::from_millis(4)),
            Err(e) => {
                warn!("Read error: {}", e);
                break;
            }
        }
    }

    Ok(())
}

fn find_ps4_controller(api: &HidApi) -> anyhow::Result<HidDevice> {
    for info in api.device_list() {
        if info.vendor_id() == VID && (info.product_id() == PID_V1 || info.product_id() == PID_V2) {
            return Ok(api.open(VID, info.product_id())?);
        }
    }
    anyhow::bail!("PS4 Controller not found. Is it connected?");
}

fn find_changes(prev: &Ps4Event, current: &Ps4Event) -> Ps4Event {
    let mut changes = Ps4Event::default();

    if prev.accel_x != current.accel_x {
        changes.accel_x = current.accel_x;
    }
    if prev.accel_y != current.accel_y {
        changes.accel_y = current.accel_y;
    }
    if prev.accel_z != current.accel_z {
        changes.accel_z = current.accel_z;
    }
    if prev.gyro_x != current.gyro_x {
        changes.gyro_x = current.gyro_x;
    }
    if prev.gyro_y != current.gyro_y {
        changes.gyro_y = current.gyro_y;
    }
    if prev.gyro_z != current.gyro_z {
        changes.gyro_z = current.gyro_z;
    }
    if prev.axis_lx != current.axis_lx {
        changes.axis_lx = current.axis_lx;
    }
    if prev.axis_ly != current.axis_ly {
        changes.axis_ly = current.axis_ly;
    }
    if prev.axis_rx != current.axis_rx {
        changes.axis_rx = current.axis_rx;
    }
    if prev.axis_ry != current.axis_ry {
        changes.axis_ry = current.axis_ry;
    }
    if Some(true) == current.button_up {
        changes.button_up = current.button_up;
    }
    if Some(true) == current.button_down {
        changes.button_down = current.button_down;
    }
    if Some(true) == current.button_left {
        changes.button_left = current.button_left;
    }
    if Some(true) == current.button_right {
        changes.button_right = current.button_right;
    }
    if Some(true) == current.button_circle {
        changes.button_circle = current.button_circle;
    }
    if Some(true) == current.button_cross {
        changes.button_cross = current.button_cross;
    }
    if Some(true) == current.button_square {
        changes.button_square = current.button_square;
    }
    if Some(true) == current.button_triangle {
        changes.button_triangle = current.button_triangle;
    }
    if Some(true) == current.button_left_trigger {
        changes.button_left_trigger = current.button_left_trigger;
    }
    if Some(true) == current.button_right_trigger {
        changes.button_right_trigger = current.button_right_trigger;
    }
    if Some(true) == current.button_left_shoulder {
        changes.button_left_shoulder = current.button_left_shoulder;
    }
    if Some(true) == current.button_right_shoulder {
        changes.button_right_shoulder = current.button_right_shoulder;
    }
    if Some(true) == current.button_options {
        changes.button_options = current.button_options;
    }
    if Some(true) == current.button_share {
        changes.button_share = current.button_share;
    }
    if prev.battery_level != current.battery_level {
        changes.battery_level = current.battery_level;
    }
    if Some(true) == current.button_left_joystick {
        changes.button_left_joystick = current.button_left_joystick;
    }
    if Some(true) == current.button_right_joystick {
        changes.button_right_joystick = current.button_right_joystick;
    }
    // Add other fields as needed
    //  changes.debug = current.debug.clone();
    changes.temp = current.temp;
    if Some(0) != current.axis_lx {
        changes.axis_lx = current.axis_lx;
    }
    if Some(0) != current.axis_ly {
        changes.axis_ly = current.axis_ly;
    }
    if Some(0) != current.axis_rx {
        changes.axis_rx = current.axis_rx;
    }
    if Some(0) != current.axis_ry {
        changes.axis_ry = current.axis_ry;
    }
    changes
}

/*
Buffer description for PS4 controller input report:
https://www.psdevwiki.com/ps4/DS4-USB

*/

fn parse_input_report(buf: &[u8], bluetooth: bool) -> Ps4Event {
    let offset = if bluetooth { 2 } else { 0 }; // Bluetooth has 2-byte header

    // D-pad is encoded as a hat value in lower 4 bits of byte 5
    let dpad_hat = buf[5 + offset] & 0x0F;
    let right_buttons = buf[5 + offset] & 0xF0;

    let mut ps4_info = Ps4Event::default();
    ps4_info.bluetooth = Some(bluetooth);

    ps4_info.accel_x = Some(i16::from_le_bytes([buf[19 + offset], buf[20 + offset]]) as i32);
    ps4_info.accel_y = Some(i16::from_le_bytes([buf[21 + offset], buf[22 + offset]]) as i32);
    ps4_info.accel_z = Some(i16::from_le_bytes([buf[23 + offset], buf[24 + offset]]) as i32);

    ps4_info.gyro_x = Some(i16::from_le_bytes([buf[13 + offset], buf[14 + offset]]) as i32);
    ps4_info.gyro_y = Some(i16::from_le_bytes([buf[15 + offset], buf[16 + offset]]) as i32);
    ps4_info.gyro_z = Some(i16::from_le_bytes([buf[17 + offset], buf[18 + offset]]) as i32);

    ps4_info.axis_lx = Some(buf[1 + offset] as i32 - 128);
    ps4_info.axis_ly = Some(-(buf[2 + offset] as i32 - 128));
    ps4_info.axis_rx = Some(buf[3 + offset] as i32 - 128);
    ps4_info.axis_ry = Some(-(buf[4 + offset] as i32 - 128));

    // D-pad: hat value 0-7 = directions, 8 or 15 = neutral
    ps4_info.button_up = Some(dpad_hat == 0 || dpad_hat == 1 || dpad_hat == 7);
    ps4_info.button_down = Some(dpad_hat == 3 || dpad_hat == 4 || dpad_hat == 5);
    ps4_info.button_left = Some(dpad_hat == 5 || dpad_hat == 6 || dpad_hat == 7);
    ps4_info.button_right = Some(dpad_hat == 1 || dpad_hat == 2 || dpad_hat == 3);

    // Face buttons (upper bits)
    ps4_info.debug = Some(format!(
        "{:10} {:10} {:10} ",
        ps4_info.gyro_x.unwrap(),
        ps4_info.gyro_y.unwrap(),
        ps4_info.gyro_z.unwrap()
    ));

    ps4_info.button_circle = Some((right_buttons & 0x40) != 0);
    ps4_info.button_cross = Some((right_buttons & 0x20) != 0);
    ps4_info.button_square = Some((right_buttons & 0x10) != 0);
    ps4_info.button_triangle = Some((right_buttons & 0x80) != 0);

    ps4_info.button_left_trigger = Some((buf[8 + offset] & 0xff) != 0);
    ps4_info.button_right_trigger = Some((buf[9 + offset] & 0xff) != 0);

    ps4_info.button_left_shoulder = Some((buf[6 + offset] & 0x01) != 0);
    ps4_info.button_right_shoulder = Some((buf[6 + offset] & 0x02) != 0);

    ps4_info.button_options = Some((buf[6 + offset] & 0x20) != 0);
    ps4_info.button_share = Some((buf[6 + offset] & 0x10) != 0);
    ps4_info.button_touchpad = Some((buf[7 + offset] & 0x02) != 0);
    ps4_info.button_ps = Some((buf[7 + offset] & 0x01) != 0);

    ps4_info.button_left_joystick = Some((buf[6 + offset] & 0x40) != 0);
    ps4_info.button_right_joystick = Some((buf[6 + offset] & 0x80) != 0);

    ps4_info.battery_level = Some((buf[30 + offset] & 0x0F) as i32);
    ps4_info.temp = Some(buf[12 + offset] as i32); // Temperature sensor value

    ps4_info
}

fn parse_touchpad(data: &[u8], bluetooth: bool) -> Vec<TouchPoint> {
    let mut points = Vec::new();
    let count = if bluetooth { 3 } else { 2 };
    let mut offset = 0;

    for _ in 0..count {
        let active = (data[offset] & 0x80) == 0;
        let id = data[offset] & 0x7F;
        let x = ((data[offset + 2] as u16) << 4) | ((data[offset + 1] as u16) >> 4);
        let y = ((data[offset + 3] as u16) << 4) | ((data[offset + 2] as u16) & 0x0F);

        points.push(TouchPoint { active, id, x, y });
        offset += if bluetooth { 9 } else { 9 };
    }
    points
}

fn send_command(device: &HidDevice, cmd: &Ps4Cmd, bluetooth: bool) -> anyhow::Result<()> {
    info!(
        "Command to send: rumble_small={}, rumble_large={}, led_rgb=({}, {}, {}), led_flash_on={}, led_flash_off={}",
        cmd.rumble_small.unwrap_or(0),
        cmd.rumble_large.unwrap_or(0),
        cmd.led_red.unwrap_or_else(|| 0),
        cmd.led_green.unwrap_or_else(|| 0),
        cmd.led_blue.unwrap_or_else(|| 0),
        cmd.led_flash_on.unwrap_or_else(|| 0),
        cmd.led_flash_off.unwrap_or_else(|| 0),
    );

    if bluetooth {
        // Bluetooth: 78-byte report (HID BT header + 74-byte payload + CRC)
        let mut report = [0u8; 78];
        report[0] = 0xA2; // HID BT output
        report[1] = 0x11; // Report ID for output
        report[2] = 0xC0; // Unknown/required
        report[3] = 0x20; // Unknown/required
        report[4] = 0x07; // Valid flag (enables rumble/LED)

        // Control byte (byte 5): Enable rumble/LED updates
        report[5] = 0x01; // Bit 0: Rumble enable, Bit 1: LED enable (set both)

        // Runtime data (bytes 6-9): Rumble + timestamp (low byte)
        report[6] = cmd.rumble_small.unwrap_or(0) as u8;
        report[7] = cmd.rumble_large.unwrap_or(0) as u8;
        report[8] = 0x00; // Timestamp low (fixed for simplicity)
        report[9] = 0x00; // Timestamp high

        // LED data (bytes 10-16): RGB + brightness + blink
        report[10] = cmd.led_red.unwrap_or(0) as u8;
        report[11] = cmd.led_green.unwrap_or(0) as u8;
        report[12] = cmd.led_blue.unwrap_or(0) as u8;
        report[13] = 0x00; // LED brightness (0x00 = full; adjust if dimming needed)
        report[14] = cmd.led_flash_on.unwrap_or(0) as u8; // Blink on duration
        report[15] = cmd.led_flash_off.unwrap_or(0) as u8; // Blink off duration
        report[16] = 0x00; // LED delay? (fixed)

        // Pad the rest with zeros (required for valid report)
        for i in 17..74 {
            report[i] = 0x00;
        }

        // CRC32 over bytes 0-73
        let crc_bytes = crc32(&report[0..74]);
        report[74..78].copy_from_slice(&crc_bytes.to_le_bytes());

        println!("BT Report sent: {:?}", &report[0..20]); // Debug: first 20 bytes
        device.write(&report)?;
    } else {
        // USB: 32-byte report (report ID + 31-byte payload)
        let mut report = [0u8; 32];
        report[0] = 0x05; // Report ID

        // Control byte (byte 1): Enable rumble/LED updates
        let mut v: u8 = 0x00;
        if cmd.rumble_small.unwrap_or(0) > 0 || cmd.rumble_large.unwrap_or(0) > 0 {
            v |= 0x01; // Enable rumble
        }
        if cmd.led_red.unwrap_or(0) > 0
            || cmd.led_green.unwrap_or(0) > 0
            || cmd.led_blue.unwrap_or(0) > 0
        {
            v |= 0x02; // Enable LED
        }
        v |= 0x04; // Always set bit 2
        report[1] = v;
        report[2] = 0x00; // Unknown/required
        // Rumble (bytes 2-3)
        report[3] = cmd.rumble_small.unwrap_or(0) as u8;
        report[4] = cmd.rumble_large.unwrap_or(0) as u8;
        // LED data (bytes 4-10): RGB + blink (note: offset shifted vs BT)
        report[6] = cmd.led_red.unwrap_or(0) as u8;
        report[7] = cmd.led_green.unwrap_or(0) as u8;
        report[8] = cmd.led_blue.unwrap_or(0) as u8;
        report[9] = 0x00; // LED brightness (full)
        report[10] = cmd.led_flash_on.unwrap_or(0) as u8;
        report[11] = cmd.led_flash_off.unwrap_or(0) as u8;
        report[12] = 0x00; // Delay?
        // Pad the rest with zeros (required)
        for i in 13..32 {
            report[i] = 0x00;
        }

        println!("USB Report sent: {:?}", &report); // Debug: full report
        device.write(&report)?;
    }

    Ok(())
}

// Sony's CRC32 for DS4 Bluetooth reports
fn crc32(data: &[u8]) -> u32 {
    let mut crc: u32 = 0xFFFFFFFF;
    let poly: u32 = 0x04C11DB7;

    for &b in data {
        crc ^= (b as u32) << 24;
        for _ in 0..8 {
            if (crc & 0x80000000) != 0 {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }
    !crc
}
