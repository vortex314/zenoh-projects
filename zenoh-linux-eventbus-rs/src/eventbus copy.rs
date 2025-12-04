use tokio::sync::{broadcast, mpsc};
use tokio::time::{sleep, Duration};

use std::any::TypeId;
use std::sync::Arc;

#[derive(Clone)]
pub struct MessageEnvelope {
    pub type_id: TypeId,
    pub src: String,
    pub dst: Option<String>,
    payload: Arc<dyn std::any::Any + Send + Sync>,
}

impl MessageEnvelope {
    pub fn new<T: Send + Sync + 'static>(
        src: impl Into<String>,
        dst: Option<String>,
        payload: T,
    ) -> Self {
        Self {
            type_id: TypeId::of::<T>(),
            src: src.into(),
            dst,
            payload: Arc::new(payload),
        }
    }

    pub fn downcast<T: Send + Sync + 'static>(&self) -> Option<&T> {
        if self.type_id == TypeId::of::<T>() {
            self.payload.downcast_ref::<T>()
        } else {
            None
        }
    }
}


#[derive(Clone)]
pub struct EventBus {
    tx: broadcast::Sender<MessageEnvelope>,
}

impl EventBus {
    pub fn new(buffer: usize) -> Self {
        let (tx, _) = broadcast::channel(buffer);
        Self { tx }
    }

    pub fn publish<T: Send + Sync + 'static>(
        &self,
        src: impl Into<String>,
        dst: Option<String>,
        msg: T,
    ) {
        let _ = self.tx.send(MessageEnvelope::new(src, dst, msg));
    }

    /// Subscribe with an `id` (actor name) and a type filter
    pub fn subscribe<T: Send + Sync + Clone + 'static>(
        &self,
        my_id: impl Into<String>,
    ) -> mpsc::Receiver<T> {
        let my_id = my_id.into();
        let mut rx = self.tx.subscribe();
        let (out_tx, out_rx) = mpsc::channel(16);

        tokio::spawn(async move {
            while let Ok(env) = rx.recv().await {
                // filter by type
                if let Some(ev) = env.downcast::<T>() {
                    // filter by dst
                    if env.dst.as_deref().map_or(true, |d| d == my_id) {
                        if out_tx.send(ev.clone()).await.is_err() {
                            break;
                        }
                    }
                }
            }
        });

        out_rx
    }

    /// Spawn a timer that delivers a message to an actor (dst)
    pub fn spawn_timer<T: Send + Sync + Clone + 'static>(
        &self,
        src: impl Into<String>,
        dst: Option<String>,
        event: T,
        interval: Duration,
    ) {
        let bus = self.clone();
        let src = src.into();
        tokio::spawn(async move {
            loop {
                sleep(interval).await;
                bus.publish(src.clone(), dst.clone(), event.clone());
            }
        });
    }
}


#[derive(Clone, Debug)]
struct PoseUpdate {
    x: f32,
    y: f32,
    heading: f32,
}

#[derive(Clone, Debug)]
struct TimerFired;

async fn planner(mut rx: mpsc::Receiver<PoseUpdate>, my_id: String) {
    while let Some(msg) = rx.recv().await {
        println!("[{}] Planner got pose: {:?}", my_id, msg);
    }
}

#[tokio::main]
async fn main() {
    let bus = EventBus::new(32);

    // Start planner actor with id "planner"
    let planner_rx = bus.subscribe::<PoseUpdate>("planner");
    tokio::spawn(planner(planner_rx, "planner".into()));

    // Timer sends to planner every 2s
    bus.spawn_timer("bus", Some("planner".into()), TimerFired, Duration::from_secs(2));

    // Broadcast PoseUpdate (all listeners of that type will see it)
    for i in 0..5 {
        bus.publish(
            "robot",
            None, // broadcast
            PoseUpdate { x: i as f32, y: i as f32 * 0.5, heading: 90.0 },
        );
        sleep(Duration::from_secs(1)).await;
    }

    // Direct message just to planner
    bus.publish(
        "controller",
        Some("planner".into()),
        PoseUpdate { x: 42.0, y: 99.0, heading: 180.0 },
    );
}


