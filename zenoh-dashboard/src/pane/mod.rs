use egui::{include_image, ImageSource, Rect, Sense};
use egui_tiles::UiResponse;
use log::{debug, info};
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
mod plot_widget;
pub use plot_widget::PlotWidget;

#[derive(Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct EndPoint {
    topic: String,
    field: Option<String>,
    lua_filter: Option<String>,
}

impl ToString for EndPoint {
    fn to_string(&self) -> String {
        format!("{}.{}", self.topic, self.field.clone().unwrap_or("".to_string()))
    }
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
    PlotWidget(PlotWidget),
}

impl PaneWidget for Widget {
    fn show(&mut self, ui: &mut egui::Ui) -> UiResponse {
        match self {
            Widget::TextWidget(tw) => tw.show(ui),
            Widget::StatusWidget(sw) => sw.show(ui),
            Widget::NullWidget(nw) => nw.show(ui),
            Widget::GaugeWidget(gw) => gw.show(ui),
            Widget::PlotWidget(pw) => pw.show(ui),
        }
    }

    fn context_menu(&mut self, ui: &mut egui::Ui) {
        match self {
            Widget::TextWidget(tw) => tw.context_menu(ui),
            Widget::StatusWidget(sw) => sw.context_menu(ui),
            Widget::NullWidget(nw) => nw.context_menu(ui),
            Widget::GaugeWidget(gw) => gw.context_menu(ui),
            Widget::PlotWidget(pw) => pw.context_menu(ui),
        }
    }

    fn process_data(&mut self, topic: String, value: &Value) {
        match self {
            Widget::TextWidget(tw) => tw.process_data(topic, value),
            Widget::StatusWidget(sw) => sw.process_data(topic, value),
            Widget::NullWidget(nw) => nw.process_data(topic, value),
            Widget::GaugeWidget(gw) => gw.process_data(topic, value),
            Widget::PlotWidget(pw) => pw.process_data(topic, value),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Pane {
    retain: bool,
    pub title: String,
    src: Vec<EndPoint>,
    widget: Widget,
    #[serde(skip)]
    value: Value,
    #[serde(skip)]
    fields: Vec<String>,
}

impl Pane {
    pub fn new(widget: Widget) -> Pane {
        Pane {
            retain: true,
            title: "Pane".to_string(),
            src: Vec::new(),
            widget,
            value: Value::Null,
            fields: Vec::new(),
        }
    }
    pub fn retain(&self) -> bool {
        self.retain
    }

    pub fn title(&self) -> String {
        self.title.clone()
    }
}

fn get_topic(ui: &mut egui::Ui, selected_value: &mut String, cnt: usize) {
    let options = possible_topics();
    ui.label("Topic");
    egui::ComboBox::from_id_salt(format!("topic{}", cnt))
        .selected_text(selected_value.as_str())
        .width(100.0)
        .show_ui(ui, |ui| {
            for option in options {
                ui.selectable_value::<String>(selected_value, option.clone(), option);
            }
        });
}

fn get_opt_field(ui: &mut egui::Ui, selected_value: &mut Option<String>, cnt: usize, fields: &Vec<String>) {
    let fields  = fields.clone();
    ui.label("Field");
    let mut sel_value =selected_value.clone().unwrap_or("".to_string());
    egui::ComboBox::from_id_salt(format!("field{}", cnt))
        .selected_text(sel_value.as_str())
        .width(100.0)
        .show_ui(ui, |ui| {
            for field in fields {
                ui.selectable_value::<String>(&mut sel_value, field.clone(), field);
            }
        });
        if sel_value.len() == 0 {
            *selected_value = None;
        } else {
            *selected_value = Some(sel_value);
        }
}

fn get_lua_filter(ui: &mut egui::Ui, lua_filter: &mut Option<String>) {
    let mut filter = lua_filter.clone().unwrap_or("".to_string());
    ui.label("Lua filter");
    ui.text_edit_multiline(&mut filter);
    if filter.len() == 0 {
        *lua_filter = None;
    } else {
        *lua_filter = Some(filter);
    }
}

fn get_endpoints(ui: &mut egui::Ui, src: &mut Vec<EndPoint>, fields: &Vec<String>) {
    ui.horizontal(|ui| {
        ui.label("Source topics");
        if ui.button("+").clicked() {
            src.push(EndPoint {
                topic: "topic".to_string(),
                field: None,
                lua_filter: None,
            });
        }
    });
    let mut ep_to_remove = Vec::new();
    let mut cnt = 0;
    for ep in src.iter_mut() {
        ui.horizontal(|ui| {
            if ui.button("-").clicked() {
                ep_to_remove.push(ep.clone());
            }
            get_topic(ui, &mut ep.topic, cnt);
            get_opt_field(ui, &mut ep.field, cnt, fields);
            if ui.button("Lua ...").clicked() {
                get_lua_filter(ui, &mut ep.lua_filter);
            }
            cnt += 1;
        });
        // ui.label("Lua filter");
        // ui.text_edit_multiline(&mut ep.lua_filter);
    }
    for ep in ep_to_remove {
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
        button_rect.max.y = button_rect.min.y + 15.0;
        let resp = ui.put(
            button_rect,
            egui::Button::new(self.title()).sense(egui::Sense::drag()), //             .fill(THEME.title_background_color),
        );
        resp.context_menu(|ui| {
            get_title(ui, &mut self.title);
            get_endpoints(ui, &mut self.src, &self.fields);
            ui.label(self.value.to_string());
            button_bar(ui).map(|icon| {
                info!("Selected icon {:?}", icon);
                self.widget = match icon {
                    IconEvent::Gauge => Widget::GaugeWidget(GaugeWidget::new()),
                    IconEvent::Graph => Widget::PlotWidget(PlotWidget::new()),
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
                debug!(
                    "Processing data for topic {} [{}]{:?} {:?}",
                    topic,
                    ep.field.clone().unwrap_or("None".to_string()),
                    value.get_opt(&ep.field).unwrap_or(&Value::Null),
                    value
                );
                value.get_opt(&ep.field).map(|v| {
                    self.value = v.clone();
                    self.widget.process_data(topic.clone(), &v)
                });
            })
            .collect::<Vec<_>>();
        value.keys().map(|keys| {
            if self.fields.len() != keys.len() {
                self.fields = keys;
            }
        });
    }
}

pub fn find_inner_rectangle(rect: Rect, rectangle_ratio_y_vs_x: f32) -> Rect {
    let mut inner_rect = rect;
    if rect.width() * rectangle_ratio_y_vs_x > rect.height() {
        let width = rect.height() / rectangle_ratio_y_vs_x;
        inner_rect.min.x = rect.center().x - width / 2.0;
        inner_rect.max.x = rect.center().x + width / 2.0;
    } else {
        let height = rect.width() * rectangle_ratio_y_vs_x;
        inner_rect.min.y = rect.max.y - height;
    }
    inner_rect
}
