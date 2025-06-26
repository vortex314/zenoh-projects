use std::fmt::Debug;
use tokio::sync::mpsc::{self, Sender, Receiver};
use tokio::sync::oneshot;
use async_trait::async_trait;
use thiserror::Error;
use std::sync::Arc;

// ========== Error Types ==========
#[derive(Debug, Error)]
pub enum ActorError {
    #[error("Actor mailbox closed")]
    MailboxClosed,
    #[error("Actor failed to process message")]
    ProcessingFailed,
    #[error("Request timed out")]
    Timeout,
}

// ========== Message Trait ==========
pub trait Message: Debug + Send + 'static {
    type Result: Send + 'static;
}

// ========== Actor Trait ==========
#[async_trait]
pub trait Actor: Send + 'static {
    type Message: Message;

    async fn handle(&mut self, msg: Self::Message) -> Result<(), ActorError>;
    
    async fn pre_start(&mut self) {}
    async fn post_stop(&mut self) {}
    
    fn supervisor_strategy(&self) -> SupervisorStrategy {
        SupervisorStrategy::Restart
    }
}

// ========== Supervisor Strategy ==========
#[derive(Debug, Clone, Copy)]
pub enum SupervisorStrategy {
    Resume,
    Restart,
    Stop,
    Escalate,
}

// ========== ActorRef ==========
#[derive(Debug, Clone)]
pub struct ActorRef<M: Message> {
    tx: Sender<Envelope<M>>,
}

impl<M: Message> ActorRef<M> {
    pub async fn tell(&self, msg: M) -> Result<(), ActorError> {
        self.tx.send(Envelope::Message(msg)).await
            .map_err(|_| ActorError::MailboxClosed)
    }

    pub async fn ask<R>(&self, msg: M) -> Result<R, ActorError>
    where
        M: Message<Result = R>,
    {
        let (resp_tx, resp_rx) = oneshot::channel();
        let envelope = Envelope::MessageWithReply(msg, resp_tx);
        
        self.tx.send(envelope).await
            .map_err(|_| ActorError::MailboxClosed)?;
            
        resp_rx.await
            .map_err(|_| ActorError::ProcessingFailed)
    }
}

// ========== Envelope ==========
#[derive(Debug)]
enum Envelope<M: Message> {
    Message(M),
    MessageWithReply(M, oneshot::Sender<M::Result>),
    Stop,
}

// ========== Actor System ==========
pub struct ActorSystem {
    // Could include supervision hierarchy, dispatchers, etc.
}

impl ActorSystem {
    pub fn new() -> Self {
        ActorSystem {}
    }

    pub async fn spawn<A>(&self, actor: A) -> ActorRef<A::Message>
    where
        A: Actor,
    {
        let (tx, rx) = mpsc::channel(100); // Mailbox size
        let actor_ref = ActorRef { tx: tx.clone() };
        
        tokio::spawn(run_actor(actor, rx));
        
        actor_ref
    }
}

// ========== Actor Runtime ==========
async fn run_actor<A>(mut actor: A, mut rx: Receiver<Envelope<A::Message>>)
where
    A: Actor,
{
    actor.pre_start().await;
    
    while let Some(envelope) = rx.recv().await {
        match envelope {
            Envelope::Message(msg) => {
                if let Err(e) = actor.handle(msg).await {
                    // Handle error according to supervisor strategy
                    eprintln!("Actor failed: {:?}", e);
                    break;
                }
            }
            Envelope::MessageWithReply(msg, reply) => {
                let result = actor.handle(msg).await;
            //    let _ = reply.send(result.map_err(|_| ActorError::ProcessingFailed));
            }
            Envelope::Stop => {
                break;
            }
        }
    }
    
    actor.post_stop().await;
}

// Define a message type
#[derive(Debug)]
enum CounterMessage {
    Increment(i32),
    GetCount(oneshot::Sender<i32>),
}

impl Message for CounterMessage {
    type Result = i32;
}

// Define an actor
struct CounterActor {
    count: i32,
}

#[async_trait]
impl Actor for CounterActor {
    type Message = CounterMessage;

    async fn handle(&mut self, msg: CounterMessage) -> Result<(), ActorError> {
        match msg {
            CounterMessage::Increment(amount) => {
                self.count += amount;
                Ok(())
            }
            CounterMessage::GetCount(reply) => {
                reply.send(self.count)
                    .map_err(|_| ActorError::ProcessingFailed)
            }
        }
    }
}

#[tokio::main]
async fn main() {
    let system = ActorSystem::new();
    
    // Spawn the actor
    let counter_ref = system.spawn(CounterActor { count: 0 }).await;
    
    // Send messages
    counter_ref.tell(CounterMessage::Increment(5)).await.unwrap();
    counter_ref.tell(CounterMessage::Increment(3)).await.unwrap();
    
    // Ask for the count
    let (reply_tx, reply_rx) = tokio::sync::oneshot::channel();
    counter_ref.tell(CounterMessage::GetCount(reply_tx)).await.unwrap();
    let count = reply_rx.await.unwrap();
    println!("Current count: {}", count); // Should print 8
    
    // The actor will be stopped when all ActorRefs are dropped
}