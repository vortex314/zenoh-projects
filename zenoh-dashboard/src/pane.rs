use std::sync::{Arc, Mutex};
use egui_tiles::UiResponse;
use serde::{ser::SerializeStruct, Deserialize, Serialize, Serializer};

use crate::value::Value;

pub trait PaneWidget: std::fmt::Debug + Send {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse;
    fn title(&self) -> String;
    fn process_data(&mut self, topic: String, value: &Value) -> ();
}

#[derive(Debug, Clone)]

pub struct Pane {
   pub widget: Box<dyn PaneWidget>,
}
impl Pane {
    pub fn new(widget: impl PaneWidget + 'static) -> Self {
        Self {
            widget: Arc::new(Mutex::new(Box::new(widget))),
        }
    }
    pub fn title(&self) -> String {
        self.widget.try_lock().map(|w| w.title()).unwrap_or_default()
    }
}

impl Serialize for Pane {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let title = self.title();
        let mut state = serializer.serialize_struct("Pane", 1)?;
        state.serialize_field("title", &title)?;
        state.end()
    }
}