use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{find_inner_rectangle, PaneWidget, PubSub, WidgetReaction};

const RADIUS: f32 = 100.0;
const THICKNESS: f32 = 10.0;
const MARGIN: i8 = 5;

#[derive(Debug, Serialize, Deserialize)]
enum Status {
    Ok,
    Warning,
    Error,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct GaugeWidget {
    // config fields 
    min: f32,
    max: f32,
    start: f32,
    unit: String,
    // runtime fields
    #[serde(skip)]
    value: f32,
    #[serde(skip)]
    bottom_center: Pos2,
    #[serde(skip)]
    outer_radius: f32,
    #[serde(skip)]
    thickness: f32,
}

impl GaugeWidget {
    pub fn new() -> GaugeWidget {
        GaugeWidget {
            min: 0.0,
            max: 100.0,
            start: 0.0,
            unit: String::from(""), // unit of measure
            value: 0.0,
            bottom_center: Pos2::new(0.0, 0.0),
            outer_radius: RADIUS,
            thickness: THICKNESS,
        }
    }
}

impl PaneWidget for GaugeWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        self.draw_gauge(ui);
        WidgetReaction::default()
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("GaugeWidget context menu");
        ui.horizontal(|ui| {
            ui.label("Min:");
            ui.add(egui::DragValue::new(&mut self.min));
        });
        ui.horizontal(|ui| {
            ui.label("Max:");
            ui.add(egui::DragValue::new(&mut self.max));
        });
        ui.horizontal(|ui| {
            ui.label("Start:");
            ui.add(egui::DragValue::new(&mut self.start));
        });
        ui.horizontal(|ui| {
            ui.label("Unit:");
            ui.text_edit_singleline(&mut self.unit);
        });
    }

    fn process_data(&mut self, _topic: String, _value: &Value) {
        self.value = _value.as_f64().unwrap_or(0.0) as f32;
    }
}

use egui::{Color32, Margin, Pos2, Rect, Response, Sense, Shape, Stroke, Ui, Vec2};
use std::f32::consts::PI;



impl GaugeWidget {
    pub fn draw_gauge(&mut self, ui: &mut Ui) {
        let rect = ui.available_rect_before_wrap();
        let rect = rect
            - Margin {
                left: MARGIN,
                right: MARGIN,
                top: MARGIN,
                bottom: MARGIN,
            };
        let inner_rect = find_inner_rectangle(rect, 0.5);
        self.bottom_center = inner_rect.center_bottom();
        self.outer_radius = inner_rect.height();
        self.thickness = self.outer_radius * 0.2;
        self.draw_gauge_arc(ui.painter(), self.bottom_center, self.outer_radius);
        let value_angle = map_value_to_angle(self.value, self.min, self.max);
        let start_angle = map_value_to_angle(self.start, self.min, self.max);
        draw_filler_arc(
            ui.painter(),
            self.bottom_center,
            self.outer_radius * 0.9,
            start_angle,
            value_angle,
        );
        self.draw_value(ui);
    }

    fn draw_gauge_arc(&self, painter: &egui::Painter, center: Pos2, radius: f32) {
        let mut points = fill_arc(center, radius, -PI, 0.0);
        let mut inner_arc = fill_arc(center, radius - self.thickness, 0.0, -PI);
        points.append(&mut inner_arc);
        points.push(points[0]);
        painter.add(Shape::line(points, Stroke::new(1.0, Color32::GRAY)));
    }

    fn draw_value(&self, ui: &mut Ui) {
        let text_rect = Rect { // make text fit thickness of Gauge Arc
            min: Pos2 {
                x: self.bottom_center.x - self.outer_radius + self.thickness,
                y: self.bottom_center.y - self.thickness * 2.0,
            },
            max: Pos2 {
                x: self.bottom_center.x + self.outer_radius - self.thickness,
                y: self.bottom_center.y - self.thickness,
            },
        };
        ui.put(
            text_rect,
            egui::Label::new(
                egui::RichText::new(format!("{}", self.value.to_string()))
                    .size(self.thickness)
                    .color(egui::Color32::BLUE),
            )
        );
        ui.put(
            text_rect.translate(Vec2::new(0.0, self.thickness)),
            egui::Label::new(
                egui::RichText::new(self.unit.as_str())
                    .size(self.thickness/2.0)
                    .color(egui::Color32::BLUE),
            )
        );
    }

    fn draw_ticks(&self, painter: &egui::Painter, center: Pos2, radius: f32) {
        let num_ticks = 9; // Draw 9 ticks for 8 equal segments
        let tick_length = 10.0;

        for i in 0..num_ticks {
            let angle = map_range(i as f32, 0.0, (num_ticks - 1) as f32, PI, 0.0);

            // Draw tick mark
            let outer = Pos2::new(
                center.x + (radius + 2.0) * angle.cos(),
                center.y + (radius + 2.0) * angle.sin(),
            );
            let inner = Pos2::new(
                center.x + (radius - tick_length) * angle.cos(),
                center.y + (radius - tick_length) * angle.sin(),
            );
            painter.line_segment([outer, inner], Stroke::new(1.0, Color32::WHITE));

            // Draw label
            let value = map_range(i as f32, 0.0, (num_ticks - 1) as f32, self.min, self.max);
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
    if value < from_min {
        return to_min;
    }
    if value > from_max {
        return to_max;
    }
    let slope = (to_max - to_min) / (from_max - from_min);
    to_min + slope * (value - from_min)
}

fn fill_arc(center: Pos2, radius: f32, start_angle: f32, end_angle: f32) -> Vec<Pos2> {
    let mut points = Vec::new();
    let num_points = 50;
    let delta = (end_angle - start_angle) / num_points as f32;
    for i in 0..=num_points {
        let angle = start_angle + delta * i as f32;
        let point = center + Vec2::angled(angle) * radius;
        points.push(point);
    }
    points
}

fn draw_filler_arc(
    painter: &egui::Painter,
    center: Pos2,
    radius: f32,
    start_angle: f32,
    end_angle: f32,
) {
    let thickness = radius * 0.2; // 20% of radius
    let  points = fill_arc(center, radius, start_angle, end_angle);
    painter.add(Shape::line(points, Stroke::new(thickness, Color32::BLUE)));
}
/* 
fn find_inner_rectangle(rect: Rect, rectangle_ratio_y_vs_x: f32) -> Rect {
    let mut inner_rect = rect;
    if rect.width() * rectangle_ratio_y_vs_x > rect.height() {
        let width = rect.height() / rectangle_ratio_y_vs_x;
        inner_rect.min.x = rect.center().x - width / 2.0;
        inner_rect.max.x = rect.center().x + width / 2.0;
    } else {
        let height = rect.width() * rectangle_ratio_y_vs_x;
        inner_rect.min.y = rect.max.y - height;
    }
    inner_rect
}*/

fn map_value_to_angle(value: f32, min: f32, max: f32) -> f32 {
    map_range(value, min, max, -PI, 0.0)
}
