// src/main.rs
use gilrs::{Button, Event, EventType, Gilrs};
use serde::Serialize;
use std::time::Duration;
use zenoh::Result as ZResult;
use zenoh::Session;
use zenoh::bytes::ZBytes;

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
    // Open Zenoh session (auto-peers with local router or cloud if configured)
    let session = zenoh::open(zenoh::Config::default()).await?;
    println!("Zenoh session opened");

    // Key expression to publish to (e.g. "controller/ps4/1")
    let key_expr = "src/ps4/Ps4Info";

    let mut gilrs = Gilrs::new().unwrap();
    println!("Looking for PS4 controller...");

    // Wait for at least one gamepad
    loop {
        if let Some((_id, gamepad)) = gilrs.gamepads().next() {
            println!("Found controller: {} (PS4 detected)", gamepad.name());
            break;
        }
        gilrs.next_event();
        std::thread::sleep(Duration::from_millis(500));
    }

    println!("Publishing controller state to Zenoh key: {}", key_expr);
    println!("Press Ctrl+C to quit");

    loop {
        // Process all events
        while let Some(Event { id, event, .. }) = gilrs.next_event() {
            let gamepad = gilrs.gamepad(id);

            if matches!(event, EventType::Disconnected) {
                println!("Controller disconnected!");
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
            let state = ControllerState {
                connected: true,
                left_stick_x: gamepad
                    .axis_data(gilrs::Axis::LeftStickX)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                left_stick_y: gamepad
                    .axis_data(gilrs::Axis::LeftStickY)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                right_stick_x: gamepad
                    .axis_data(gilrs::Axis::RightStickX)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                right_stick_y: gamepad
                    .axis_data(gilrs::Axis::RightStickY)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                l2_trigger: gamepad
                    .axis_data(gilrs::Axis::LeftZ)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                r2_trigger: gamepad
                    .axis_data(gilrs::Axis::RightZ)
                    .map(|a| a.value())
                    .unwrap_or(0.0),
                dpad_x: if gamepad.is_pressed(Button::DPadLeft) {
                    -1
                } else if gamepad.is_pressed(Button::DPadRight) {
                    1
                } else {
                    0
                },
                dpad_y: if gamepad.is_pressed(Button::DPadDown) {
                    -1
                } else if gamepad.is_pressed(Button::DPadUp) {
                    1
                } else {
                    0
                },
                buttons: vec![
                    if gamepad.is_pressed(Button::South) {
                        "Cross".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::East) {
                        "Circle".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::North) {
                        "Triangle".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::West) {
                        "Square".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::LeftTrigger) {
                        "L1".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::RightTrigger) {
                        "R1".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::Select) {
                        "Share".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::Start) {
                        "Options".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::LeftThumb) {
                        "L3".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::RightThumb) {
                        "R3".to_string()
                    } else {
                        "".to_string()
                    },
                    if gamepad.is_pressed(Button::Mode) {
                        "PS".to_string()
                    } else {
                        "".to_string()
                    },
                ]
                .into_iter()
                .filter(|s| !s.is_empty())
                .collect(), 
            };

            let json = serde_json::to_string(&state).unwrap();
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
