use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{PaneWidget, PubSub};

#[derive(Debug, Serialize, Deserialize)]
enum Status {
    Ok,
    Warning,
    Error,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct GaugeWidget {
    min: f32,
    max: f32,
    start: f32,
    #[serde(skip)]
    value: f32,
}

impl GaugeWidget {
    pub fn new() -> GaugeWidget {
        GaugeWidget {
            min: 0.0,
            max: 100.0,
            start: 0.0,
            value: 0.0,
        }
    }
}

impl PaneWidget for GaugeWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        // paint rectangle yellow
        let rect = ui.available_rect_before_wrap();
        self.draw_gauge(ui);
        // draw gauge
        UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("GaugeWidget context menu");
        ui.horizontal(|ui| {
            ui.label("Min:");
            ui.add(egui::Slider::new(&mut self.min, 0.0..=100.0));
        });
        ui.horizontal(|ui| {
            ui.label("Max:");
            ui.add(egui::Slider::new(&mut self.max, 0.0..=100.0));
        });
        ui.horizontal(|ui| {
            ui.label("Start:");
            ui.add(egui::Slider::new(&mut self.start, 0.0..=100.0));
        });

    }

    fn process_data(&mut self, _topic: String, _value: &Value) {
        self.start = _value.as_f64().unwrap_or(0.0) as f32;
    }
}

use egui::{Color32, Pos2, Rect, Response, Sense, Shape, Stroke, Ui, Vec2};
use std::f32::consts::PI;

const RADIUS : f32 = 100.0;
const THICKNESS : f32 = 8.0;

impl GaugeWidget  {
    

    pub fn draw_gauge(&self, ui: &mut Ui) -> Response {
        // Calculate required space
        let size = Vec2::splat(RADIUS * 2.0);
        let (rect, response) = ui.allocate_exact_size(size, Sense::hover());

        if ui.is_rect_visible(rect) {
            let center = rect.center();
            let painter = ui.painter();

            // Draw background arc (180 degrees)
            let bg_stroke = Stroke::new(THICKNESS, Color32::from_gray(60));
            painter.add(Shape::circle_stroke(center, RADIUS, bg_stroke));

            // Calculate angle for current value (map value to -PI to 0 range)
            let angle = map_range(
                self.value,
                self.min,
                self.max,
                PI, // Start at -180 degrees
                0.0, // End at 0 degrees
            );

            // Draw needle
            let needle_length = RADIUS - THICKNESS;
            let needle_end = Pos2::new(
                center.x + needle_length * angle.cos(),
                center.y + needle_length * angle.sin(),
            );
            
            painter.line_segment(
                [center, needle_end],
                Stroke::new(2.0, Color32::RED),
            );

            // Draw center circle
            painter.circle_filled(center, 5.0, Color32::RED);

            // Draw tick marks and labels
            self.draw_ticks(painter, center, RADIUS);
        }

        response
    }

    fn draw_ticks(&self, painter: &egui::Painter, center: Pos2, radius: f32) {
        let num_ticks = 9; // Draw 9 ticks for 8 equal segments
        let tick_length = 10.0;

        for i in 0..num_ticks {
            let angle = map_range(
                i as f32,
                0.0,
                (num_ticks - 1) as f32,
                PI,
                0.0,
            );

            // Draw tick mark
            let outer = Pos2::new(
                center.x + (radius + 2.0) * angle.cos(),
                center.y + (radius + 2.0) * angle.sin(),
            );
            let inner = Pos2::new(
                center.x + (radius - tick_length) * angle.cos(),
                center.y + (radius - tick_length) * angle.sin(),
            );
            painter.line_segment(
                [outer, inner],
                Stroke::new(1.0, Color32::WHITE),
            );

            // Draw label
            let value = map_range(
                i as f32,
                0.0,
                (num_ticks - 1) as f32,
                self.min,
                self.max,
            );
            let label = format!("{:.0}", value);
            let label_pos = Pos2::new(
                center.x + (radius + 20.0) * angle.cos(),
                center.y + (radius + 20.0) * angle.sin(),
            );
            painter.text(
                label_pos,
                egui::Align2::CENTER_CENTER,
                label,
                egui::FontId::proportional(14.0),
                Color32::WHITE,
            );
        }
    }
}

fn map_range(value: f32, from_min: f32, from_max: f32, to_min: f32, to_max: f32) -> f32 {
    let slope = (to_max - to_min) / (from_max - from_min);
    to_min + slope * (value - from_min)
}