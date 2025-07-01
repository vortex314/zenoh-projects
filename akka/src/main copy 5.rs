use std::collections::HashMap;
use std::fmt::Debug;

use std::time::Duration;
use tokio::sync::{mpsc, oneshot};
use tokio::time::{sleep, timeout};

mod logger;
//mod multicast;
mod actor;
mod value;
use crate::actor::{spawn_actor, Actor, ActorContext, Ask};
//use crate::multicast::McActor;
use actor::TimerId;

pub trait CommonMsg : Send {} 

pub enum SysMsg {
    SetUtc(u64),
}

// Example usage - Enhanced with Ask pattern
#[derive(Debug)]
pub enum CounterMessage {
    Increment,
    Decrement,
    GetCount { respond_to: oneshot::Sender<u32> },
    Reset,
    Add { value: u32, respond_to: oneshot::Sender<u32> },
    Multiply { factor: u32, respond_to: oneshot::Sender<u32> },
}

impl CommonMsg for SysMsg {}
impl CommonMsg for CounterMessage {}



/// Askable queries for the CounterActor
#[derive(Debug)]
pub struct GetCountQuery;

#[derive(Debug)]
pub struct AddQuery {
    pub value: u32,
}

#[derive(Debug)]
pub struct MultiplyQuery {
    pub factor: u32,
}

/// Implement Ask trait for each query type
impl Ask<CounterActor> for GetCountQuery {
    type Response = u32;
    
    fn into_message(self, respond_to: oneshot::Sender<Self::Response>) -> CounterMessage {
        CounterMessage::GetCount { respond_to }
    }
}

impl Ask<CounterActor> for AddQuery {
    type Response = u32;
    
    fn into_message(self, respond_to: oneshot::Sender<Self::Response>) -> CounterMessage {
        CounterMessage::Add { value: self.value, respond_to }
    }
}

impl Ask<CounterActor> for MultiplyQuery {
    type Response = u32;
    
    fn into_message(self, respond_to: oneshot::Sender<Self::Response>) -> CounterMessage {
        CounterMessage::Multiply { factor: self.factor, respond_to }
    }
}

pub struct CounterActor {
    count: u32,
    timer_id: Option<TimerId>,
}

impl CounterActor {
    pub fn new() -> Self {
        Self {
            count: 0,
            timer_id: None,
        }
    }
}

#[async_trait::async_trait]
impl Actor for CounterActor {
    type Message = CounterMessage;
    type Error = Box<dyn std::error::Error + Send + Sync>;

    async fn prestart(&mut self) -> Result<(), Self::Error> {
        println!("CounterActor starting up!");
        Ok(())
    }

    async fn poststop(&mut self) -> Result<(), Self::Error> {
        println!("CounterActor shutting down! Final count: {}", self.count);
        Ok(())
    }

    async fn handle_message(
        &mut self,
        msg: Self::Message,
        ctx: &mut ActorContext<Self>,
    ) -> Result<(), Self::Error> {
        match msg {
            CounterMessage::Increment => {
                self.count += 1;
                println!("Count incremented to: {}", self.count);
            }
            CounterMessage::Decrement => {
                if self.count > 0 {
                    self.count -= 1;
                }
                println!("Count decremented to: {}", self.count);
            }
            CounterMessage::GetCount { respond_to } => {
                let _ = respond_to.send(self.count);
            }
            CounterMessage::Add { value, respond_to } => {
                self.count += value;
                println!("Added {} to count, new value: {}", value, self.count);
                let _ = respond_to.send(self.count);
            }
            CounterMessage::Multiply { factor, respond_to } => {
                self.count *= factor;
                println!("Multiplied count by {}, new value: {}", factor, self.count);
                let _ = respond_to.send(self.count);
            }
            CounterMessage::Reset => {
                self.count = 0;
                println!("Count reset to: {}", self.count);
                
                // Start a timer to auto-increment every 2 seconds
                if self.timer_id.is_none() {
                    let timer_id = ctx.schedule_repeating(Duration::from_secs(2));
                    self.timer_id = Some(timer_id);
                    println!("Started auto-increment timer");
                }
            }
        }
        Ok(())
    }

    async fn handle_timer(
        &mut self,
        timer_id: TimerId,
        ctx: &mut ActorContext<Self>,
    ) -> Result<(), Self::Error> {
        if Some(timer_id) == self.timer_id {
            self.count += 1;
            println!("Timer auto-increment! Count: {}", self.count);
            
            // Stop timer after count reaches 10
            if self.count >= 10 {
                ctx.cancel_timer(timer_id);
                self.timer_id = None;
                println!("Auto-increment timer stopped");
            }
        }
        Ok(())
    }
}



