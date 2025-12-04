// AI generated with timers and ask

use std::any::Any;
use std::collections::HashMap;
use std::fmt::Debug;
use std::sync::Arc;
use tokio::sync::oneshot;
use tokio::sync::RwLock;
use tokio::task::JoinHandle;
use tokio::time::{interval, sleep, Instant,Duration};
use tokio::sync::mpsc;


// Core trait for messages that can be sent to actors
pub trait Message: Send + Sync + 'static + Debug {}
impl<T: Send + Sync + 'static + Debug> Message for T {}

// Timer handle for cancelling scheduled tasks
#[derive(Debug)]
pub struct TimerHandle {
    id: String,
    cancel_tx: Option<oneshot::Sender<()>>,
}

impl TimerHandle {
    pub fn cancel(mut self) {
        if let Some(tx) = self.cancel_tx.take() {
            let _ = tx.send(());
        }
    }

    pub fn id(&self) -> &str {
        &self.id
    }
}

// Scheduler for managing timed tasks
#[derive(Clone)]
pub struct Scheduler {
    next_timer_id: Arc<RwLock<u64>>,
}

impl Scheduler {
    pub fn new() -> Self {
        Self {
            next_timer_id: Arc::new(RwLock::new(0)),
        }
    }

    // Schedule a one-time message after a delay
    pub async fn schedule_once<M: Message>(
        &self,
        delay: Duration,
        actor_ref: ActorRef,
        message: M,
    ) -> TimerHandle {
        let timer_id = self.generate_timer_id().await;
        let (cancel_tx, cancel_rx) = oneshot::channel();

        tokio::spawn(async move {
            tokio::select! {
                _ = sleep(delay) => {
                    actor_ref.tell(message).await;
                }
                _ = cancel_rx => {
                    // Timer was cancelled
                }
            }
        });

        TimerHandle {
            id: timer_id,
            cancel_tx: Some(cancel_tx),
        }
    }

    // Schedule a recurring message with fixed interval
    pub async fn schedule_with_fixed_delay<M: Message + Clone>(
        &self,
        initial_delay: Duration,
        duration: Duration,
        actor_ref: ActorRef,
        message: M,
    ) -> TimerHandle {
        let timer_id = self.generate_timer_id().await;
        let (cancel_tx, mut cancel_rx) = oneshot::channel();

        tokio::spawn(async move {
            // Initial delay
            tokio::select! {
                _ = sleep(initial_delay) => {}
                _ = &mut cancel_rx => return,
            }

            let mut timer = interval(duration);
            loop {
                tokio::select! {
                    _ = timer.tick() => {
                        actor_ref.tell(message.clone()).await;
                    }
                    _ = &mut cancel_rx => {
                        break;
                    }
                }
            }
        });

        TimerHandle {
            id: timer_id,
            cancel_tx: Some(cancel_tx),
        }
    }

    // Schedule at fixed rate (compensates for processing time)
    pub async fn schedule_at_fixed_rate<M: Message + Clone>(
        &self,
        initial_delay: Duration,
        period: Duration,
        actor_ref: ActorRef,
        message: M,
    ) -> TimerHandle {
        let timer_id = self.generate_timer_id().await;
        let (cancel_tx, mut cancel_rx) = oneshot::channel();

        tokio::spawn(async move {
            // Initial delay
            tokio::select! {
                _ = sleep(initial_delay) => {}
                _ = &mut cancel_rx => return,
            }

            let start_time = Instant::now();
            let mut tick_count = 0u64;

            loop {
                let next_tick = start_time + period * (tick_count + 1) as u32;
                let now = Instant::now();
                
                if next_tick > now {
                    tokio::select! {
                        _ = sleep(next_tick - now) => {}
                        _ = &mut cancel_rx => break,
                    }
                }

                actor_ref.tell(message.clone()).await;
                tick_count += 1;

                // Check for cancellation after sending message
                if cancel_rx.try_recv().is_ok() {
                    break;
                }
            }
        });

        TimerHandle {
            id: timer_id,
            cancel_tx: Some(cancel_tx),
        }
    }

    async fn generate_timer_id(&self) -> String {
        let mut next_id = self.next_timer_id.write().await;
        *next_id += 1;
        format!("timer-{}", *next_id)
    }
}

// Actor context provides access to system functionality
#[derive(Clone)]
pub struct ActorContext {
    pub system: ActorSystem,
    pub self_ref: ActorRef,
    pub scheduler: Scheduler,
}

impl ActorContext {
    pub async fn spawn<A: Actor + 'static>(&self, actor: A) -> ActorRef {
        self.system.spawn(actor).await
    }

    pub async fn stop(&self, actor_ref: &ActorRef) {
        self.system.stop(actor_ref).await;
    }

    // Timer convenience methods
    pub async fn schedule_once<M: Message>(
        &self,
        delay: Duration,
        message: M,
    ) -> TimerHandle {
        self.scheduler.schedule_once(delay, self.self_ref.clone(), message).await
    }

    pub async fn schedule_with_fixed_delay<M: Message + Clone>(
        &self,
        initial_delay: Duration,
        interval: Duration,
        message: M,
    ) -> TimerHandle {
        self.scheduler.schedule_with_fixed_delay(initial_delay, interval, self.self_ref.clone(), message).await
    }

    pub async fn schedule_at_fixed_rate<M: Message + Clone>(
        &self,
        initial_delay: Duration,
        period: Duration,
        message: M,
    ) -> TimerHandle {
        self.scheduler.schedule_at_fixed_rate(initial_delay, period, self.self_ref.clone(), message).await
    }

    // Schedule to other actors
    pub async fn schedule_once_to<M: Message>(
        &self,
        delay: Duration,
        actor_ref: ActorRef,
        message: M,
    ) -> TimerHandle {
        self.scheduler.schedule_once(delay, actor_ref, message).await
    }

    pub async fn schedule_with_fixed_delay_to<M: Message + Clone>(
        &self,
        initial_delay: Duration,
        interval: Duration,
        actor_ref: ActorRef,
        message: M,
    ) -> TimerHandle {
        self.scheduler.schedule_with_fixed_delay(initial_delay, interval, actor_ref, message).await
    }
}

