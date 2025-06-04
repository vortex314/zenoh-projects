#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
use axum::{
    Json, Router,
    extract::{
        State,
        ws::{WebSocket, WebSocketUpgrade},
    },
    response::IntoResponse,
    routing::{get, post},
};
use futures::{SinkExt, StreamExt};
use log::debug;
use log::error;
use log::{info, logger};
use serde::{Deserialize, Serialize};
use std::{
    collections::VecDeque,
    error::Error,
    net::{Ipv4Addr, SocketAddr, SocketAddrV4},
    sync::Arc,
};
use tokio::{
    net::UdpSocket,
    sync::{Mutex, mpsc},
};
mod logger; // Custom logger module
mod property_cache;

// Configuration
const UDP_LISTEN_ADDR: &str = "0.0.0.0:6502";
const UDP_SEND_ADDR: &str = "225.0.0.1:6502"; // Example multicast address
const HTTP_SERVER_ADDR: &str = "0.0.0.0:3000";
const MAX_CACHED_MESSAGES: usize = 100;

// Message storage
#[derive(Debug, Clone)]
struct AppState {
    udp_messages: Arc<Mutex<VecDeque<String>>>,
    udp_socket: Arc<UdpSocket>,
}

// For sending UDP messages via HTTP
#[derive(Debug, Deserialize)]
struct UdpMessage {
    message: String,
    target: Option<String>,
}

#[derive(Debug, Serialize)]
struct ApiResponse {
    success: bool,
    message: String,
}

#[tokio::main]
async fn main() {
    logger::init(); // Log to stderr (if you run with `RUST_LOG=debug`);
    info!("Starting UDP/HTTP server...");
    // Create UDP socket

    let udp_socket = UdpSocket::bind("0.0.0.0:6502")
        .await
        .expect("Failed to create std UDP socket");
    udp_socket
        .join_multicast_v4(Ipv4Addr::new(225, 0, 0, 1), Ipv4Addr::new(0, 0, 0, 0))
        .expect("Failed to join multicast group");

    let udp_socket = Arc::new(udp_socket);
    let udp_messages = Arc::new(Mutex::new(VecDeque::with_capacity(MAX_CACHED_MESSAGES)));

    // Clone for UDP receiver task
    let udp_receiver_socket = udp_socket.clone();
    let udp_receiver_messages = udp_messages.clone();

    // Spawn UDP receiver task
    tokio::spawn(async move {
        let mut buf = [0; 1024];
        loop {
            match udp_receiver_socket.recv_from(&mut buf).await {
                Ok((len, src)) => {
                    let message = String::from_utf8_lossy(&buf[..len]).to_string();
                    info!("Received UDP from {}: {}", src, message);

                    let mut messages = udp_receiver_messages.lock().await;
                    if messages.len() >= MAX_CACHED_MESSAGES {
                        messages.pop_front();
                    }
                    messages.push_back(message);
                }
                Err(e) => error!("UDP receive error: {}", e),
            }
        }
    });

    // Create shared state
    let state = AppState {
        udp_messages,
        udp_socket: udp_socket.clone(),
    };

    // Build HTTP server
    let app = Router::new()
        .route("/ws", get(websocket_handler))
        .route("/send", post(send_udp_message))
        .route("/messages", get(get_cached_messages))
        .with_state(state);
    info!("HTTP server listening on {}", HTTP_SERVER_ADDR);
    let listener = tokio::net::TcpListener::bind(HTTP_SERVER_ADDR)
        .await
        .unwrap();
    info!("listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

// WebSocket handler for real-time message streaming
async fn websocket_handler(
    ws: WebSocketUpgrade,
    State(state): State<AppState>,
) -> impl IntoResponse {
    ws.on_upgrade(|socket| async move { handle_websocket_connection(socket, state).await })
}

async fn handle_websocket_connection(mut socket: WebSocket, state: AppState) {
    // Send initial cached messages
    {
        let messages = state.udp_messages.lock().await;
        for msg in messages.iter() {
            if let Err(e) = socket
                .send(axum::extract::ws::Message::Text(msg.clone().into()))
                .await
            {
                error!("WebSocket send error: {}", e);
                return;
            }
        }
    }

    // Channel for new messages
    let (tx, mut rx) = mpsc::channel(32);
    let mut message_rx = tokio::spawn(async move {
        while let Some(msg) = rx.recv().await {
            if let Err(e) = socket.send(axum::extract::ws::Message::Text(msg)).await {
                error!("WebSocket send error: {}", e);
                break;
            }
        }
    });

    // Watch for new UDP messages
    let mut last_count = state.udp_messages.lock().await.len();
    loop {
        tokio::time::sleep(std::time::Duration::from_millis(100)).await;

        let messages = state.udp_messages.lock().await;
        if messages.len() > last_count {
            for msg in messages.range(last_count..) {
                if tx.send(msg.clone().into()).await.is_err() {
                    break;
                }
            }
            last_count = messages.len();
        }
    }

    message_rx.abort();
}

// HTTP endpoint to send UDP messages
async fn send_udp_message(
    State(state): State<AppState>,
    Json(payload): Json<UdpMessage>,
) -> Json<ApiResponse> {
    let target = payload.target.as_deref().unwrap_or(UDP_SEND_ADDR);

    match state
        .udp_socket
        .send_to(payload.message.as_bytes(), target)
        .await
    {
        Ok(_) => Json(ApiResponse {
            success: true,
            message: format!("Message sent to {}", target),
        }),
        Err(e) => Json(ApiResponse {
            success: false,
            message: format!("Failed to send message: {}", e),
        }),
    }
}

// HTTP endpoint to get cached messages
async fn get_cached_messages(State(state): State<AppState>) -> Json<Vec<String>> {
    let messages = state.udp_messages.lock().await;
    Json(messages.iter().cloned().collect())
}
