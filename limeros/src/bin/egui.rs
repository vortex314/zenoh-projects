use dashmap::DashMap;
use eframe::egui;
use egui::Widget;
use egui_plot::{Line, Plot, PlotPoints};
use ehmi::{Bar, Gauge, ToggleStyle, ToggleSwitch};
use gtk::{atk::Range, gdk::keys::constants::w};
use limeros::{
    logger,
    msgs::{HoverboardEvent, SysEvent, TypedMessage, WifiEvent},
    Endpoint, UdpMessage, UdpMessageHandler, UdpNode,
};
use log::info;
use socket2::Socket;
use std::{collections::HashSet, ops::RangeInclusive, time::SystemTime};
use std::{sync::Arc, time::Instant};

// --- 1. Shared Data Structure ---

#[derive(Clone)]
struct Record {
    pub timestamp: SystemTime,
    pub src: String,
    pub msg_type: String,
    pub field_name: String,
    pub value: String,
}

#[derive(PartialEq)]
enum Tab {
    Endpoints,
    Events,
}

#[derive(PartialEq, Clone, Copy)]
enum SortColumn {
    Time,
    Source,
    Type,
    Field,
}

struct MetricData {
    name: String,
    points: Vec<[f64; 2]>,
    start_time: std::time::Instant,
}
// --- 2. The egui Application State ---

struct UdpMonitorApp {
    cache: Arc<DashMap<String, Record>>,
    node: Arc<UdpNode>,
    current_tab: Tab,
    // Sorting state
    sort_col: SortColumn,
    sort_desc: bool,
    // Local copy of endpoints to avoid async blocking in UI thread
    endpoints: Arc<DashMap<String, Endpoint>>,
    // Graph data: Map from field name to Vec<(x, y)>
    start_time: std::time::Instant,

    widget_windows: DashMap<String, HashSet<String>>,
    graph_data: Arc<DashMap<String, MetricData>>,
    selected_fields: HashSet<String>,
}

impl UdpMonitorApp {
    fn new(
        cc: &eframe::CreationContext<'_>,
        node: Arc<UdpNode>,
        cache: Arc<DashMap<String, Record>>,
        endpoints: Arc<DashMap<String, Endpoint>>,
        graph_data: Arc<DashMap<String, MetricData>>,
    ) -> Self {
        Self {
            node,
            cache,
            current_tab: Tab::Endpoints,
            sort_col: SortColumn::Time,
            sort_desc: true,
            endpoints,
            widget_windows: DashMap::new(),
            start_time: std::time::Instant::now(),
            graph_data,
            selected_fields: HashSet::new(),
        }
    }
}

// --- 3. UI Rendering Logic ---

impl eframe::App for UdpMonitorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Periodically refresh endpoints (non-blocking try_lock or similar)
        // For simplicity in this example, we'd ideally use a channel or shared Arc

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("UDP Node Monitor");

            // Tab Selection
            ui.horizontal(|ui| {
                ui.selectable_value(&mut self.current_tab, Tab::Endpoints, "Endpoints");
                ui.selectable_value(&mut self.current_tab, Tab::Events, "Live Events");
            });

            ui.separator();

            match self.current_tab {
                Tab::Endpoints => self.render_endpoints(ui),
                Tab::Events => self.render_events(ui),
            }
        });

        // Continuous refresh to see live UDP updates
        ctx.request_repaint_after(std::time::Duration::from_millis(100));
    }
}

impl UdpMonitorApp {
    fn render_endpoints(&mut self, ui: &mut egui::Ui) {
        egui::Grid::new("endpoint_grid")
            .striped(true)
            .show(ui, |ui| {
                ui.label("Peer ID");
                ui.label("Address");
                ui.label("Subscribe");
                ui.label("Publish");
                ui.label("Services");
                ui.label("Last Seen");
                ui.end_row();

                for entry in self.endpoints.iter() {
                    let ep = entry.value();
                    let secs_ago = Instant::now().duration_since(ep.last_seen).as_millis() / 1000;

                    ui.label(entry.key());
                    ui.label(ep.addr.to_string());
                    ui.label(format!("{:?}", ep.subscribe));
                    ui.label(format!("{:?}", ep.publish));
                    ui.label(format!("{:?}", ep.services));
                    ui.label(format!("{} sec ago", secs_ago));
                    ui.end_row();
                }
            });
    }

