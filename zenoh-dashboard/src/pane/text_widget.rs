use crate::pane::PaneWidget;
use crate::value::Value;

use egui::{Margin, TextEdit};
use egui_tiles::UiResponse;
use log::*;
use serde::{Deserialize, Serialize};

use super::find_inner_rectangle;
const MARGIN: i8 = 5;


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
            prefix: "".to_string(), 
            suffix: "".to_string(),
            text: "----".to_string(),
        }
    }
}

impl PaneWidget for TextWidget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let rect = ui.available_rect_before_wrap();
        let rect = rect
            - Margin {
                left: MARGIN,
                right: MARGIN,
                top: MARGIN,
                bottom: MARGIN,
            };
        let s = format!("{}{}{}",self.prefix, self.text,self.suffix );

        let rct = find_inner_rectangle(rect, 2.0/ s.len() as f32  ) ;
        let text_height = rct.height() * 0.8;

        ui.put(
            rct,
            egui::Label::new(
                egui::RichText::new(s.clone())
                    .size(text_height)
                    .color(egui::Color32::BLUE),
            ));
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
        debug!("TextWidget process_data {} {:?}", topic, value);
        match value {
            Value::String(s) => {
                self.text = s.clone();
            }
            Value::Number(n) => {
                self.text = format!("{}", n);
            }

 
            Value::Bool(b) => {
                self.text = format!("{}", b);
            }
            Value::Null => {
                self.text = "-".to_string();
            }
            _ => {
                self.text = "no text".to_string();
            }
        }
    }
}
