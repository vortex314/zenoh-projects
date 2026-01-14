use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints};

use crate::my_window::MyWindow;

use crate::{MetricData, Record};

struct WindowPlot {
    window_name: String,
    metric_data: MetricData,
}

impl MyWindow for WindowPlot {
    fn name(&self) -> &'static str {
        "Plot Window"
    }

    fn on_message(&mut self, udp_message: &limeros::UdpMessage) {
        // Handle incoming UDP messages to update plot data
    }

    fn ui(&mut self, ui: &mut egui::Ui) {
        ui.label("This is a placeholder for the Plot Window UI.");
    }

    fn show(&mut self, ui: &mut egui::Ui) {
        egui::Window::new(self.name())
            .open(&mut true)
            .resizable([true, true])
            .default_size([600.0, 400.0])
            .show(ui.ctx(), |ui| {
                let points = self.metric_data.points.clone();
                let plot_points: PlotPoints = PlotPoints::from(points);
                let line = Line::new(plot_points).name(&self.window_name);

                Plot::new(&self.window_name)
                    .view_aspect(2.0)
                    .show(ui, |plot_ui| {
                        plot_ui.line(line);
                    });
            });
    }
}
