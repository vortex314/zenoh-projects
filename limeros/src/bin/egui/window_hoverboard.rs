use std::time::Instant;

use anyhow::Result;
use eframe::egui::{self, Slider};
use ehmi::{Bar, Gauge};
use limeros::msgs::{HoverboardCmd, HoverboardEvent};
use log::info;

const MAX_UPDATE_INTERVAL_MS: u128 = 10000;

use crate::{my_window::MyWindow, widget_alive::WidgetAlive};

pub struct HoverboardWindow {
    open: bool,
    enabled: bool,
    speed: f32,
    steer: f32,
    last_update: Instant,
    last_event: Option<HoverboardEvent>,
    speed_left: f32,
    speed_right: f32,
    temperature: f32,
    voltage: f32,
    current: f32,
}

impl Default for HoverboardWindow {
    fn default() -> Self {
        Self {
            open: true,
            enabled: true,
            speed: 0.0,
            steer: 0.0,
            last_update: Instant::now(),
            last_event: None,
            speed_left: 0.0,
            speed_right: 0.0,
            temperature: 0.0,
            voltage: 0.0,
            current: 0.0,
        }
    }
}
impl HoverboardWindow {
    fn ui(&mut self, ui: &mut egui::Ui) {
        self.enabled = Instant::now().duration_since(self.last_update).as_millis()
            < MAX_UPDATE_INTERVAL_MS as u128;
        let alive_widget = WidgetAlive::new(self.enabled);
        ui.add(alive_widget);
        ui.horizontal(|ui| {
            ui.spacing_mut().slider_width = 400.0;
            ui.add(
                Slider::new(&mut self.speed, -400.0..=400.0)
                    .suffix(" Speed")
                    .step_by(1.0),
            );
        });
        ui.horizontal(|ui| {
            ui.spacing_mut().slider_width = 400.0;
            ui.add(
                Slider::new(&mut self.steer, -100.0..=100.0)
                    .suffix(" Steer")
                    .step_by(1.0),
            );
        });
        ui.vertical_centered(|ui| {
            ui.horizontal(|ui| {
                let text = format!("Left Wheel: \n{:.1} RPM", self.speed_left);
                ui.add(
                    Gauge::new(self.speed_left)
                        .size(200.0)
                        .text(&text)
                        .range(-400f64..=400f64)
                        .stroke_width(0.0)
                        .angle_range(-45i16..=225i16),
                );
                ui.add(egui::Label::new("    "));
                let text = format!("Right Wheel: \n{:.1} RPM", self.speed_right);
                ui.add(
                    Gauge::new(self.speed_right)
                        .size(200.0)
                        .text(&text)
                        .range(-400f64..=400f64)
                        .stroke_width(0.0)
                        .angle_range(-45i16..=225i16),
                );
            });
        });

        ui.vertical(|ui| {
            let text = format!("Temperature: \n{:.1} °C", self.temperature);
            ui.add(
                Bar::new(self.temperature as f32)
                    .range(-20.0..=100.0)
                    .bar_size(5.0)
                    .text(&text),
            );
            let text = format!("Battery: \n{:.1} V", self.voltage);
            ui.add(
                Bar::new(self.voltage as f32)
                    .range(-20.0..=100.0)
                    .bar_size(5.0)
                    .text(&text),
            );
            let text = format!("Current: \n{:.1} A", self.current);
            ui.add(
                Bar::new(self.current as f32)
                    .range(-20.0..=100.0)
                    .bar_size(5.0)
                    .text(&text),
            );
        });

        let msg = HoverboardCmd {
            speed: Some(self.speed as i32),
            steer: Some(self.steer as i32),
            ..Default::default()
        };
    }
}

impl MyWindow for HoverboardWindow {
    fn name(&self) -> &'static str {
        "Hoverboard Control"
    }

    fn show(&mut self, ui: &mut egui::Ui) -> Result<()> {
        let mut open = self.open;
        egui::Window::new(self.name())
            .open(&mut open)
            .resizable([true, true])
            .default_size([200.0, 200.0])
            .show(ui.ctx(), |ui| {
                self.ui(ui);
                ui.allocate_space(ui.available_size());
            });
        self.open = open;
        Ok(())
    }

    fn is_closed(&self) -> bool {
        !self.open
    }
}
