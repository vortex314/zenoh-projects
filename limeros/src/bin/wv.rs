use anyhow::Ok;
use dashmap::DashMap;
use limeros::{UdpMessage, UdpMessageHandler, UdpNode};
use std::sync::Arc;
use winit::{
    application::ApplicationHandler,
    event::{Event, StartCause, WindowEvent},
    event_loop::{ActiveEventLoop, ControlFlow, EventLoop},
    window::{Window, WindowId},
};
use wry::WebViewBuilder;

// Data structure to pass to JS
#[derive(serde::Serialize, Clone)]
struct Record {
    pub src: String,
    pub msg_type: String,
    pub field_name: String,
    pub value: String,
}

#[derive(Default)]
struct App {
    window: Option<Window>,
    webview: Option<wry::WebView>,
    cache: Arc<DashMap<String, Record>>,
}

impl ApplicationHandler for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        // 2. Define our HTML + JS
        let html = r#"
        <!DOCTYPE html>
        <html>
            <style>
                body { font-family: sans-serif; background: #222; color: #eee; padding: 1rem; }
                table { width: 100%; border-collapse: collapse; }
                th, td { border: 1px solid #444; padding: 8px; text-align: left; }
                th { background: #333; color: #00ffcc; }
            </style>
            <body>
                <h2>UDP Live View</h2>
                <table>
                    <thead><tr><th>Source</th><th>Type</th><th>Field</th><th>Value</th></tr></thead>
                    <tbody id="table-body"></tbody>
                </table>
                <script>
                    // Function called from Rust
                    window.updateData = (data) => {
                        const tbody = document.getElementById('table-body');
                        tbody.innerHTML = data.map(r => `
                            <tr>
                                <td>${r.src}</td>
                                <td>${r.msg_type}</td>
                                <td>${r.field_name}</td>
                                <td>${r.value}</td>
                            </tr>
                        `).join('');
                    };
                </script>
            </body>
        </html>
    "#;
        let window = event_loop
            .create_window(Window::default_attributes())
            .unwrap();
        let webview = WebViewBuilder::new()
            .with_html(html)
            .build(&window)
            .unwrap();

        self.window = Some(window);
        self.webview = Some(webview);
    }

    fn window_event(
        &mut self,
        _event_loop: &ActiveEventLoop,
        _window_id: WindowId,
        event: WindowEvent,
    ) {
    }

    // Advance GTK event loop <!----- IMPORTANT
    fn about_to_wait(&mut self, _event_loop: &ActiveEventLoop) {
        #[cfg(target_os = "linux")]
        while gtk::events_pending() {
            gtk::main_iteration_do(false);
        }
    }
}

struct AppHandler {
    cache: Arc<DashMap<String, Record>>,
}
#[async_trait::async_trait]
impl UdpMessageHandler for AppHandler {
    async fn handle(&self, udp_message: &UdpMessage) -> anyhow::Result<()> {
        let payload = udp_message
            .payload
            .as_ref()
            .ok_or(anyhow::anyhow!("No payload"))?;
        let v: serde_json::Value = serde_json::from_slice(payload).unwrap_or_default();

        if let serde_json::Value::Object(map) = v {
            for (field_name, value) in map {
                let key = format!(
                    "{}:{}:{}",
                    udp_message.src.as_deref().unwrap_or("?"),
                    udp_message.msg_type.as_deref().unwrap_or("?"),
                    field_name
                );
                let record = Record {
                    src: udp_message.src.clone().unwrap_or_default(),
                    msg_type: udp_message.msg_type.clone().unwrap_or_default(),
                    field_name: field_name.clone(),
                    value: value.to_string(),
                };
                self.cache
                    .entry(key)
                    .and_modify(|e| {
                        *e = record.clone();
                    })
                    .or_insert(record);
            }
        }
        Ok(())
    }
}

fn main() -> anyhow::Result<()> {
    let cache = Arc::new(DashMap::new());

    gtk::init()?;
    let event_loop = EventLoop::new()?;
    let mut app = App::default();
    app.cache = cache.clone();
    event_loop.run_app(&mut app).unwrap();

    // 1. Initialize UdpNode in a background Runtime
    let rt = tokio::runtime::Runtime::new()?;
    let _guard = rt.enter();

    rt.spawn(async move {
        let node = UdpNode::new("webview-monitor", "239.0.0.1:50000")
            .await
            .unwrap();
        node.add_generic_handler(AppHandler {
            cache: cache.clone(),
        })
        .await;
        // Keep the node alive
        loop {
            tokio::time::sleep(std::time::Duration::from_secs(1)).await;
        }
    });

    Ok(())
}
