use crate::pane::PaneWidget;
use crate::value::Value;

use egui::TextEdit;
use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct TextWidget {
    // config fields
    prefix : String,
    suffix : String,
    // runtime fields
    #[serde(skip)]
    text: String,
}

impl TextWidget {
    pub fn new() -> TextWidget {
        TextWidget {
            prefix: "'".to_string(),
            suffix: "'".to_string(),
            text: "-".to_string(),
        }
    }
}

impl PaneWidget for TextWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let s = format!("{}{}{}",self.prefix, self.text,self.suffix );
        ui.label(s.clone());
        egui_tiles::UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.separator();
        ui.label("TextWidget context menu");
        ui.separator();
        ui.horizontal(|ui| {
            ui.label("Prefix:");
            ui.text_edit_singleline(&mut self.prefix);
        });
        ui.horizontal(|ui| {
            ui.label("Suffix:");
            ui.text_edit_singleline(&mut self.suffix);
        });

    }

    fn process_data(&mut self, topic: String, value: &Value)  {
        info!("TextWidget process_data {} {:?}", topic, value);
        self.text = format!("{}", value);
    }
}
