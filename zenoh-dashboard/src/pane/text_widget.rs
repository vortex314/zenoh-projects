use crate::value::Value;
use crate::{pane::PaneWidget, theme::THEME};

use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct TextWidget {
    title: String,
    topic: String,
    text: String,
}

impl TextWidget {
    pub fn new(title: String, topic: String) -> TextWidget {
        TextWidget {
            title,
            topic,
            text: "".to_string(),
        }
    }
}

impl PaneWidget for TextWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;
        let response = if ui
            .put(
                button_rect,
                egui::Button::new(self.title.clone())
                    .context_menu(|ui| {
                        ui.button("Reset");
                    })
                    .sense(egui::Sense::drag()), //             .fill(THEME.title_background_color),
            )
            .drag_started()
        {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        };
        ui.label(&self.text);
        ui.label("============================================");
        response
    }

    fn title(&self) -> String {
        self.title.clone()
    }

    fn process_data(&mut self, topic: String, value: &Value) -> bool {
        if topic == self.topic {
            //       info!("{}", value.at_idx(0).unwrap());
            let r = value.at_idx(0);
            if r.is_none() {
                info!(" didn't find value at index 0");
            }
            self.text = format!("{}", value.at_idx(5).unwrap());
            true
        } else {
            false
        }
    }
}
