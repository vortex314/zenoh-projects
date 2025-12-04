// AI generated with timers and ask
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
use log::info;
use std::any::Any;
use std::collections::HashMap;
use std::fmt::Debug;
use std::sync::Arc;
use tokio::sync::mpsc;
use tokio::sync::oneshot;
use tokio::sync::RwLock;
use tokio::task::JoinHandle;
use tokio::time::{interval, sleep, Duration, Instant};

use crate::actor::Actor;
use crate::actor::ActorContext;
use crate::actor::ActorSystem;
use crate::multicast::McActor;
use crate::multicast::McMessage;
use crate::value::Value;
use zenoh_actor;

mod actor;
mod logger;
mod multicast;
mod value;

struct PrinterActor;

impl PrinterActor {
    pub fn new() -> Self {
        Self {}
    }
}
#[async_trait::async_trait]
impl Actor for PrinterActor {
    async fn receive(&mut self, msg: Box<dyn Any + Send>, _ctx: &ActorContext) {
        if let Some(s) = msg.downcast_ref::<String>() {
            info!("PrinterActor received: {}", s);
        } else if let Some(mc) = msg.downcast_ref::<McMessage>() {
            match mc {
                McMessage::Received(v) => {
                    info!("PrinterActor received McMessage::Received: {}", v.to_json());
                }
                McMessage::AddListener(_) => {
                    info!("PrinterActor received McMessage::AddListener");
                }
                McMessage::Send(v) => {
                    info!("PrinterActor received McMessage::Send: {}", v.to_json());
                }
                _ => {
                    info!("PrinterActor received unknown McMessage type");
                }
            }
        } else {
            info!("PrinterActor received unknown message type");
        }
    }   
    async fn pre_start(&mut self, _ctx: &ActorContext) {
        info!("PrinterActor starting up");
    }
    async fn post_stop(&mut self, _ctx: &ActorContext) {
        info!("PrinterActor shutting down");
    }
}

// Main function demonstrating usage
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    logger::init();
    info!("Starting Akka-style Actor System in Rust");

    let system = ActorSystem::new();


    let mc = system.spawn(McActor::new()).await;
    let printer = system.spawn(PrinterActor::new()).await;
    mc.tell(McMessage::AddListener(printer.clone())).await;



    let mut v = Value::object();
    v["src"]="master/brain".into();
    v["pub"]=Value::object();
    v["pub"]["rpm_target"]=1234.5.into();

    mc.tell(McMessage::Send(v)).await;


    // Let actors process messages
    tokio::time::sleep(Duration::from_millis(100000)).await;

    info!("Shutting down actor system...");
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
                info!("{} says: Hello, {}!", self.name, m.0);
            },
            String => |m: &String, _ctx: &ActorContext|  {
                info!("{} received: {}", self.name, m);
            }
        );
    }

    async fn pre_start(&mut self, _ctx: &ActorContext) {
        info!("GreeterActor {} starting up", self.name);
    }

    async fn post_stop(&mut self, _ctx: &ActorContext) {
        info!("GreeterActor {} shutting down", self.name);
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
                info!("Count incremented to: {}", self.count);
            },
            GetCountMessage => |_: &GetCountMessage, _ctx: &ActorContext|  {
                info!("Current count: {}", self.count);
            }
        );
    }
}

// Example usage
#[cfg(test)]
mod tests {
    use super::*;
    use tokio::time::{sleep, Duration};

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
                    _ => info!("Supervisor received unknown message"),
                }
            }
            async fn pre_start(&mut self, _ctx: &ActorContext) {
                info!("SupervisorActor starting up");
            }
            async fn post_stop(&mut self, _ctx: &ActorContext) {
                info!("SupervisorActor shutting down");
            }
        }

        let supervisor = system.spawn(SupervisorActor).await;
        supervisor.tell("spawn_greeter".to_string()).await;

        sleep(Duration::from_millis(100)).await;
        system.shutdown().await;
    }
}