// Core Actor trait
#[async_trait::async_trait]
pub trait Actor: Send + Sync {
    async fn receive(&mut self, msg: Box<dyn Any + Send>, ctx: &ActorContext);
    async fn pre_start(&mut self, _ctx: &ActorContext) {}
    async fn post_stop(&mut self, _ctx: &ActorContext) {}
}

// Actor reference for sending messages
#[derive(Clone, Debug)]
pub struct ActorRef {
    id: String,
    sender: mpsc::UnboundedSender<ActorMessage>,
}

impl ActorRef {
    pub async fn tell<M: Message>(&self, msg: M) {
        let boxed_msg = Box::new(msg) as Box<dyn Any + Send>;
        let actor_msg = ActorMessage::UserMessage(boxed_msg);
        let _ = self.sender.send(actor_msg);
    }

    pub async fn ask<M: Message, R: Send + 'static>(&self, msg: M) -> Result<R, ActorError> {
        let (tx, rx) = oneshot::channel();
        let boxed_msg = Box::new(msg) as Box<dyn Any + Send>;
        let actor_msg = ActorMessage::Ask(boxed_msg, tx);
        
        self.sender.send(actor_msg)
            .map_err(|_| ActorError::ActorNotFound)?;
            
        let response = rx.await.map_err(|_| ActorError::Timeout)?;
        response.downcast::<R>()
            .map(|r| *r)
            .map_err(|_| ActorError::InvalidResponse)
    }

    pub fn id(&self) -> &str {
        &self.id
    }
}

// Internal actor messages
enum ActorMessage {
    UserMessage(Box<dyn Any + Send>),
    Ask(Box<dyn Any + Send>, oneshot::Sender<Box<dyn Any + Send>>),
    Stop,
}

// Actor system errors
#[derive(Debug, thiserror::Error)]
pub enum ActorError {
    #[error("Actor not found")]
    ActorNotFound,
    #[error("Request timeout")]
    Timeout,
    #[error("Invalid response type")]
    InvalidResponse,
    #[error("Actor system shutdown")]
    SystemShutdown,
}

// Actor system manages all actors
#[derive(Clone)]
pub struct ActorSystem {
    actors: Arc<RwLock<HashMap<String, JoinHandle<()>>>>,
    next_id: Arc<RwLock<u64>>,
    scheduler: Scheduler,
}

impl ActorSystem {
    pub fn new() -> Self {
        Self {
            actors: Arc::new(RwLock::new(HashMap::new())),
            next_id: Arc::new(RwLock::new(0)),
            scheduler : Scheduler::new(),
        }
    }

    pub async fn spawn<A: Actor + 'static>(&self, mut actor: A) -> ActorRef {
        let id = self.generate_id().await;
        let (tx, mut rx) = mpsc::unbounded_channel::<ActorMessage>();
        
        let actor_ref = ActorRef {
            id: id.clone(),
            sender: tx,
        };

        let ctx = ActorContext {
            system: self.clone(),
            self_ref: actor_ref.clone(),
            scheduler:Scheduler::new(),
        };

        let handle = tokio::spawn(async move {
            // Call pre_start
            actor.pre_start(&ctx).await;

            // Main message loop
            while let Some(msg) = rx.recv().await {
                match msg {
                    ActorMessage::UserMessage(msg) => {
                        actor.receive(msg, &ctx).await;
                    }
                    ActorMessage::Ask(msg, response_tx) => {
                        // For ask pattern, we need a way to capture responses
                        // This is a simplified implementation
                        actor.receive(msg, &ctx).await;
                        // In a real implementation, the actor would need to send back a response
                        let _ = response_tx.send(Box::new(()) as Box<dyn Any + Send>);
                    }
                    ActorMessage::Stop => {
                        break;
                    }
                }
            }

            // Call post_stop
            actor.post_stop(&ctx).await;
        });

        self.actors.write().await.insert(id.clone(), handle);
        actor_ref
    }

    pub async fn stop(&self, actor_ref: &ActorRef) {
        let _ = actor_ref.sender.send(ActorMessage::Stop);
        if let Some(handle) = self.actors.write().await.remove(&actor_ref.id) {
            let _ = handle.await;
        }
    }

    pub async fn shutdown(&self) {
        let mut actors = self.actors.write().await;
        for (_, handle) in actors.drain() {
            handle.abort();
            let _ = handle.await;
        }
    }

    async fn generate_id(&self) -> String {
        let mut next_id = self.next_id.write().await;
        *next_id += 1;
        format!("actor-{}", *next_id)
    }
}

// Convenient macro for implementing typed message handlers
#[macro_export]
macro_rules! handle_message {
    ($msg:expr, $ctx:expr, $($msg_type:ty => $handler:expr),+ $(,)?) => {
        $(
            if let Some(typed_msg) = $msg.downcast_ref::<$msg_type>() {
                return $handler(typed_msg, $ctx);
            }
        )+
    };
}
