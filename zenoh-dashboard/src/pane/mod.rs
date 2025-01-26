use egui::{include_image, ImageSource, Sense};
use egui_tiles::UiResponse;
use log::info;
use serde::{Deserialize, Serialize};

use crate::{shared::possible_topics, value::Value};
mod text_widget;
pub use text_widget::TextWidget;
mod status_widget;
pub use status_widget::StatusWidget;
mod null_widget;
pub use null_widget::NullWidget;
mod gauge_widget;
pub use gauge_widget::GaugeWidget;

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct EndPoint {
    topic: String,
    field: String,
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
    fn process_data(&mut self, topic: String, value: &Value);
}

const GAUGE_ICON: ImageSource<'_> = include_image!("../../assets/gauge.png");
const GRAPH_ICON: ImageSource<'_> = include_image!("../../assets/graph.png");
const PROGRESS_ICON: ImageSource<'_> = include_image!("../../assets/progress.png");
const TEXT_ICON: ImageSource<'_> = include_image!("../../assets/text.png");
const LABEL_ICON: ImageSource<'_> = include_image!("../../assets/label.png");

#[derive(Debug)]
enum IconEvent {
    Gauge,
    Graph,
    Progress,
    Text,
    Label,
}

fn button_bar(ui: &mut egui::Ui) -> Option<IconEvent> {
    let mut event = None;

    ui.horizontal(|ui| {
        if ui
            .add(
                egui::Image::new(GAUGE_ICON)
                    .max_width(20.0)
                    .rounding(1.0)
                    .sense(Sense::click()),
            )
            .clicked()
        {
            event = Some(IconEvent::Gauge);
        }
        if ui
            .add(
                egui::Image::new(GRAPH_ICON)
                    .max_width(20.0)
                    .rounding(1.0)
                    .sense(Sense::click()),
            )
            .clicked()
        {
            event = Some(IconEvent::Graph);
        }
        if ui
            .add(
                egui::Image::new(PROGRESS_ICON)
                    .max_width(20.0)
                    .rounding(1.0)
                    .sense(Sense::click()),
            )
            .clicked()
        {
            event = Some(IconEvent::Progress);
        }
        if ui
            .add(
                egui::Image::new(TEXT_ICON)
                    .max_width(20.0)
                    .rounding(1.0)
                    .sense(Sense::click()),
            )
            .clicked()
        {
            event = Some(IconEvent::Text);
        }
        if ui
            .add(
                egui::Image::new(LABEL_ICON)
                    .max_width(20.0)
                    .rounding(1.0)
                    .sense(Sense::click()),
            )
            .clicked()
        {
            event = Some(IconEvent::Label);
        }

    });
    event

}

#[derive(Debug, Serialize, Deserialize)]
pub enum Widget {
    TextWidget(TextWidget),
    StatusWidget(StatusWidget),
    NullWidget(NullWidget),
    GaugeWidget(GaugeWidget),
}

impl PaneWidget for Widget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        match self {
            Widget::TextWidget(tw) => tw.show(ui),
            Widget::StatusWidget(sw) => sw.show(ui),
            Widget::NullWidget(nw) => nw.show(ui),
            Widget::GaugeWidget(gw) => gw.show(ui),
        }
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        match self {
            Widget::TextWidget(tw) => tw.context_menu(ui),
            Widget::StatusWidget(sw) => sw.context_menu(ui),
            Widget::NullWidget(nw) => nw.context_menu(ui),
            Widget::GaugeWidget(gw) => gw.context_menu(ui), 
        }
    }

    fn process_data(&mut self, topic: String, value: &Value) {
        match self {
            Widget::TextWidget(tw) => tw.process_data(topic, value),
            Widget::StatusWidget(sw) => sw.process_data(topic, value),
            Widget::NullWidget(nw) => nw.process_data(topic, value),
            Widget::GaugeWidget(gw) => gw.process_data(topic, value),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Pane {
    retain: bool,
    pub title: String,
    src: Vec<EndPoint>,
    widget: Widget,
}

impl Pane {
    pub fn new(widget: Widget) -> Pane {
        Pane {
            retain: true,
            title: "Pane".to_string(),
            src: Vec::new(),
            widget,
        }
    }
    pub fn retain(&self) -> bool {
        self.retain
    }

    pub fn title(&self) -> String {
        self.title.clone()
    }
}

fn get_topic(ui: &mut egui::Ui, selected_value: &mut String) {
    let options = possible_topics();
    egui::ComboBox::from_label("Select a topic ")
        .width(200.0)
        .show_ui(ui, |ui| {
            for option in options {
                ui.selectable_value::<String>(selected_value, option.clone(), option);
            }
        });
}

fn get_endpoints(ui: &mut egui::Ui, src: &mut Vec<EndPoint>) {
    ui.horizontal(|ui| {
        ui.label("Source topics");
        if ui.button("+").clicked() {
            src.push(EndPoint {
                topic: "topic".to_string(),
                field: "".to_string(),
                lua_filter: "".to_string(),
            });
        }
    });
    let mut ep_to_remove = None;
    for ep in src.iter_mut() {
        if ui.button("-").clicked() {
            ep_to_remove = Some(ep.clone());
        }
        ui.horizontal(|ui| {
            ui.label("Topic");
            get_topic(ui, &mut ep.topic);
            ui.label("Field");
            ui.text_edit_singleline(&mut ep.field);
        });
        ui.label("Lua filter");
        ui.text_edit_multiline(&mut ep.lua_filter);
    }
    if let Some(ep) = ep_to_remove {
        src.retain(|e| e != &ep);
    }
}

fn get_title(ui: &mut egui::Ui, title: &mut String) {
    ui.horizontal(|ui| {
        ui.label("Title");
        ui.text_edit_singleline(title);
    });
}

impl PaneWidget for Pane {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        let mut button_rect = ui.max_rect();
        button_rect.max.y = button_rect.min.y + 20.0;
        let resp = ui.put(
            button_rect,
            egui::Button::new(self.title()).sense(egui::Sense::drag()), //             .fill(THEME.title_background_color),
        );
        resp.context_menu(|ui| {
            get_title(ui, &mut self.title);
            get_endpoints(ui, &mut self.src);
            button_bar(ui).map(|icon| {
                info!("Selected icon {:?}", icon);
                self.widget = match icon {
                    IconEvent::Gauge => Widget::GaugeWidget(GaugeWidget::new()),
                    IconEvent::Graph => Widget::StatusWidget(StatusWidget::new()),
                    IconEvent::Progress => Widget::StatusWidget(StatusWidget::new()),
                    IconEvent::Text => Widget::TextWidget(TextWidget::new()),
                    IconEvent::Label => Widget::NullWidget(NullWidget::new()),
                };
            });
            ui.separator();
            self.widget.context_menu(ui);
            ui.separator();
            ui.horizontal(|ui| {
                if ui.button("Ok").clicked() {
                    ui.close_menu();
                };
                if ui.button("Delete").clicked() {
                    self.retain = false;
                }
            });
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

    fn process_data(&mut self, topic: String, value: &Value) {
        let _ = self
            .src
            .iter()
            .filter(|ep| ep.topic == topic)
            .map(|ep| {
                value
                    .get(&ep.field)
                    .map(|v| self.widget.process_data(topic.clone(), &v));
            })
            .collect::<Vec<_>>();
    }
}
