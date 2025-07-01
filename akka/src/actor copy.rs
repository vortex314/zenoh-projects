use std::collections::HashMap;
use std::fmt::Debug;

use std::time::Duration;
use tokio::sync::{mpsc, oneshot};
use tokio::time::{sleep, timeout};

/// Trait that defines the lifecycle methods for an actor
#[async_trait::async_trait]
pub trait Actor: Send + Sized + 'static {
    type Message: Send + 'static;
    type Error: Send + Debug + 'static;

    /// Called before the actor starts processing messages
    async fn prestart(&mut self) -> Result<(), Self::Error> {
        Ok(())
    }

    /// Called after the actor stops processing messages
    async fn poststop(&mut self) -> Result<(), Self::Error> {
        Ok(())
    }

    /// Handle incoming messages
    async fn handle_message(&mut self, msg: Self::Message, ctx: &mut ActorContext<Self>) -> Result<(), Self::Error>;

    /// Handle timer events
    async fn handle_timer(&mut self, _timer_id: TimerId, _ctx: &mut ActorContext<Self>) -> Result<(), Self::Error> {
        // Default implementation does nothing
        Ok(())
    }
}

/// Trait for messages that can be "asked" (request-response pattern)
pub trait Ask<A: Actor>: Send + 'static {
    type Response: Send + 'static;
    
    /// Convert this askable message into a regular actor message
    fn into_message(self, respond_to: oneshot::Sender<Self::Response>) -> A::Message;
}

/// Errors that can occur during ask operations
#[derive(Debug, thiserror::Error)]
pub enum AskError {
    #[error("Actor is unreachable (channel closed)")]
    ActorUnreachable,
    #[error("Request timed out after {timeout:?}")]
    Timeout { timeout: Duration },
    #[error("Actor did not respond (channel closed)")]
    NoResponse,
}

/// Default timeout for ask operations
pub const DEFAULT_ASK_TIMEOUT: Duration = Duration::from_secs(5);

/// Unique identifier for timers
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct TimerId(pub u64);

/// Timer information
#[derive(Debug, Clone)]
pub struct Timer {
    pub id: TimerId,
    pub delay: Duration,
    pub repeat: bool,
}

/// Internal actor messages
#[derive(Debug)]
pub enum InternalMessage<M> {
    UserMessage(M),
    Timer(TimerId),
    Stop,
}

/// Actor context that provides access to timers and other actor facilities
pub struct ActorContext<A: Actor> {
    sender: mpsc::Sender<InternalMessage<A::Message>>,
    timers: HashMap<TimerId, Timer>,
    next_timer_id: u64,
}

impl<A: Actor> ActorContext<A> {
    fn new(sender: mpsc::Sender<InternalMessage<A::Message>>) -> Self {
        Self {
            sender,
            timers: HashMap::new(),
            next_timer_id: 0,
        }
    }

    /// Schedule a one-time timer
    pub fn schedule_once(&mut self, delay: Duration) -> TimerId {
        let timer_id = TimerId(self.next_timer_id);
        self.next_timer_id += 1;

        let timer = Timer {
            id: timer_id,
            delay,
            repeat: false,
        };
        
        self.timers.insert(timer_id, timer.clone());
        self.spawn_timer(timer);
        timer_id
    }

    /// Schedule a repeating timer
    pub fn schedule_repeating(&mut self, delay: Duration) -> TimerId {
        let timer_id = TimerId(self.next_timer_id);
        self.next_timer_id += 1;

        let timer = Timer {
            id: timer_id,
            delay,
            repeat: true,
        };
        
        self.timers.insert(timer_id, timer.clone());
        self.spawn_timer(timer);
        timer_id
    }

    /// Cancel a timer
    pub fn cancel_timer(&mut self, timer_id: TimerId) {
        self.timers.remove(&timer_id);
    }

    /// Stop the actor
    pub async fn stop(&self) {
        let _ = self.sender.send(InternalMessage::Stop).await;
    }

