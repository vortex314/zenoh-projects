// AI generated with timers and ask
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
use std::any::Any;
use std::collections::HashMap;
use std::fmt::Debug;
use std::sync::Arc;
use tokio::sync::RwLock;
use tokio::sync::mpsc;
use tokio::sync::oneshot;
use tokio::task::JoinHandle;
use tokio::time::{Duration, Instant, interval, sleep};

use crate::actor::Actor;
use crate::actor::ActorContext;
use crate::actor::ActorSystem;

mod actor;
mod logger;

// Main function demonstrating usage
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Starting Akka-style Actor System in Rust");

    let system = ActorSystem::new();

    // Create actors
    let greeter1 = system.spawn(GreeterActor::new("Alice".to_string())).await;
    let greeter2 = system.spawn(GreeterActor::new("Bob".to_string())).await;
    let counter = system.spawn(CounterActor::new()).await;

    // Send some messages
    greeter1.tell(HelloMessage("Rust".to_string())).await;
    greeter2.tell(HelloMessage("Tokio".to_string())).await;
    greeter1.tell("How are you?".to_string()).await;

    counter.tell(CountMessage).await;
    counter.tell(CountMessage).await;
    counter.tell(CountMessage).await;
    counter.tell(GetCountMessage).await;

    // Let actors process messages
    tokio::time::sleep(Duration::from_millis(500)).await;

    println!("Shutting down actor system...");
    system.shutdown().await;

    Ok(())
}

// Example usage and test actors
#[derive(Debug)]
pub struct HelloMessage(pub String);

#[derive(Debug)]
pub struct CountMessage;

#[derive(Debug)]
pub struct GetCountMessage;

pub struct GreeterActor {
    name: String,
}

impl GreeterActor {
    pub fn new(name: String) -> Self {
        Self { name }
    }
}

#[async_trait::async_trait]
impl Actor for GreeterActor {
    async fn receive(&mut self, msg: Box<dyn Any + Send>, _ctx: &ActorContext) {
        handle_message!(msg, _ctx,
            HelloMessage => |m: &HelloMessage, _ctx: &ActorContext|  {
                println!("{} says: Hello, {}!", self.name, m.0);
            },
            String => |m: &String, _ctx: &ActorContext|  {
                println!("{} received: {}", self.name, m);
            }
        );
    }

    async fn pre_start(&mut self, _ctx: &ActorContext) {
        println!("GreeterActor {} starting up", self.name);
    }

    async fn post_stop(&mut self, _ctx: &ActorContext) {
        println!("GreeterActor {} shutting down", self.name);
    }
}

pub struct CounterActor {
    count: u64,
}

impl CounterActor {
    pub fn new() -> Self {
        Self { count: 0 }
    }
}

#[async_trait::async_trait]
impl Actor for CounterActor {
    async fn receive(&mut self, msg: Box<dyn Any + Send>, _ctx: &ActorContext) {
        handle_message!(msg, _ctx,
            CountMessage => |_: &CountMessage, _ctx: &ActorContext|  {
                self.count += 1;
                println!("Count incremented to: {}", self.count);
            },
            GetCountMessage => |_: &GetCountMessage, _ctx: &ActorContext|  {
                println!("Current count: {}", self.count);
            }
        );
    }
}

// Example usage
#[cfg(test)]
mod tests {
    use super::*;
    use tokio::time::{Duration, sleep};

    #[tokio::test]
    async fn test_actor_system() {
        let system = ActorSystem::new();

        // Spawn greeter actor
        let greeter = system.spawn(GreeterActor::new("Alice".to_string())).await;

        // Spawn counter actor
        let counter = system.spawn(CounterActor::new()).await;

        // Send messages
        greeter.tell(HelloMessage("World".to_string())).await;
        greeter.tell("Direct string message".to_string()).await;

        counter.tell(CountMessage).await;
        counter.tell(CountMessage).await;
        counter.tell(GetCountMessage).await;

        // Let messages process
        sleep(Duration::from_millis(100)).await;

        // Stop actors
        system.stop(&greeter).await;
        system.stop(&counter).await;

        // Shutdown system
        system.shutdown().await;
    }

    #[tokio::test]
    async fn test_actor_spawning_actors() {
        let system = ActorSystem::new();

        // Create a supervisor-like actor that spawns other actors
        pub struct SupervisorActor;

        #[async_trait::async_trait]
        impl Actor for SupervisorActor {
            async fn receive(&mut self, msg: Box<dyn Any + Send>, ctx: &ActorContext) {
                match msg.downcast_ref::<String>() {
                    Some(m) if m == "spawn_greeter" => {
                        let child = ctx.spawn(GreeterActor::new("Child".to_string())).await;
                        child
                            .tell(HelloMessage("from supervisor".to_string()))
                            .await;
                    }
                    _ => println!("Supervisor received unknown message"),
                }
            }
            async fn pre_start(&mut self, _ctx: &ActorContext) {
                println!("SupervisorActor starting up");
            }
            async fn post_stop(&mut self, _ctx: &ActorContext) {
                println!("SupervisorActor shutting down");
            }
        }

        let supervisor = system.spawn(SupervisorActor).await;
        supervisor.tell("spawn_greeter".to_string()).await;

        sleep(Duration::from_millis(100)).await;
        system.shutdown().await;
    }
}
