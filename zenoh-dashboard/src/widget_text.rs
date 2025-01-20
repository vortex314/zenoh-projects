use crate::pane::Pane;
use crate::pane::PaneWidget;
use crate::value::Value;
use egui::Ui;
use egui::Widget;
use egui_tiles::UiResponse;
use log::*;
use serde::{Serialize, Deserialize};

#[derive(Debug,Serialize,Deserialize)]
pub struct WidgetText {
    title: String,
    topic: String,
    text: String,
}

impl WidgetText {
    pub fn new(title: String, topic: String) -> WidgetText {
        WidgetText {
            title,
            topic,
            text: "".to_string(),
        }
    }
}

impl PaneWidget for WidgetText {
    fn show(&mut self, ui: &mut egui::Ui)-> UiResponse {

        ui.label(&self.text);
        ui.label("============================================");
        UiResponse::None
    }

    fn title(&self) -> String {
        self.title.clone()
    }

    fn process_data(&mut self, topic: String, value: &Value) -> () {
        if topic == self.topic {
            info!("{}", value.at_idx(0).unwrap());
            let r = value.at_idx(0);
            if r.is_none() {
                info!(" didn't find value at index 0");
            }
            self.text = format!("{}", value.at_idx(0).unwrap());
        } 
    }
}
