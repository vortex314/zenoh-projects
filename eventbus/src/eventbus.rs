use log::info;
use tokio::sync::{broadcast, mpsc};
use tokio::task::JoinHandle;
use tokio::time::{Duration, sleep};

use std::any::TypeId;
use std::sync::Arc;

#[derive(Clone)]
pub struct MessageEnvelope {
    pub type_id: TypeId,
    pub src: Option<String>,
    pub dst: Option<String>,
    payload: Arc<dyn std::any::Any + Send + Sync>,
}

pub struct MessageFilter {
    pub src: Option<String>,
    pub dst: Option<String>,
    pub type_ids: Vec<TypeId>,
}

impl MessageFilter {
    pub fn new(src: Option<String>, dst: Option<String>, type_ids: Vec<TypeId>) -> Self {
        Self { src, dst, type_ids }
    }

    pub fn matches(&self, env: &MessageEnvelope) -> bool {
        (self.src.is_none() || self.src == env.src)
            && (self.dst.is_none() || self.dst == env.dst)
            && (self.type_ids.is_empty() || self.type_ids.contains(&env.type_id))
    }

    pub fn any() -> Self {
        Self {
            src: None,
            dst: None,
            type_ids: vec![],
        }
    }
    pub fn from_type<T: 'static>() -> Self {
        Self {
            src: None,
            dst: None,
            type_ids: vec![TypeId::of::<T>()],
        }
    }
}

impl MessageEnvelope {
    pub fn new<T: Send + Sync + 'static>(
        src: Option<String>,
        dst: Option<String>,
        payload: T,
    ) -> Self {
        Self {
            type_id: TypeId::of::<T>(),
            src,
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
        src: Option<String>,
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

    pub fn subscribe_filtered(&self, filter: MessageFilter, sender: mpsc::Sender<MessageEnvelope>) {
        let mut rx = self.tx.subscribe();
        tokio::spawn(async move {
            while let Ok(env) = rx.recv().await {
                if filter.matches(&env) {
                    if sender.send(env).await.is_err() {
                        break;
                    }
                }
            }
        });
    }

    pub fn spawn_timer<T: Send + Sync + Clone + 'static>(
        &self,
        src: Option<String>,
        dst: Option<String>,
        event: T,
        interval: Duration,
    ) -> JoinHandle<()> {
        let bus = self.clone();
        tokio::spawn(async move {
            loop {
                info!("Publishing timer event");
                sleep(interval).await;
                bus.publish(src.clone(), dst.clone(), event.clone());
            }
        })
    }
}
