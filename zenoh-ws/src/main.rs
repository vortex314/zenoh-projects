use actix::ActorContext;
use actix::AsyncContext;
use actix_files::Files;
use actix_web::test;
use actix_web::{web, App, Error, HttpRequest, HttpResponse, HttpServer};
use actix_web_actors::ws;
use futures::StreamExt;
use log::debug;
use log::info;
use serde::Deserialize;
use serde_json::json;
use std::result::Result;
use std::time::{Duration, Instant};
use tokio::sync::{broadcast, mpsc};
// use zenoh::prelude::r#async::*;
use zenoh::*;
mod logger;
use logger::init;
mod msg;
use msg::{Load, Message, Publish, Save, Subscribe};

use crate::msg::test_serialization;

#[derive(Clone)]
struct ZenohSample {
    key: String,
    value: Vec<u8>,
}

#[derive(Clone)]
struct AppState {
    tx_broadcast: broadcast::Sender<ZenohSample>,
    tx_publish: mpsc::Sender<PublishRequest>,
}

#[derive(Debug)]
struct PublishRequest {
    key: String,
    payload: Vec<u8>,
}

#[derive(Deserialize)]
struct WsPublishMessage {
    topic: String,
    payload: String,
    #[serde(default)]
    base64: bool,
}

// === WebSocket Actor ===

struct WsActor {
    tx_publish: mpsc::Sender<PublishRequest>,
    rx_broadcast: broadcast::Receiver<ZenohSample>,
    hb: Instant,
}

impl WsActor {
    fn new(
        tx_publish: mpsc::Sender<PublishRequest>,
        rx_broadcast: broadcast::Receiver<ZenohSample>,
    ) -> Self {
        Self {
            tx_publish,
            rx_broadcast,
            hb: Instant::now(),
        }
    }

    fn start_broadcast(addr: actix::Addr<Self>, mut rx: broadcast::Receiver<ZenohSample>) {
        actix_rt::spawn(async move {
            while let Ok(sample) = rx.recv().await {
                addr.do_send(BroadcastMsg(sample));
            }
        });
    }

    fn start_heartbeat(&self, ctx: &mut ws::WebsocketContext<Self>) {
        ctx.run_interval(Duration::from_secs(10), |act, ctx| {
            if Instant::now().duration_since(act.hb) > Duration::from_secs(30) {
                ctx.stop();
                return;
            }
            ctx.ping(b"ping");
        });
    }
}

#[derive(actix::Message)]
#[rtype(result = "()")]
struct BroadcastMsg(ZenohSample);

impl actix::Actor for WsActor {
    type Context = ws::WebsocketContext<Self>;

    fn started(&mut self, ctx: &mut Self::Context) {
        Self::start_broadcast(ctx.address(), self.rx_broadcast.resubscribe());
        self.start_heartbeat(ctx);
    }
}

impl actix::Handler<BroadcastMsg> for WsActor {
    type Result = ();

    fn handle(&mut self, msg: BroadcastMsg, ctx: &mut Self::Context) {
        let str = String::from_utf8(msg.0.value.clone()).unwrap_or_default();
        let js: serde_json::Value = serde_json::from_str(&str).unwrap_or_default();
        debug!("Broadcasting Zenoh message to WebSocket client {:?}", msg.0.key);

        let json = serde_json::json!({
            "type": "Publish",
            "topic": msg.0.key,
            "payload": js,
        });
        if let Ok(txt) = serde_json::to_string(&json) {
            ctx.text(txt);
        }
    }
}

impl actix::StreamHandler<Result<ws::Message, ws::ProtocolError>> for WsActor {
    fn handle(&mut self, msg: Result<ws::Message, ws::ProtocolError>, ctx: &mut Self::Context) {
        info!("📩 Received WebSocket message {}", match &msg {
            Ok(m) => format!("{:?}", m),
            Err(e) => format!("Error: {:?}", e),
        });
        match msg {
            Ok(ws::Message::Text(txt)) => {
                if let Ok(cmd) = serde_json::from_str::<msg::Message>(&txt) {
                    info!("Parsed command: {:?}", cmd);
                    match cmd {
                        msg::Message::Publish(pub_msg) => {
                            info!("Processing Publish command for topic {} with payload {}", pub_msg.topic, pub_msg.payload);

                            let tx = self.tx_publish.clone();
                            actix_rt::spawn(async move {
                                let _ = tx
                                    .send(PublishRequest {
                                        key: pub_msg.topic,
                                        payload:pub_msg.payload.to_string().into_bytes(),
                                    })
                                    .await;
                            });
                        }
                        _ => {
                            info!("Unsupported command type");
                        }
                    }
                } else {
                    info!("Failed to parse WebSocket message as Command");
                }
            }
            Ok(ws::Message::Ping(p)) => ctx.pong(&p),
            Ok(ws::Message::Pong(_)) => self.hb = Instant::now(),
            Ok(ws::Message::Close(_)) => ctx.stop(),
            _ => { info!("Unsupported WebSocket message");()},
        }
    }
}
// assign new WebSocket handler actor
async fn ws_handler(
    req: HttpRequest,
    stream: web::Payload,
    state: web::Data<AppState>,
) -> Result<HttpResponse, Error> {
    let actor = WsActor::new(state.tx_publish.clone(), state.tx_broadcast.subscribe());
    info!("🛰️ New WebSocket connection established {:?}",req);

    ws::start(actor, &req, stream)
}

// #[actix_web::main]
#[tokio::main(flavor = "multi_thread", worker_threads = 1)]

async fn main() -> std::io::Result<()> {
    logger::init();
    test_serialization();
    // Channels
    let (tx_broadcast, _rx_broadcast) = broadcast::channel(128);
    let (tx_publish, rx_publish) = mpsc::channel(128);

    info!("🚀 Starting server...");
    // Spawn Zenoh async worker
    tokio::spawn(zenoh_worker(tx_broadcast.clone(), rx_publish));

    let state = web::Data::new(AppState {
        tx_broadcast,
        tx_publish,
    });

    info!("🌐 Serving on http://localhost:8080");

    HttpServer::new(move || {
        App::new()
            .app_data(state.clone())
            .route("/ws", web::get().to(ws_handler))
            .service(Files::new("/", "./static").index_file("index.html"))
    })
    .bind(("0.0.0.0", 8080))?
    .run()
    .await
}

async fn zenoh_worker(
    tx_broadcast: broadcast::Sender<ZenohSample>,
    mut rx_publish: mpsc::Receiver<PublishRequest>,
) {
    let mut config = zenoh::Config::default();
    config.insert_json5("mode", &json!("router").to_string()).unwrap();
    let session = zenoh::open(config).await.unwrap();
    info!("✅ Connected to Zenoh");

    // Subscribe to everything under "demo/**"
    let sub = session.declare_subscriber("src/**").await.unwrap();

    let tx_broadcast_clone = tx_broadcast.clone();

    // Zenoh subscription loop
    tokio::spawn(async move {
        let mut stream = sub.stream();
        while let Some(sample) = stream.next().await {
            debug!("Received Zenoh sample on key {}:{}", sample.key_expr(),sample.payload().try_to_string().unwrap());
            let data = ZenohSample {
                key: sample.key_expr().to_string(),
                value: sample.payload().to_bytes().to_vec(),
            };
            let _ = tx_broadcast_clone.send(data);
        }
    });

    // Publishing loop
    while let Some(req) = rx_publish.recv().await {
        info!("📤 Publishing to Zenoh key {}", req.key);
        let _ = session.put(&req.key, req.payload).await;
    }
}