#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create and spawn the actor
    let counter = CounterActor::new();
    let actor_ref = spawn_actor(counter, 32);
    
    // Clone the reference to demonstrate it's cloneable
    let actor_ref2 = actor_ref.clone();
    
    // Send some messages (fire-and-forget)
    actor_ref.send(CounterMessage::Increment).await?;
    actor_ref2.send(CounterMessage::Increment).await?;
    actor_ref.send(CounterMessage::Decrement).await?;
    
    // Use the ask pattern to get responses
    println!("\n=== Demonstrating Ask Pattern ===");
    
    // Ask for current count
    let count = actor_ref.ask(GetCountQuery).await?;
    println!("Current count via ask: {}", count);
    
    // Ask to add a value and get the new count
    let new_count = actor_ref.ask(AddQuery { value: 5 }).await?;
    println!("Count after adding 5: {}", new_count);
    
    // Ask to multiply and get the new count
    let multiplied_count = actor_ref.ask(MultiplyQuery { factor: 3 }).await?;
    println!("Count after multiplying by 3: {}", multiplied_count);
    
    // Demonstrate ask with custom timeout
    println!("\n=== Demonstrating Ask with Custom Timeout ===");
    let count_with_timeout = actor_ref
        .ask_with_timeout(GetCountQuery, Duration::from_millis(100))
        .await?;
    println!("Count with custom timeout: {}", count_with_timeout);
    
    // Demonstrate timeout error (reset starts a timer, but we'll stop before it causes issues)
    actor_ref.send(CounterMessage::Reset).await?;
    
    // Wait a bit for timer to work
    sleep(Duration::from_secs(3)).await;
    
    // Final count check
    let final_count = actor_ref.ask(GetCountQuery).await?;
    println!("Final count: {}", final_count);
    
    // Stop the actor
    actor_ref.stop().await?;
    
    // Give it time to shut down gracefully
    sleep(Duration::from_millis(100)).await;
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use crate::actor::AskError;

    use super::*;
    use tokio::time::timeout;

    #[tokio::test]
    async fn test_actor_lifecycle() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        
        // Test basic increment
        actor_ref.send(CounterMessage::Increment).await.unwrap();
        
        // Test ask pattern
        let count = actor_ref.ask(GetCountQuery).await.unwrap();
        assert_eq!(count, 1);
        
        // Test graceful shutdown
        actor_ref.stop().await.unwrap();
        sleep(Duration::from_millis(10)).await;
    }

    #[tokio::test]
    async fn test_cloneable_actor_ref() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        let actor_ref2 = actor_ref.clone();
        
        // Send from both references
        actor_ref.send(CounterMessage::Increment).await.unwrap();
        actor_ref2.send(CounterMessage::Increment).await.unwrap();
        
        let count = actor_ref.ask(GetCountQuery).await.unwrap();
        assert_eq!(count, 2);
        
        actor_ref.stop().await.unwrap();
    }

    #[tokio::test]
    async fn test_ask_pattern() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        
        // Test adding via ask
        let result = actor_ref.ask(AddQuery { value: 10 }).await.unwrap();
        assert_eq!(result, 10);
        
        // Test multiplying via ask
        let result = actor_ref.ask(MultiplyQuery { factor: 3 }).await.unwrap();
        assert_eq!(result, 30);
        
        // Test getting count via ask
        let count = actor_ref.ask(GetCountQuery).await.unwrap();
        assert_eq!(count, 30);
        
        actor_ref.stop().await.unwrap();
    }

    #[tokio::test]
    async fn test_ask_timeout() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        
        // Test successful ask with timeout
        let result = actor_ref
            .ask_with_timeout(GetCountQuery, Duration::from_secs(1))
            .await
            .unwrap();
        assert_eq!(result, 0);
        
        // Test that we can handle very short timeouts
        let result = actor_ref
            .ask_with_timeout(AddQuery { value: 5 }, Duration::from_millis(100))
            .await
            .unwrap();
        assert_eq!(result, 5);
        
        actor_ref.stop().await.unwrap();
    }

    #[tokio::test]
    async fn test_ask_after_actor_stop() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        
        // Stop the actor
        actor_ref.stop().await.unwrap();
        sleep(Duration::from_millis(10)).await;
        
        // Try to ask after stopping - should get ActorUnreachable error
        let result = actor_ref.ask(GetCountQuery).await;
        assert!(matches!(result, Err(AskError::ActorUnreachable)));
    }

    #[tokio::test]
    async fn test_timer_functionality() {
        let counter = CounterActor::new();
        let actor_ref = spawn_actor(counter, 32);
        
        // Reset to start timer
        actor_ref.send(CounterMessage::Reset).await.unwrap();
        
        // Wait for timer to fire a few times
        sleep(Duration::from_secs(3)).await;
        
        let count = actor_ref.ask(GetCountQuery).await.unwrap();
        
        // Should have incremented due to timer
        assert!(count > 0);
        
        actor_ref.stop().await.unwrap();
    }
}
