use std::str::FromStr;

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
pub struct InputWidget {
    // config fields
    prefix: String,
    suffix: String,
    kind: InputKind,
    // runtime fields
    #[serde(skip)]
    text: String,
}

impl InputWidget {
    pub fn new() -> InputWidget {
        InputWidget {
            kind: InputKind::Float32,
            prefix: "".to_string(),
            suffix: "".to_string(),
            text: "0.01".to_string(),
        }
    }
}

impl PaneWidget for InputWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let mut value = Value::Null;
        let rect = ui.available_rect_before_wrap();
        ui.horizontal(|ui| {
            ui.label("Value:");
            ui.add(egui::TextEdit::singleline(&mut self.text));

            if ui
                .button(">")
                .on_hover_text("Send value to topic")
                .clicked()
            {
                value = Value::from_text(&self.text)
            };

        });
        if value != Value::Null {
            info!("InputWidget send value {} {:?}", self.text, value);
            WidgetReaction {
                ui_response: UiResponse::None,
                event: Some(WidgetEvent::Publish(EndPoint::from_str("").unwrap(), value)),
            }
        } else {
            WidgetReaction::default()
        }
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.separator();
        ui.label("InputWidget context menu");
        ui.separator();
        ui.horizontal(|ui| {
            ui.label("Input kind:");
            ui.radio_value(&mut self.kind, InputKind::Float32, "Float32");
            ui.radio_value(&mut self.kind, InputKind::Float64, "Float64");
            ui.radio_value(&mut self.kind, InputKind::Int32, "Int32");
            ui.radio_value(&mut self.kind, InputKind::Int64, "Int64");
            ui.radio_value(&mut self.kind, InputKind::UInt32, "UInt32");
            ui.radio_value(&mut self.kind, InputKind::UInt64, "UInt64");
            ui.radio_value(&mut self.kind, InputKind::String, "String");
            ui.radio_value(&mut self.kind, InputKind::Bool, "Bool");
        });
    }

    fn process_data(&mut self, topic: String, value: &Value) {
        debug!("InputWidget process_data {} {:?}", topic, value);
        match value {
            Value::String(s) => {
                self.text = s.clone();
            }
            Value::Number(n) => {
                self.text = format!("{}", n);
            }

            Value::Bool(b) => {
                self.text = format!("{}", b);
            }
            Value::Null => {
                self.text = "-".to_string();
            }
            _ => {
                self.text = "no text".to_string();
            }
        }
    }
}
