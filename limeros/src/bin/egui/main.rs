use dashmap::DashMap;
use eframe::egui::{self};
use limeros::{
    Alive, Endpoint, Msg, TypedUdpMessage, UdpMessage, UdpMessageHandler, UdpNode, eventbus::{Bus, as_message}, logger, msgs::{HoverboardCmd, HoverboardEvent, SysEvent, TypedMessage, WifiEvent}
};
use log::info;
use socket2::Socket;
use std::{any::Any, collections::HashSet, ops::RangeInclusive, time::SystemTime};
use std::{sync::Arc, time::Instant};
use tokio::sync::Mutex;

mod my_window;
use my_window::MyWindow;
mod window_endpoints;
mod window_events;
use window_endpoints::WindowEndpoints;
use window_events::WindowEvents;
mod window_gauge;
mod window_hoverboard;
mod window_plot;
use window_hoverboard::HoverboardWindow;
use limeros::eventbus::Eventbus;
use limeros::eventbus::ActorImpl;

mod widget_alive;
use rand::prelude::*;


// --- 1. Data Structures ---
struct MetricData {
    name: String,
    points: Vec<[f64; 2]>,
    start_time: std::time::Instant,
}

// --- 1. Shared Data Structure ---

#[derive(Clone)]
struct Record {
    pub timestamp: SystemTime,
    pub src: String,
    pub msg_type: String,
    pub field_name: String,
    pub value: String,
    pub counter : u64,
}

// --- 2. The egui Application State ---

struct UdpMonitorApp {
    cache: Arc<DashMap<String, Record>>,
    node: Arc<UdpNode>,
    bus : Bus,

    graph_data: Arc<DashMap<String, MetricData>>,
    selected_fields: HashSet<String>,
    window_hoverboard: HoverboardWindow,
    window_events: window_events::WindowEvents,
    window_endpoints: window_endpoints::WindowEndpoints,
    window_others: Arc<DashMap<String, Box<dyn MyWindow>>>,
}

impl UdpMonitorApp {
    fn new(
        cc: &eframe::CreationContext<'_>,
        node: Arc<UdpNode>,
        bus : Bus,
        cache: Arc<DashMap<String, Record>>,
        endpoints: Arc<DashMap<String, Endpoint>>,
        graph_data: Arc<DashMap<String, MetricData>>,
    ) -> Self {
        let node = node.clone();
        let cache = cache.clone();
        let endpoints = endpoints.clone();
        let graph_data = graph_data.clone();
        let window_others = Arc::new(DashMap::new());
        Self {
            node: node.clone(),
            bus: bus.clone(),
            cache: cache.clone(),
            window_events: WindowEvents::new(
                cache.clone(),
                window_others.clone(),
                graph_data.clone(),
            ),
            window_endpoints: WindowEndpoints::new(node.clone(), endpoints.clone()),
            graph_data: graph_data.clone(),
            selected_fields: HashSet::new(),
            window_hoverboard: HoverboardWindow::new(node.clone()),
            window_others: window_others.clone(),
        }
    }
}

// --- 3. UI Rendering Logic ---

impl eframe::App for UdpMonitorApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            self.window_endpoints
                .show(ui)
                .expect("Failed to show Endpoints window");
            self.window_events
                .show(ui)
                .expect("Failed to show Events window");
            self.window_hoverboard
                .show(ui)
                .expect("Failed to show Hoverboard window");
            for mut window in self.window_others.iter_mut() {
                window.show(ui).expect("Failed to show other window");
                if window.is_closed() {
                    self.window_others.remove(window.key());
                }
            }
        });

        // Continuous refresh to see live UDP updates
        ctx.request_repaint_after(std::time::Duration::from_millis(100));
    }

    fn save(&mut self, _storage: &mut dyn eframe::Storage) {
        // Implement if you want to save app state
    }
}

impl UdpMonitorApp {}

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
            let mut record = Record {
                timestamp: SystemTime::now(),
                src: udp_message.src.clone().unwrap_or_default(),
                msg_type: udp_message.msg_type.clone().unwrap_or_default(),
                field_name: field_name.clone(),
                value: value.clone(),
                counter : 1,
            };
            self.cache
                .entry(key)
                .and_modify(|e| {
                    record.counter += e.counter ;
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

struct EventbusActor {
    eb: Bus,
    node : Arc<UdpNode>,
}
#[async_trait::async_trait]
impl ActorImpl for EventbusActor {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>)  {
        info!("EventbusActor received a message: {:?}", msg);
        // serialize message and send over UDP if needed
    }
}

#[async_trait::async_trait]
impl UdpMessageHandler for EventbusActor {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        // deserialize UDP message and emit to eventbus
        self.eb.emit(udp_message.clone());
        Ok(())
    }
}

unsafe impl Send for EventbusActor {}
unsafe impl Sync for EventbusActor {}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    logger::init();

    let eb = Eventbus::new();

    eb.bus().emit(Alive::default());

    let cache = Arc::new(DashMap::new());
    let graph_data = Arc::new(DashMap::new());

    let mut rng = rand::rng();
    let rand_u16: u16 = rng.random();

    let node_name = format!("egui-monitor-{}", rand_u16);

    // Initialize Node and Handler inside the runtime
    let n = UdpNode::new(node_name.as_str(), "239.0.0.1:50000")
        .await
        .unwrap();
    /*n.add_subscription(SysEvent::MSG_TYPE).await;
    n.add_subscription(WifiEvent::MSG_TYPE).await;
    n.add_subscription(HoverboardEvent::MSG_TYPE).await;*/
    n.add_subscription("*").await;
    n.add_generic_handler(GuiHandler {
        cache: cache.clone(),
        graph_data: graph_data.clone(),
        start_time: std::time::Instant::now(),
    })
    .await;
    let node = n.clone();
    let endpoints = node.get_endpoints().await;

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([1024.0, 768.0]), //            .with_position([100.0, 100.0])
        ..Default::default()
    };
    let r = eframe::run_native(
        "Limeros UDP Monitor",
        options,
        Box::new(|cc| {
            Ok(Box::new(UdpMonitorApp::new(
                cc, node,eb.bus(), cache, endpoints, graph_data,
            )))
        }),
    );
    info!("UDP Monitor GUI Ended {:?}", r);

    Ok(())
}
