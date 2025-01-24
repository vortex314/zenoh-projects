use crate::pane::PaneWidget;
use crate::value::Value;

use egui::TextEdit;
use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct TextWidget {
    title: String,
    topic: String,
    #[serde(skip)]
    text: String,
}

impl TextWidget {
    pub fn new(title: String, topic: String) -> TextWidget {
        TextWidget {
            title,
            topic,
            text: "-".to_string(),
        }
    }
}

fn get_text_property(ui: &mut egui::Ui, label: &str, target: &mut String) {
    ui.horizontal(|ui| {
        ui.label(label);
        ui.add(TextEdit::singleline(target));
    });
}

impl PaneWidget for TextWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let s = format!("{} : {}", self.topic, self.text);
        ui.label(s.clone());
        //     info!("TextWidget {} {}",self.title, s);
        egui_tiles::UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("TextWidget context menu");
        get_text_property(ui, "Title", &mut self.title);
        get_text_property(ui, "Topic", &mut self.topic);
    }

    fn title(&self) -> String {
        self.title.clone()
    }

    fn process_data(&mut self, topic: String, value: &Value) -> bool {
        if topic == self.topic {
            self.text = format!("{}", value);
            //      info!("TextWidget::process_data: {}, value={}", self.title, self.text);
            true
        } else {
            false
        }
    }
}
