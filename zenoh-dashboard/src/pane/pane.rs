use egui_tiles::UiResponse;
// use serde::{ser::SerializeStruct, Deserialize, Deserializer, Serialize, Serializer};
//use dyn_serde::{ser::Serializer, de::Deserializer};
use anyhow::Result;
use serde::{ser::SerializeStruct, Deserialize, Serialize};

use crate::{text_widget::TextWidget, value::Value};

struct EndPoint {
    topic: String,
    field: u32,
    lua_filter: String,
}

#[derive(Debug, Serialize, Deserialize)]
struct PubSub {
    src : Option<Vec<Endpoint>>,
    dst : Option<Vec<Endpoint>>,
}

pub trait PaneWidget
where
    Self: std::fmt::Debug + Send  ,
{
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse;
    fn process_data(&mut self, topic: String, value: &Value) -> bool;
}

#[derive(Debug, Serialize, Deserialize)]
enum Pane {
    TextWidget(TextWidget),
    StatusWidget(StatusWidget),
}

impl Pane {

}

impl PaneWidget for Pane {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        match self {
            Pane::TextWidget(tw) => tw.show(ui),
            Pane::StatusWidget(sw) => sw.show(ui),
        }
    }

    fn process_data(&mut self, topic: String, value: &Value) -> bool {
        match self {
            Pane::TextWidget(tw) => tw.process_data(topic, value),
            Pane::StatusWidget(sw) => sw.process_data(topic, value),
        }
    }
}

