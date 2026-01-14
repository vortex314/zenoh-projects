use std::sync::Arc;
use anyhow::Result;

use dashmap::DashMap;
use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints};

use crate::my_window::MyWindow;

use crate::MetricData;

pub(crate) struct WindowPlot {
    keys: Vec<String>,
    graph_data: Arc<DashMap<String, MetricData>>,
    window_name: String,
    open: bool,
}

impl WindowPlot {
    pub fn new(keys: Vec<String>, graph_data: Arc<DashMap<String, MetricData>>) -> Self {
        let window_name = keys.iter().cloned().collect::<Vec<String>>().join(", ");
        Self {
            keys,
            graph_data,
            window_name,
            open: true,
        }
    }
}

impl MyWindow for WindowPlot {
    fn name(&self) -> &'static str {
        "Plot Window"
    }

    fn is_closed(&self) -> bool {
        !self.open
    }

    fn show(&mut self, ui: &mut egui::Ui) -> Result<()> {
        let mut lines = Vec::new();
        for key in &self.keys {
            let metric_data = self.graph_data.get(key);
            if metric_data.is_none() {
                ui.label(format!("No data available for metric key: {}", key));
                return Ok(());
            }
            let points = &metric_data.unwrap().points;
            let plot_points: PlotPoints = PlotPoints::from(points.clone());
            let line = Line::new(plot_points).name(key);
            lines.push(line);
        }

        egui::Window::new(self.window_name.clone())
            .open(&mut self.open)
            .resizable([true, true])
            .default_size([600.0, 400.0])
            .show(ui.ctx(), |ui| {
                Plot::new(&self.window_name)
                    .view_aspect(2.0)
                    .show(ui, |plot_ui| {
                        for line in lines {
                            plot_ui.line(line);
                        }
                    });
            });
        Ok(())
    }
}
