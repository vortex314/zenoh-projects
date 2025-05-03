use std::{
    collections::VecDeque,
    time::{Duration, SystemTime, UNIX_EPOCH},
};

use egui_plot::{Line, Plot, PlotPoints};
use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{PaneWidget, PubSub, Widget, WidgetReaction};

#[derive(Debug, Serialize, Deserialize)]
enum Status {
    Ok,
    Warning,
    Error,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct PlotWidget {
    max_points: usize,
    time_window: f64,
    min: f64,
    max: f64,
    #[serde(skip)]
    data: VecDeque<[f64; 2]>,
}

pub fn get_current_time() -> f64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs_f64()
}

impl PlotWidget {
    pub fn new() -> PlotWidget {
        PlotWidget {
            max_points: 1000,
            min: 0.0,
            max: 100.0,
            data: VecDeque::new(),
            time_window: 100.0,
        }
    }

    pub fn add_point(&mut self, value: f64) {
        let current_time = get_current_time();

        if self.data.len() >= self.max_points {
            self.data.pop_front();
        }
       // info!("Adding point: {} {}", current_time, value);
        self.data.push_back([current_time, value]);

        // Remove points outside the time window
        let cutoff_time = current_time - self.time_window;
        while let Some(point) = self.data.front() {
            if point[0] < cutoff_time || self.data.len() > self.max_points {
                self.data.pop_front();
            } else {
                break;
            }
        }
    }
}

impl PaneWidget for PlotWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let x_axis_formatter = |value: egui_plot::GridMark, _range: &std::ops::RangeInclusive<f64>| {
            let value = value.value;
            let time = UNIX_EPOCH + Duration::from_secs_f64(value );
            let datetime = chrono::DateTime::<chrono::Local>::from(time);
            datetime.format("%H:%M:%S").to_string()
        };
        let min_x = self.data.front().map(|point| point[0]).unwrap_or(get_current_time()-1000.0);
        let max_x = self.data.back().map(|point| point[0]).unwrap_or(get_current_time());
        Plot::new("realtime_plot")
            .include_y(self.min)
            .include_y(self.max)
            .include_x(min_x)
            .include_x(max_x)
            .show_axes([true, true])
            .x_axis_formatter(x_axis_formatter) // Apply custom x-axis formatter
            .show(ui, |plot_ui| {
                let points: PlotPoints = self.data.iter().copied().collect();
                let line = Line::new(points).width(1.0).color(egui::Color32::BLUE);
                plot_ui.line(line);
            });
        WidgetReaction::default()
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("PlotWidget context menu");
        ui.horizontal(|ui| {
            ui.label("Max Samples:");
            ui.add(egui::DragValue::new(&mut self.max_points));
        });
        ui.horizontal(|ui| {
            ui.label("Time Window:");
            ui.add(egui::DragValue::new(&mut self.time_window));
        });
        ui.horizontal(|ui| {
            ui.label("Min:");
            ui.add(egui::DragValue::new(&mut self.min));
        });
        ui.horizontal(|ui| {
            ui.label("Max:");
            ui.add(egui::DragValue::new(&mut self.max));
        });
    }

    fn process_data(&mut self, _topic: String, _value: &Value) {
        if let Some(value) = _value.as_f64() {
            if value < self.min {
                self.min = value;
            }
            if value > self.max {
                self.max = value;
            }
            self.add_point(value);
        }
    }
}
