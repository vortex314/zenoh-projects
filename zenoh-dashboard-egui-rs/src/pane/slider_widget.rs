use std::str::FromStr;
use std::sync::Arc;

use crate::pane::{EndPoint, WidgetEvent};
use crate::value::Value;
use crate::{pane::PaneWidget, shared::SHARED};

use egui::{InnerResponse, Margin, TextEdit, Widget};
use egui_tiles::UiResponse;
use log::*;
use serde::de::value;
use serde::{Deserialize, Serialize};

use super::{find_inner_rectangle, WidgetReaction};
const MARGIN: i8 = 5;

#[derive(Debug, Serialize, Deserialize, PartialEq)]

enum InputKind {
    Float32,
    Float64,
    Int32,
    Int64,
    UInt32,
    UInt64,
    String,
    Bool,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SliderWidget {
    min: f32,
    max: f32,
    dst: String,
    dst_endpoint: Option<EndPoint>,
    send_direct: bool,

    format: String,
    // runtime fields
    #[serde(skip)]
    text: String,
    #[serde(skip)]
    value: f32,
}

impl SliderWidget {
    pub fn new() -> SliderWidget {
        SliderWidget {
            min: 0.0,
            max: 100.0,
            dst: "".to_string(),
            dst_endpoint: None,
            send_direct: false,

            format: "{value:5.5}".to_string(),
            text: "0.0".to_string(),
            value: 0.0,
        }
    }
}

impl PaneWidget for SliderWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let mut value = Value::Null;
        let _rect = ui.available_rect_before_wrap();
        ui.spacing_mut().slider_width = _rect.width()/2.0;


        ui.horizontal(|ui| {
            ui.label("Value:");
            let response = egui::Slider::new(&mut self.value, self.min..=self.max)
                .text(&self.text)
                .show_value(true)
                .ui(ui);

            if self.send_direct  {
                if response.changed() {
                    value = Value::Number(self.value as f64);
                }
            } else if ui
                .button("Send")
                .on_hover_text("Send value to topic")
                .clicked()
            {
                value = Value::Number(self.value as f64);
            };
        });
        if value != Value::Null && self.dst_endpoint.is_some() {
            info!("SliderWidget {:?} {:?}", self.dst_endpoint.clone().unwrap(), value);
            WidgetReaction {
                ui_response: UiResponse::None,
                event: Some(WidgetEvent::Publish(self.dst_endpoint.clone().unwrap(), value)),
            }
        } else {
            WidgetReaction::default()
        }
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.separator();
        ui.label("SliderWidget context menu");
        ui.separator();
        ui.horizontal(|ui| {
            ui.label("Destination:");
            ui.text_edit_singleline(&mut self.dst);
            self.dst_endpoint = EndPoint::from_str(&self.dst).ok();
        });
        ui.horizontal(|ui| {
            ui.label("Send direct:");
            ui.checkbox(&mut self.send_direct, "");
        });
        ui.horizontal(|ui| {
            ui.label("Min:");
            ui.add(egui::DragValue::new(&mut self.min));
        });
        ui.horizontal(|ui| {
            ui.label("Max:");
            ui.add(egui::DragValue::new(&mut self.max));
        });
        ui.horizontal(|ui| {
            ui.label("Format:");
            ui.text_edit_singleline(&mut self.format);
        });
    }

    fn process_data(&mut self, topic: String, value: &Value) {
        self.text = value.formatter(&self.format);
    }
}
