use crate::{pane::PaneWidget, shared::SHARED};
use crate::value::Value;

use egui::{Margin, TextEdit};
use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};

use super::find_inner_rectangle;
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
            text: "----".to_string(),
        }
    }
}

impl PaneWidget for InputWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        ui.horizontal(|ui| {
            ui.label("Value:");
            ui.text_edit_singleline(&mut self.text);
            if ui
                .button("Send")
                .on_hover_text("Send value to topic")
                .clicked()
            {
                let value = match self.kind {
                    InputKind::Float32 => Value::Number(self.text.parse::<f32>().unwrap().into()),
                    InputKind::Float64 => Value::Number(self.text.parse::<f64>().unwrap().into()),
                    InputKind::Int32 => Value::Number(self.text.parse::<i32>().unwrap().into()),
                    InputKind::UInt32 => Value::Number(self.text.parse::<u32>().unwrap().into()),
                    InputKind::String => Value::String(self.text.clone()),
                    InputKind::Bool => Value::Bool(self.text.parse::<bool>().unwrap()),
                    _ => {
                        error!("Unsupported type");
                        Value::Null
                    }
                };
                info!("InputWidget send value {} {:?}", self.text, value);

            }
        });

        egui_tiles::UiResponse::None
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
