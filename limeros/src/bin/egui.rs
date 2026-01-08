use dashmap::DashMap;
use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints};
use limeros::{
    Endpoint, UdpMessage, UdpMessageHandler, UdpNode, logger, msgs::{HoverboardEvent, SysEvent, TypedMessage, WifiEvent}
};
use log::info;
use socket2::Socket;
use std::time::SystemTime;
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
    Graphs,
}

#[derive(PartialEq, Clone, Copy)]
enum SortColumn {
    Time,
    Source,
    Type,
    Field,
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
    graph_data: Arc<DashMap<String, Vec<[f64; 2]>>>,
    start_time: std::time::Instant,
}

impl UdpMonitorApp {
    fn new(
        cc: &eframe::CreationContext<'_>,
        node: Arc<UdpNode>,
        cache: Arc<DashMap<String, Record>>,
        endpoints: Arc<DashMap<String, Endpoint>>,
        graph_data: Arc<DashMap<String, Vec<[f64; 2]>>>,
    ) -> Self {
        Self {
            node,
            cache,
            current_tab: Tab::Endpoints,
            sort_col: SortColumn::Time,
            sort_desc: true,
            endpoints,
            graph_data,
            start_time: std::time::Instant::now(),
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
                ui.selectable_value(&mut self.current_tab, Tab::Graphs, "Telemetry Graph");
                // New
            });

            ui.separator();

            match self.current_tab {
                Tab::Endpoints => self.render_endpoints(ui),
                Tab::Events => self.render_events(ui),
                Tab::Graphs => self.render_graphs(ui), // New
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
                            ui.label(&r.value);
                        });
                        row.col(|ui| {
                            if ui.button("Graph").clicked() {
                                // Could implement functionality to highlight/select this field for graphing
                                // open new window with graph of this field
                                


                            }});
                    });
                });
        });
    }

    fn render_graphs(&mut self, ui: &mut egui::Ui) {
        ui.label("Real-time Telemetry (Numeric Fields)");

        let plot = Plot::new("udp_plot")
            .view_aspect(2.0)
            .legend(egui_plot::Legend::default())
            .x_axis_label("Seconds since start")
            .y_axis_label("Value");

        plot.show(ui, |plot_ui| {
            for entry in self.graph_data.iter() {
                let (name, points) = entry.pair();

                // Convert your Vec<[f64; 2]> into PlotPoints
                let plot_points = PlotPoints::from(points.clone());

                // Pass the points twice: once for the data, once for the series definition
                let line = Line::new(name.clone(), plot_points).name(name);

                plot_ui.line(line);
            }
        });

        if ui.button("Clear Graphs").clicked() {
            self.graph_data.clear();
        }
    }

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
    graph_data: Arc<DashMap<String, Vec<[f64; 2]>>>,
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
                    .or_insert_with(Vec::new)
                    .push([now, num]);

                // Optional: Limit history to last 200 points to save memory
                let mut entry = self.graph_data.get_mut(&key_clone).unwrap();
                if entry.len() > 200 {
                    entry.remove(0);
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
    n.add_subscription(WifiEvent::MSG_TYPE) .await;
    n.add_subscription(HoverboardEvent::MSG_TYPE).await;
    n.add_generic_handler(GuiHandler {
        cache: cache.clone(),
        graph_data: graph_data.clone(),
        start_time: std::time::Instant::now(),
    })
    .await;
    let node = n.clone();
    let endpoints = node.get_endpoints().await;

    let options = eframe::NativeOptions::default();
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
