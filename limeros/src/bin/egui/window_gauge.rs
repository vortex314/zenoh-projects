use std::ops::RangeInclusive;
use std::sync::Arc;

use anyhow::Result;
use dashmap::DashMap;
use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints};
use ehmi::Gauge;
use log::info;

use crate::my_window::{ MyWindow};

use crate::{MetricData, Record};

pub(crate) struct WindowGauge {
    key: String,
    cache: Arc<DashMap<String, Record>>,
    window_name: String,
    range: RangeInclusive<f64>,
    open : bool,
}

impl WindowGauge {
    pub fn new(key: String, cache: Arc<DashMap<String, Record>>) -> Self {
        let window_name = format!("Gauge: {}", key);
        Self {
            key,
            cache,
            window_name,
            range: std::f64::MAX..=std::f64::MIN,
            open : true,
        }
    }

    fn adjust_range(&mut self, new_value: f64) {
        if new_value < *self.range.start() {
            self.range = (new_value)..=(self.range.end().clone());
        }
        if new_value > *self.range.end() {
            self.range = (self.range.start().clone())..=(new_value);
        }
        if self.range.end() == self.range.start() {
            let mid = (self.range.start() + self.range.end()) / 2.0;
            self.range = (mid - 0.5)..=(mid + 0.5);
        }
    }
}

impl MyWindow for WindowGauge {
    fn name(&self) -> &'static str {
        "Gauge Window"
    }

    fn show(&mut self, ui: &mut egui::Ui) -> Result<()> {
        let key = self.key.clone();
        let numeric_value = {
            let record = self
                .cache
                .get(&key)
                .ok_or(anyhow::anyhow!("Value missing"))?;
            record.value.parse::<f64>()?
        };
        self.adjust_range(numeric_value);

        egui::Window::new(self.window_name.clone())
            .open(&mut self.open)
            .resizable([true, true])
            .default_size([250.0, 250.0])
            .show(ui.ctx(), |ui| {
                ui.vertical_centered(|ui| {
                    ui.add(Gauge::new(numeric_value as f32).range(self.range.clone()))
                })
            });
        Ok(())
    }

    fn is_closed(&self) -> bool {
        !self.open
    }
}
