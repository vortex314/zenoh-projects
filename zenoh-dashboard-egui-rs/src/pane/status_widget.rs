use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::value::Value;

use super::{PaneWidget, PubSub, Widget, WidgetReaction};

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
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;
        let ui_response = if ui
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
      //  response
        WidgetReaction { 
            ui_response,
            ..WidgetReaction::default()
        }
    }

    fn context_menu(&mut self,ui: &mut egui::Ui) {
        ui.label("StatusWidget context menu");
    }

    fn process_data(&mut self, _topic: String, _value: &Value)  {
        self.status = Status::Ok;
    }


}
