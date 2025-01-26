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
pub struct NullWidget {}

impl NullWidget {
    pub fn new() -> NullWidget {
        NullWidget {}
    }
}

impl PaneWidget for NullWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        // paint rectangle yellow
        let rect = ui.available_rect_before_wrap();
        ui.painter().rect_filled(rect, 0.0, egui::Color32::from_rgb(255, 255, 0));
        ui.label("NullWidget");
        ui.separator();
        UiResponse::None
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.label("NullWidget context menu");
    }

    fn process_data(&mut self, _topic: String, _value: &Value) {}
}
