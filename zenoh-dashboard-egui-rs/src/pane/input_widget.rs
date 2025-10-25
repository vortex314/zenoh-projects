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
    dst: String,
    kind: InputKind,
    format: String,
    // runtime fields
    #[serde(skip)]
    src_text: String,
    #[serde(skip)]
    dst_text: String,
    #[serde(skip)]
    dst_endpoint: Option<EndPoint>,
}

impl InputWidget {
    pub fn new() -> InputWidget {
        InputWidget {
            dst: "".to_string(),
            kind: InputKind::Float32,
            format: "{value:5.5}".to_string(),
            dst_text:"".to_string(),
            src_text: "0.0".to_string(),
            dst_endpoint: None,
        }
    }
}

impl PaneWidget for InputWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let mut value = Value::Null;
        let _rect = ui.available_rect_before_wrap();
        ui.horizontal(|ui| {
            ui.label( self.src_text.clone());
            ui.label("New Value:");
            ui.add(egui::TextEdit::singleline(&mut self.dst_text));

            if ui
                .button("Send")
                .on_hover_text("Send value to topic")
                .clicked()
            {
                value = Value::from_text(&self.dst_text)
            };
        });
        if value != Value::Null && self.dst_endpoint.is_some() {
            info!("InputWidget send value {} {:?}", self.dst_text, value);
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
        ui.label("InputWidget context menu");
        ui.separator();
        ui.horizontal(|ui| {
            ui.label("Destination:");
            ui.text_edit_singleline(&mut self.dst);
            self.dst_endpoint = EndPoint::from_str(&self.dst).ok();
        });

        ui.horizontal(|ui| {
            ui.label("Input kind:");
            ui.radio_value(&mut self.kind, InputKind::Float32, "F32");
            ui.radio_value(&mut self.kind, InputKind::Float64, "F64");
            ui.radio_value(&mut self.kind, InputKind::Int32, "I32");
            ui.radio_value(&mut self.kind, InputKind::Int64, "I64");
            ui.radio_value(&mut self.kind, InputKind::UInt32, "U32");
            ui.radio_value(&mut self.kind, InputKind::UInt64, "U64");
            ui.radio_value(&mut self.kind, InputKind::String, "Str");
            ui.radio_value(&mut self.kind, InputKind::Bool, "Bool");
        });
    }

    fn process_data(&mut self, topic: String, value: &Value) {
        self.src_text = value.formatter(&self.format);
    }
}
