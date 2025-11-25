// src/main.rs
use gilrs::{Button, Event, EventType, Gilrs};
use log::warn;
use serde::Serialize;
use std::time::Duration;
use zenoh::Result as ZResult;
use zenoh::Session;
use zenoh::bytes::ZBytes;

mod limero;
use limero::Ps4Cmd;
use limero::Ps4Info;
mod logger;
use hidapi::HidApi;
use log::info;

#[derive(Serialize, Debug)]
struct ControllerState {
    left_stick_x: f32,
    left_stick_y: f32,
    right_stick_x: f32,
    right_stick_y: f32,
    l2_trigger: f32, // 0.0 to 1.0
    r2_trigger: f32, // 0.0 to 1.0
    dpad_x: i8,      // -1 left, 0 neutral, 1 right
    dpad_y: i8,      // -1 down, 0 neutral, 1 up
    buttons: Vec<String>,
    connected: bool,
}

#[tokio::main]
async fn main() -> ZResult<()> {
    logger::init();
    unsafe {
        std::env::set_var("HIDAPI_SKIP_SYSTEM_DEVICES", "0");
    }

    match HidApi::new() {
        Ok(api) => {
            for dev in api.device_list() {
                    info!(
                        "{:04x}:{:04x}  {}  {}",
                        dev.vendor_id(),
                        dev.product_id(),
                        dev.manufacturer_string().unwrap_or(""),
                        dev.product_string().unwrap_or("")
                    );
            }
        }
        Err(e) => warn!("HidApi init failed: {}", e),
    }
    info!("going to sleep for a long time");
    tokio::time::sleep(Duration::from_millis(16000000000000)).await;

    // Open Zenoh session (auto-peers with local router or cloud if configured)
    let session = zenoh::open(zenoh::Config::default()).await?;
    info!("Zenoh session opened");

    // Key expression to publish to (e.g. "controller/ps4/1")
    let key_expr = "src/ps4/Ps4Info";

    let mut gilrs = Gilrs::new().unwrap();
    info!("Looking for PS4 controller...");

    // Wait for at least one gamepad
    loop {
        if let Some((_id, gamepad)) = gilrs.gamepads().next() {
            info!("Found controller: {} (PS4 detected)", gamepad.name());
            break;
        }
        gilrs.next_event();
        std::thread::sleep(Duration::from_millis(500));
    }

    info!("Publishing controller state to Zenoh key: {}", key_expr);
    info!("Press Ctrl+C to quit");

    loop {
        // Process all events
        while let Some(Event { id, event, .. }) = gilrs.next_event() {
            let gamepad = gilrs.gamepad(id);

            if matches!(event, EventType::Disconnected) {
                info!("Controller disconnected!");
                let state = ControllerState {
                    left_stick_x: 0.0,
                    left_stick_y: 0.0,
                    right_stick_x: 0.0,
                    right_stick_y: 0.0,
                    l2_trigger: 0.0,
                    r2_trigger: 0.0,
                    dpad_x: 0,
                    dpad_y: 0,
                    buttons: vec![],
                    connected: false,
                };
                let json = serde_json::to_string(&state).unwrap();
                session.put(key_expr, json).await?;
            }
        }

        // Build current state
        if let Some((_id, gamepad)) = gilrs.gamepads().next() {
            let mut ps4_info = Ps4Info::default();
            ps4_info.axis_lx = gamepad
                .axis_data(gilrs::Axis::LeftStickX)
                .map(|a| Some((a.value() * 1000.0) as i32))
                .unwrap_or(None);
            ps4_info.axis_ly = gamepad
                .axis_data(gilrs::Axis::LeftStickY)
                .map(|a| Some((a.value() * 1000.0) as i32))
                .unwrap_or(None);
            ps4_info.axis_rx = gamepad
                .axis_data(gilrs::Axis::RightStickX)
                .map(|a| Some((a.value() * 1000.0) as i32))
                .unwrap_or(None);
            ps4_info.axis_ry = gamepad
                .axis_data(gilrs::Axis::RightStickY)
                .map(|a| Some((a.value() * 1000.0) as i32))
                .unwrap_or(None);
            ps4_info.button_up = if gamepad.is_pressed(Button::DPadUp) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_down = if gamepad.is_pressed(Button::DPadDown) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_left = if gamepad.is_pressed(Button::DPadLeft) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_right = if gamepad.is_pressed(Button::DPadRight) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_cross = if gamepad.is_pressed(Button::South) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_circle = if gamepad.is_pressed(Button::East) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_square = if gamepad.is_pressed(Button::West) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_triangle = if gamepad.is_pressed(Button::North) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_left_trigger = if gamepad.is_pressed(Button::LeftTrigger) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_right_trigger = if gamepad.is_pressed(Button::RightTrigger) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_share = if gamepad.is_pressed(Button::Select) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_left_shoulder = if gamepad.is_pressed(Button::LeftThumb) {
                Some(true)
            } else {
                Some(false)
            };
            ps4_info.button_right_shoulder = if gamepad.is_pressed(Button::RightThumb) {
                Some(true)
            } else {
                Some(false)
            };
            let json = serde_json::to_string(&ps4_info).unwrap();
            info!("PS4 Info: {:?}", json);

            let json = serde_json::to_string(&ps4_info).unwrap();
            // convert string to zbytes
            let zb: ZBytes = ZBytes::from(json);
            session.put(key_expr, zb).await?;
        }

        // Limit publish rate to ~60 Hz
        tokio::time::sleep(Duration::from_millis(16)).await;
    }
}

impl Default for ControllerState {
    fn default() -> Self {
        Self {
            left_stick_x: 0.0,
            left_stick_y: 0.0,
            right_stick_x: 0.0,
            right_stick_y: 0.0,
            l2_trigger: 0.0,
            r2_trigger: 0.0,
            dpad_x: 0,
            dpad_y: 0,
            buttons: vec![],
            connected: false,
        }
    }
}