    fn render_events(&mut self, ui: &mut egui::Ui) {
        egui::ScrollArea::vertical().show(ui, |ui| {
            use egui_extras::{Column, TableBuilder};

            let mut records: Vec<Record> = self.cache.iter().map(|e| e.value().clone()).collect();

            // Sort logic
            records.sort_by(|a, b| {
                let ord = match self.sort_col {
                    SortColumn::Time => a.timestamp.cmp(&b.timestamp),
                    SortColumn::Source => a.src.cmp(&b.src),
                    SortColumn::Type => a.msg_type.cmp(&b.msg_type),
                    SortColumn::Field => a.field_name.cmp(&b.field_name),
                };
                if self.sort_desc {
                    ord.reverse()
                } else {
                    ord
                }
            });

            TableBuilder::new(ui)
                .striped(true)
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::initial(80.0))
                .column(Column::remainder())
                .header(20.0, |mut header| {
                    header.col(|ui| {
                        ui.label("Time");
                    });
                    header.col(|ui| {
                        if ui.label("Source").clicked() {
                            self.toggle_sort(SortColumn::Source);
                        }
                    });
                    header.col(|ui| {
                        if ui.label("Type").clicked() {
                            self.toggle_sort(SortColumn::Type);
                        }
                    });
                    header.col(|ui| {
                        if ui.label("Field").clicked() {
                            self.toggle_sort(SortColumn::Field);
                        }
                    });
                    header.col(|ui| {
                        ui.label("Value");
                    });
                    header.col(|ui| {
                        ui.label("Functions");
                    });
                })
                .body(|body| {
                    body.rows(20.0, records.len(), |mut row| {
                        let r = &records[row.index()];
                        let time_str = chrono::DateTime::<chrono::Local>::from(r.timestamp)
                            .format("%H:%M:%S")
                            .to_string();
                        let key = format!("{}:{}:{}", r.src, r.msg_type, r.field_name);

                        row.col(|ui| {
                            ui.label(time_str);
                        });
                        row.col(|ui| {
                            ui.label(&r.src);
                        });
                        row.col(|ui| {
                            ui.label(&r.msg_type);
                        });
                        row.col(|ui| {
                            ui.label(&r.field_name);
                        });

                        row.col(|ui| {
                            ui.horizontal(|ui| {
                                let mut selected = self.selected_fields.contains(key.as_str());
                                if ui.checkbox(&mut selected, "").changed() {
                                    if selected {
                                        self.selected_fields.insert(key.clone());
                                    } else {
                                        self.selected_fields.remove(&key);
                                    }
                                    }
                                
                                if ui.button("📈").clicked() {
                                    // Could implement functionality to highlight/select this field for graphing
                                    self.widget_windows.entry("plot".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("⭕").clicked() {
                                    self.widget_windows.entry("gauge".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("|").clicked() {
                                    self.widget_windows.entry("bar".to_string()).or_default().insert(key.clone());
                                }
                                if ui.button("❌").clicked() {
                                    self.widget_windows.entry("plot".to_string()).or_default().remove(&key);
                                    self.widget_windows.entry("gauge".to_string()).or_default().remove(&key);
                                    self.widget_windows.entry("bar".to_string()).or_default().remove(&key);
                                }
                            });
                        });
                        row.col(|ui| {
                            ui.label(&r.value);
                        });
                    });
                });
        });
        // Render open graph windows
        self.widget_windows.iter().for_each(|entry| {
            let widget_type = entry.key();
            let keys: Vec<String> = entry.value().iter().cloned().collect();
            for key in keys {
                if let Some(metric_data) = self.graph_data.get(&key) {
                    match widget_type.as_str() {
                        "plot" => self.show_plot_window(ui.ctx(), metric_data.value()),
                        "gauge" => {
                            let value = self.cache.get(&key);
                            if let Some(record) = value {
                                if let Ok(num) = record.value.parse::<f32>() {
                                    self.show_ehmi_gauge(ui.ctx(), num, &metric_data);
                                }
                            }
                        }
                        "bar" => {
                            let value = self.cache.get(&key);
                            if let Some(record) = value {
                                if let Ok(num) = record.value.parse::<f32>() {
                                    self.show_ehmi_bar(ui.ctx(), num, &metric_data);
                                }
                            }
                        }
                        _ => {}
                    }
                }
            }
        });
        
    }

    fn calculate_gauge_max_min(min: f64, max: f64) -> RangeInclusive<f64> {
        let range = max - min;
        let magnitude = 10f64.powf(range.log10().floor());
        let nice_min = (min / magnitude).floor() * magnitude;
        let nice_max = (max / magnitude).ceil() * magnitude;
        nice_min..=nice_max
    }

    fn show_ehmi_bar(&self, ctx: &egui::Context, value: f32, metric_data: &MetricData) {
         let window_name = metric_data.name.as_str();
        let title = window_name.to_string();
        let window_hash = format!("{}_bar", window_name);
        let viewport_id = egui::ViewportId::from_hash_of(&window_hash);
        let margin = 25.0;
        let hsize = 400.0;
        let vsize = 100.0;
        let offset_rectangle = egui::Rect::from_min_size(
            egui::pos2(margin, margin),
            egui::vec2(hsize - margin, vsize - margin),
        );
        let (min, mut max) =
            metric_data
                .points
                .iter()
                .fold((f32::MAX, f32::MIN), |(min, max), p| {
                    let v = p[1] as f32;
                    (min.min(v), max.max(v))
                });
        if min == max {
            max += 1.0;
        }
        let value_range = Self::calculate_gauge_max_min(min as f64 , max as f64);
        let value_range = *value_range.start() as f32 ..= *value_range.end() as f32;

        ctx.show_viewport_immediate(
            viewport_id,
            egui::ViewportBuilder::default()
                .with_title(title)
                .with_inner_size([hsize, vsize]),
            |ctx, class| {
                assert!(class == egui::ViewportClass::Immediate);
                let value_range = value_range.clone();
                let window_name = window_name.to_string();

                egui::CentralPanel::default().show(ctx, |ui| {
                    // 1. Logic to handle the window's close button (Native X)
                    if ui.input(|i| i.viewport().close_requested()) {
                        // Logic to remove this window from your 'open_windows' set
                        self.widget_windows.entry("bar".to_string()).or_default().remove(&window_name);
                    }
                    // 2. Render the Gauge
                    ui.put(
                        offset_rectangle,
                        ehmi::Bar::new(value)
                         //   .bar_size(size - margin * 2.0)
                            .range(value_range)
                    );
                });
            },
        );
    }

    fn show_ehmi_gauge(&self, ctx: &egui::Context, value: f32, metric_data: &MetricData) {
        let window_name = metric_data.name.as_str();
        let title = window_name.to_string();
        let window_hash = format!("{}_gauge", window_name);
        let viewport_id = egui::ViewportId::from_hash_of(&window_hash);
        let margin = 25.0;
        let size = 400.0;
        let offset_rectangle = egui::Rect::from_min_size(
            egui::pos2(margin, margin),
            egui::vec2(size - margin, size - margin),
        );
        let (min, mut max) =
            metric_data
                .points
                .iter()
                .fold((f64::MAX, f64::MIN), |(min, max), p| {
                    let v = p[1] as f64;
                    (min.min(v), max.max(v))
                });
        if min == max {
            max += 1.0;
        }
        let value_range = Self::calculate_gauge_max_min(min, max);

        ctx.show_viewport_immediate(
            viewport_id,
            egui::ViewportBuilder::default()
                .with_title(title)
                .with_inner_size([size, size]),
            |ctx, class| {
                assert!(class == egui::ViewportClass::Immediate);
                let value_range = value_range.clone();

                egui::CentralPanel::default().show(ctx, |ui| {
                    // 1. Logic to handle the window's close button (Native X)
                    if ui.input(|i| i.viewport().close_requested()) {
                        // Logic to remove this window from your 'open_windows' set
                        // self.open_windows.remove(&window_name);
                    }
                    // 2. Render the Gauge
                    ui.put(
                        offset_rectangle,
                        Gauge::new(value)
                            .size(size - margin * 2.0)
                            .range(value_range)
                            .stroke_width(5.0)
                            .angle_range(-45i16..=225i16),
                    );
                });
            },
        );
    }

    fn show_plot_window(
        &self, // Changed to &mut to allow updating visibility state
        ctx: &egui::Context,
        metric_data: &MetricData,
    ) {
        let window_name = metric_data.name.as_str();
        let window_hash = format!("{}_plot", window_name);
        let title = window_name.to_string();
        let viewport_id = egui::ViewportId::from_hash_of(&window_hash);

        ctx.show_viewport_immediate(
            viewport_id,
            egui::ViewportBuilder::default()
                .with_title(&title)
                .with_inner_size([600.0, 400.0]),
            |ctx, class| {
                assert!(class == egui::ViewportClass::Immediate);

                egui::CentralPanel::default().show(ctx, |ui| {
                    // 1. Logic to handle the window's close button (Native X)
                    if ui.input(|i| i.viewport().close_requested()) {
                        // Logic to remove this window from your 'open_windows' set
                        // self.open_windows.remove(&window_name);
                    }

                    // 2. Render the Plot
                    let points = metric_data.points.clone();
                    let plot_points: PlotPoints = PlotPoints::from(points);
                    let line = Line::new(plot_points).name(&window_name);

                    Plot::new(&window_name)
                        .view_aspect(2.0)
                        .show(ui, |plot_ui| {
                            plot_ui.line(line);
                        });
                });
            },
        );
    }

    /*fn show_plot_window(
        &self,
        ctx: &egui::Context,
        metric_data: &MetricData,
        start_time: std::time::Instant,
    ) {
        let window_name = metric_data.name.clone();
        let mut is_visible = true;

        egui::Window::new(&window_name)
            .open(&mut is_visible) // "X" button sets is_visible to false
            .show(ctx, |ui| {
                let points = metric_data.points.clone();
                let plot_points: PlotPoints = PlotPoints::from(points);
                let line = Line::new(window_name.clone(), plot_points).name(&window_name);
                Plot::new(&window_name)
                    .view_aspect(2.0)
                    .show(ui, |plot_ui| {
                        plot_ui.line(line);
                    });
            });
    }*/

    fn toggle_sort(&mut self, col: SortColumn) {
        if self.sort_col == col {
            self.sort_desc = !self.sort_desc;
        } else {
            self.sort_col = col;
            self.sort_desc = true;
        }
    }
}

// --- 4. Main Entry Point ---

struct GuiHandler {
    cache: Arc<DashMap<String, Record>>,
    graph_data: Arc<DashMap<String, MetricData>>,
    start_time: std::time::Instant,
}

#[async_trait::async_trait]
impl UdpMessageHandler for GuiHandler {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        // Parse the payload (assuming JSON as per your UdpNode implementation)
        let fields = if let Some(payload) = &udp_message.payload {
            let v: serde_json::Value = serde_json::from_slice(payload).unwrap_or_default();
            if let serde_json::Value::Object(map) = v {
                map.into_iter().map(|(k, v)| (k, v.to_string())).collect()
            } else {
                vec![(
                    "raw".to_string(),
                    String::from_utf8_lossy(payload).into_owned(),
                )]
            }
        } else {
            vec![]
        };

        for (field_name, value) in &fields {
            let key = format!(
                "{}:{}:{}",
                udp_message.src.clone().unwrap_or_default(),
                udp_message.msg_type.clone().unwrap_or_default(),
                field_name
            );
            let record = Record {
                timestamp: SystemTime::now(),
                src: udp_message.src.clone().unwrap_or_default(),
                msg_type: udp_message.msg_type.clone().unwrap_or_default(),
                field_name: field_name.clone(),
                value: value.clone(),
            };
            self.cache
                .entry(key)
                .and_modify(|e| {
                    *e = record.clone();
                })
                .or_insert(record);
        }
        let now = std::time::Instant::now()
            .duration_since(self.start_time)
            .as_secs_f64();

        for (field_name, value) in &fields {
            // Try to parse value as a number for graphing
            if let Ok(num) = value.parse::<f64>() {
                let key = format!(
                    "{}:{}:{}",
                    udp_message.src.as_deref().unwrap_or("?"),
                    udp_message.msg_type.as_deref().unwrap_or("?"),
                    field_name
                );
                let key_clone = key.clone();

                self.graph_data
                    .entry(key)
                    .or_insert_with(|| MetricData {
                        name: key_clone.clone(),
                        start_time: self.start_time,
                        points: Vec::new(),
                    })
                    .points
                    .push([now, num]);

                // Optional: Limit history to last 200 points to save memory
                let mut entry = self.graph_data.get_mut(&key_clone).unwrap();
                if entry.points.len() > 1000 {
                    entry.points.remove(0);
                }
            }
        }
        Ok(())
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    logger::init();

    let cache = Arc::new(DashMap::new());
    let graph_data = Arc::new(DashMap::new());

    // Initialize Node and Handler inside the runtime
    let n = UdpNode::new("egui-monitor", "239.0.0.1:50000")
        .await
        .unwrap();
    n.add_subscription(SysEvent::MSG_TYPE).await;
    n.add_subscription(WifiEvent::MSG_TYPE).await;
    n.add_subscription(HoverboardEvent::MSG_TYPE).await;
    n.add_generic_handler(GuiHandler {
        cache: cache.clone(),
        graph_data: graph_data.clone(),
        start_time: std::time::Instant::now(),
    })
    .await;
    let node = n.clone();
    let endpoints = node.get_endpoints().await;

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([800.0, 600.0])
            .with_position([100.0, 100.0]),
        ..Default::default()
    };
    let r = eframe::run_native(
        "Limeros UDP Monitor",
        options,
        Box::new(|cc| {
            Ok(Box::new(UdpMonitorApp::new(
                cc, node, cache, endpoints, graph_data,
            )))
        }),
    );
    info!("UDP Monitor GUI Ended {:?}", r);

    Ok(())
}