    fn spawn_timer(&self, timer: Timer) {
        let sender = self.sender.clone();
        tokio::spawn(async move {
            loop {
                sleep(timer.delay).await;
                
                // Send timer message
                if sender.send(InternalMessage::Timer(timer.id)).await.is_err() {
                    // Actor is dead, stop timer
                    break;
                }

                if !timer.repeat {
                    break;
                }
            }
        });
    }
}

/// Handle to communicate with an actor - can be cloned and shared
pub struct ActorRef<A: Actor> {
    sender: mpsc::Sender<InternalMessage<A::Message>>,
}

impl<A: Actor> Clone for ActorRef<A> {
    fn clone(&self) -> Self {
        Self {
            sender: self.sender.clone(),
        }
    }
}

impl<A: Actor> ActorRef<A> {
    /// Send a message to the actor
    pub async fn send(&self, msg: A::Message) -> Result<(), mpsc::error::SendError<InternalMessage<A::Message>>> {
        self.sender.send(InternalMessage::UserMessage(msg)).await
    }

    /// Try to send a message without waiting
    pub fn try_send(&self, msg: A::Message) -> Result<(), mpsc::error::TrySendError<InternalMessage<A::Message>>> {
        self.sender.try_send(InternalMessage::UserMessage(msg))
    }

    /// Ask the actor with a message and wait for a response with default timeout
    pub async fn ask<Q>(&self, question: Q) -> Result<Q::Response, AskError>
    where
        Q: Ask<A>,
    {
        self.ask_with_timeout(question, DEFAULT_ASK_TIMEOUT).await
    }

    /// Ask the actor with a message and wait for a response with custom timeout
    pub async fn ask_with_timeout<Q>(&self, question: Q, timeout_duration: Duration) -> Result<Q::Response, AskError>
    where
        Q: Ask<A>,
    {
        let (tx, rx) = oneshot::channel();
        let message = question.into_message(tx);
        
        // Send the message
        self.sender
            .send(InternalMessage::UserMessage(message))
            .await
            .map_err(|_| AskError::ActorUnreachable)?;
        
        // Wait for response with timeout
        match timeout(timeout_duration, rx).await {
            Ok(Ok(response)) => Ok(response),
            Ok(Err(_)) => Err(AskError::NoResponse),
            Err(_) => Err(AskError::Timeout { timeout: timeout_duration }),
        }
    }

    /// Stop the actor
    pub async fn stop(&self) -> Result<(), mpsc::error::SendError<InternalMessage<A::Message>>> {
        self.sender.send(InternalMessage::Stop).await
    }
}

/// Spawns an actor and returns a handle to it
pub fn spawn_actor<A: Actor>(mut actor: A, buffer_size: usize) -> ActorRef<A> {
    let (sender, mut receiver) = mpsc::channel(buffer_size);
    let actor_ref = ActorRef { sender: sender.clone() };
    
    tokio::spawn(async move {
        let mut ctx = ActorContext::new(sender);
        
        // Call prestart
        if let Err(e) = actor.prestart().await {
            eprintln!("Actor prestart failed: {:?}", e);
            return;
        }

        // Main message loop
        while let Some(msg) = receiver.recv().await {
            match msg {
                InternalMessage::UserMessage(user_msg) => {
                    if let Err(e) = actor.handle_message(user_msg, &mut ctx).await {
                        eprintln!("Actor message handling failed: {:?}", e);
                        break;
                    }
                }
                InternalMessage::Timer(timer_id) => {
                    // Check if timer is still active
                    if ctx.timers.contains_key(&timer_id) {
                        if let Err(e) = actor.handle_timer(timer_id, &mut ctx).await {
                            eprintln!("Actor timer handling failed: {:?}", e);
                            break;
                        }
                        
                        // Remove one-time timers
                        if let Some(timer) = ctx.timers.get(&timer_id) {
                            if !timer.repeat {
                                ctx.timers.remove(&timer_id);
                            }
                        }
                    }
                }
                InternalMessage::Stop => {
                    break;
                }
            }
        }

        // Call poststop
        if let Err(e) = actor.poststop().await {
            eprintln!("Actor poststop failed: {:?}", e);
        }
    });

    actor_ref
}
