use egui_tiles::UiResponse;
use serde::{ser::SerializeStruct, Deserialize, Deserializer, Serialize, Serializer};

use crate::{value::Value, text_widget::TextWidget};

pub trait PaneWidget: std::fmt::Debug + Send {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse;
    fn title(&self) -> String;
    fn process_data(&mut self, topic: String, value: &Value) -> bool;
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer;
    fn deserialize<'de, D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>;
}

#[derive(Debug)]
pub struct Pane {
    pub widget: Box<dyn PaneWidget>,
}

impl Pane {
    pub fn new(widget: impl PaneWidget + 'static) -> Self {
        Self {
            widget: Box::new(widget),
        }
    }
}

impl Serialize for Pane {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let title = self.widget.title();
        let mut state = serializer.serialize_struct("Pane", 1)?;
        state.serialize_field("title", &title)?;
        state.end()
    }
}

impl<'de> Deserialize<'de> for Pane {
    fn deserialize<'de, D>(deserializer: D) -> Result<Pane, D::Error>
    where
        D: Deserializer<'de>,
    {
        let title = String::deserialize(deserializer)?;
        Ok(Pane::new(TextWidget::new(title, "".to_string())))
    }
}
