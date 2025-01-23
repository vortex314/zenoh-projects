
use egui_tiles::UiResponse;
use anyhow::Result;
use serde::{ Deserialize, Serialize};

use crate::{ value::Value};
mod text_widget;
pub use text_widget::TextWidget as TextWidget;
mod status_widget;
pub use status_widget::StatusWidget as StatusWidget;

#[derive(Debug, Serialize, Deserialize)]
pub struct EndPoint {
    topic: String,
    field: u32,
    lua_filter: String,
}

#[derive(Debug, Serialize, Deserialize,Default)]
struct PubSub {
    src : Option<Vec<EndPoint>>,
    dst : Option<Vec<EndPoint>>,
}

pub trait PaneWidget
where
    Self: std::fmt::Debug + Send  ,
{
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse;
    fn process_data(&mut self, topic: String, value: &Value) -> bool;
    fn title(&self) -> String;
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Pane {
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
    fn title(&self) -> String {
        match self {
            Pane::TextWidget(tw) => tw.title(),
            Pane::StatusWidget(sw) => sw.title(),
        }
    }
}

