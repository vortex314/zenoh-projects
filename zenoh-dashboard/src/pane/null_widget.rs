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
pub struct NullWidget {
    title: String,
}

impl NullWidget {
    pub fn new() -> NullWidget {
        NullWidget {
            title: "Null Widget".to_string(),

        }
    }
}

impl PaneWidget for NullWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        ui.label(&self.title);
        ui.separator();
        UiResponse::None
    }

    fn context_menu(&mut self,ui: &mut egui::Ui) {
        ui.label("NullWidget context menu");
    }

    fn process_data(&mut self, _topic: String, _value: &Value) -> bool {
        return false;
    }

    fn title(&self) -> String {
        self.title.clone()
    }
}
