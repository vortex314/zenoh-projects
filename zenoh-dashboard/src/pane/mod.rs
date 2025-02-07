use std::str::FromStr;

use anyhow::{Context, Result};
use egui::{include_image, ImageSource, Rect, Sense};
use egui_tiles::UiResponse;
use log::{debug, info};
use mlua::{Error, Function, IntoLua};
use serde::{Deserialize, Serialize};

use crate::{shared::get_possible_endpoints, value::Value};
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
}

impl ToString for EndPoint {
    fn to_string(&self) -> String {
        self.field
            .as_ref()
            .map(|f| format!("{}.{}", self.topic, f))
            .unwrap_or(self.topic.clone())
    }
}

impl FromStr for EndPoint {
    type Err = String;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let parts: Vec<&str> = s.split('.').collect();
        if parts.len() == 1 {
            Ok(EndPoint {
                topic: parts[0].to_string(),
                field: None,
            })
        } else if parts.len() == 2 {
            Ok(EndPoint {
                topic: parts[0].to_string(),
                field: Some(parts[1].to_string()),
            })
        } else {
            Err("Invalid endpoint".to_string())
        }
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
    lua_code: Option<String>,
    widget: Widget,
    #[serde(skip)]
    value: Value,
    #[serde(skip)]
    lua: Option<mlua::Lua>,
}

impl Pane {
    pub fn new(widget: Widget) -> Pane {
        Pane {
            retain: true,
            title: "Pane".to_string(),
            src: Vec::new(),
            lua_code: None,
            widget,
            value: Value::Null,
            lua: None,
        }
    }
    pub fn retain(&self) -> bool {
        self.retain
    }

    pub fn title(&self) -> String {
        self.title.clone()
    }



    pub fn process_lua(&mut self, _topic: &String, value: &Value) -> Result<Value> {
        if self.lua_code == None {
            return Ok(value.clone());
        }
        if self.lua.is_none() {
            let lua = mlua::Lua::new();
            /*if lua.load(self.lua_code.as_ref().unwrap()).exec().is_err() {
                info!("Error in lua code");
                return Ok(value.clone());
            }*/
            self.lua = Some(lua);
        }
        let r = self.lua
            .as_ref()
            .map(|lua| {
                let func:Function  = lua.load(self.lua_code.as_ref().unwrap()).eval().unwrap();
                let result = func.call::<Value>(value.clone());
    //            let process_data: mlua::Function = lua.globals().get("process_data")?;
    //            let result: String = process_data.call(value.clone().into_lua(lua))?;
    //            Ok(Value::String(result))
                result
            });
        Ok(r.unwrap() ?)
        }
}

fn get_endpoint(ui: &mut egui::Ui, endpoint: &EndPoint, cnt: usize) -> Option<EndPoint> {
    let mut options = get_possible_endpoints();
    options.sort();
    ui.label("Topic");
    let mut selected_value = endpoint.to_string();
    egui::ComboBox::from_id_salt(format!("topic{}", cnt))
        .selected_text(selected_value.to_string())
        .width(100.0)
        .show_ui(ui, |ui| {
            for option in options {
                ui.selectable_value::<String>(&mut selected_value, option.clone(), option);
            }
        });
    info!(
        "Selected value {:?} {:?}",
        selected_value,
        EndPoint::from_str(&selected_value).ok()
    );
    EndPoint::from_str(&selected_value).ok()
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

fn get_endpoints(ui: &mut egui::Ui, src: &mut Vec<EndPoint>) {
    ui.horizontal(|ui| {
        ui.label("Source topics");
        if ui.button("+").clicked() {
            src.push(EndPoint {
                topic: "topic".to_string(),
                field: None,
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
            get_endpoint(ui, ep, cnt).map(|e| *ep = e);
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
            get_endpoints(ui, &mut self.src);
            get_lua_filter(ui, &mut self.lua_code);
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
        let endpoints = self.src.clone();
        let _ = endpoints
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
                    if self
                        .process_lua(&topic, v)
                        .map(|v2| {
                            let v1 = v2.clone();
                            match v1 {
                                Value::String(s) => {
                                    info!("Processed value {:?} to widget ", s)
                                }
                                _ => {}
                            };
                            info!("Processed value {} {:?} to widget ", topic, v2);
                            self.widget.process_data(topic.clone(), &v2)
                        })
                        .is_err()
                    {
                        info!("Error in lua code");
                    };
                });
            })
            .collect::<Vec<_>>();
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
