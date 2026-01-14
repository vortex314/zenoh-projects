use eframe::egui::{self, Widget};


pub struct WidgetAlive {
    pub alive: bool,
}

impl WidgetAlive {
    pub fn new(alive: bool) -> Self {
        Self { alive }
    }
}

impl Widget for WidgetAlive {
    fn ui(self, ui: &mut egui::Ui) -> egui::Response {
        ui.horizontal(|ui| {
            ui.label("Widget Alive:");
            let color = if self.alive {
                egui::Color32::GREEN
            } else {
                egui::Color32::RED
            };
            let rect_size = egui::Vec2::splat(20.0);
            let (rect_id, _response) = ui.allocate_exact_size(rect_size, egui::Sense::hover());
            let painter = ui.painter();
            painter.rect_filled(rect_id, 0.0, color);
        }).response
    }
}
