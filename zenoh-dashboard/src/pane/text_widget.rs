use std::{collections::HashMap, hash::Hash};

use crate::pane::PaneWidget;
use crate::value::Value;

use egui::{Margin, TextEdit};
use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};
use strfmt::{strfmt, DisplayStr};

use super::{find_inner_rectangle, WidgetReaction};
const MARGIN: i8 = 5;


#[derive(Debug, Serialize, Deserialize)]
pub struct TextWidget {

    format : String,
    // runtime fields
    #[serde(skip)]
    text: String,
}

impl TextWidget {
    pub fn new() -> TextWidget {
        TextWidget {
            format: "{value:5.5}".to_string(),
            text: "No value yet".to_string(),
        }
    }
}

impl PaneWidget for TextWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> WidgetReaction {
        let rect = ui.available_rect_before_wrap();
        let rect = rect
            - Margin {
                left: MARGIN,
                right: MARGIN,
                top: MARGIN,
                bottom: MARGIN,
            };

        let rct = find_inner_rectangle(rect, 2.0/ self.text.len() as f32  ) ;
        let text_height = rct.height() * 0.8;

        ui.put(
            rct,
            egui::Label::new(
                egui::RichText::new(self.text.clone())
                    .size(text_height)
                    .color(egui::Color32::BLUE),
            ));
        WidgetReaction::default()
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        ui.separator();
        ui.label("TextWidget context menu");
        ui.separator();

        ui.horizontal(|ui| {
            ui.label("Format:");
            ui.text_edit_singleline(&mut self.format);
        });

    }

    fn process_data(&mut self, topic: String, value: &Value)  {
        debug!("TextWidget process_data {} {:?}", topic, value);
    
        self.text = value.formatter(&self.format);

    }
}
