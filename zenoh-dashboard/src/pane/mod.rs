use egui_tiles::UiResponse;
use serde::{Deserialize, Serialize};

use crate::value::Value;
mod text_widget;
pub use text_widget::TextWidget;
mod status_widget;
pub use status_widget::StatusWidget;
mod null_widget;
pub use null_widget::NullWidget;

#[derive(Debug, Serialize, Deserialize)]
pub struct EndPoint {
    topic: String,
    field: u32,
    lua_filter: String,
}

#[derive(Debug, Serialize, Deserialize, Default)]
struct PubSub {
    src: Option<Vec<EndPoint>>,
    dst: Option<Vec<EndPoint>>,
}

pub trait PaneWidget
where
    Self: std::fmt::Debug + Send,
{
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse;
    fn context_menu(&mut self, ui: &mut egui::Ui);
    fn process_data(&mut self, topic: String, value: &Value) -> bool;
    fn title(&self) -> String;
}

#[derive(Debug, Serialize, Deserialize)]

pub enum Widget {
    TextWidget(TextWidget),
    StatusWidget(StatusWidget),
    NullWidget(NullWidget),
}

impl PaneWidget for Widget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        match self {
            Widget::TextWidget(tw) => tw.show(ui),
            Widget::StatusWidget(sw) => sw.show(ui),
            Widget::NullWidget(nw) => nw.show(ui),
        }
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        match self {
            Widget::TextWidget(tw) => tw.context_menu(ui),
            Widget::StatusWidget(sw) => sw.context_menu(ui),
            Widget::NullWidget(nw) => nw.context_menu(ui),
        }
    }

    fn process_data(&mut self, topic: String, value: &Value) -> bool {
        match self {
            Widget::TextWidget(tw) => tw.process_data(topic, value),
            Widget::StatusWidget(sw) => sw.process_data(topic, value),
            Widget::NullWidget(nw) => nw.process_data(topic, value),
        }
    }
    fn title(&self) -> String {
        match self {
            Widget::TextWidget(tw) => tw.title(),
            Widget::StatusWidget(sw) => sw.title(),
            Widget::NullWidget(nw) => nw.title(),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Pane {
    retain: bool,
    widget: Widget,
}

impl Pane {
    pub fn new(widget: Widget) -> Pane {
        Pane {
            retain: true,
            widget,
        }
    }
    pub fn retain(&self) -> bool {
        self.retain
    }
}

impl PaneWidget for Pane {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;
        let resp = ui.put(
            button_rect,
            egui::Button::new(self.widget.title()).sense(egui::Sense::drag()), //             .fill(THEME.title_background_color),
        );
        if ui.button("X").clicked() {
            self.retain = false;
        }
        resp.context_menu(|ui| {
            self.widget.context_menu(ui);
            if ui.button("Ok").clicked() {
                ui.close_menu();
            }
            if ui.button("Delete").clicked() {
                self.retain = false;
            }
        });
        let uiresponse = if resp.drag_started() {
            egui_tiles::UiResponse::DragStarted
        } else {
            egui_tiles::UiResponse::None
        };
        let _ = self.widget.show(ui);
        uiresponse
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        self.widget.context_menu(ui);
    }

    fn process_data(&mut self, topic: String, value: &Value) -> bool {
        self.widget.process_data(topic, value)
    }
    fn title(&self) -> String {
        self.widget.title()
    }
}
