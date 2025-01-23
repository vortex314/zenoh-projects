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
pub struct StatusWidget {
    title: String,
    pub_sub: PubSub,
    status: Status,
}

impl StatusWidget {
    pub fn new() -> StatusWidget {
        StatusWidget {
            title: "No title".to_string(),
            pub_sub: PubSub::default(),
            status: Status::Ok,
        }
    }
}

impl PaneWidget for StatusWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;
        let response = if ui
            .put(
                button_rect,
                egui::Button::new(self.title.clone()).sense(egui::Sense::drag()), //             .fill(THEME.title_background_color),
            )
            .drag_started()
        {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        };
        ui.label(&self.title);
        ui.label("============================================");
        response
    }

    fn process_data(&mut self, topic: String, _value: &Value) -> bool {
        self.pub_sub.src.as_ref().unwrap().iter().any(|ep| {
            if ep.topic == topic {
                info!("found topic: {}", topic);
                true
            } else {
                false
            }
        })
    }

    fn title(&self) -> String {
        self.title.clone()
    }
}
