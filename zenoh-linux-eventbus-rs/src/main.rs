// ACTIX
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(unused_imports)]
#![allow(unused_must_use)]
#![allow(dead_code)] // Allow dead code for now
#![allow(unused_variables)]
mod limero;
mod logger;

use anyhow::Result;
use basu::async_trait;
use limero::*;
use log::{debug, info};
use std::{any::Any, collections::HashMap, hash::Hash, sync::Arc, time::Duration};

#[async_trait]
pub trait ActorImpl: Send {
    fn emit(&self, msg: Arc<dyn Any + Send + Sync>);
    async fn start(&mut self) -> Result<()>;
    async fn stop(&mut self) -> Result<()>;
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>) -> Result<()>;
}

struct Actor1 {
    name: String,
    sender: tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>>,
}

#[async_trait]
impl ActorImpl for Actor1 {
    fn emit(&self, msg: Arc<dyn Any + Send + Sync>) {
        let _ = self.sender.send(msg);
    }

    async fn start(&mut self) -> Result<()> {
        Ok(())
    }

    async fn stop(&mut self) -> Result<()> {
        Ok(())
    }

    async fn handle(&mut self, _msg: &Arc<dyn Any + Send + Sync>) -> Result<()> {
        // test for type and downcast
        info!("Actor {} handling message", self.name);

        if _msg.is::<String>() {
            info!("Actor {} received a String message", self.name);
            let str_ref = _msg.downcast_ref::<String>().unwrap();
            info!("Actor {} message content: {}", self.name, str_ref);
        } 
        else if _msg.is::<Ps4Event>() {
            info!("Actor {} received a Ps4Event message", self.name);
            let event_ref = _msg.downcast_ref::<Ps4Event>().unwrap();
            info!("Actor {} message content: {:?}", self.name, event_ref);
            self.sender.send(Arc::new("Response from Actor".to_string()));
        }
        else {
            info!("Actor {} received an unknown message type", self.name);
        }

        Ok(())
    }
}

struct Bus {
    sender: tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>>,
    receiver: tokio::sync::mpsc::UnboundedReceiver<Arc<dyn Any + Send + Sync>>,
    actors: Vec<Box<dyn ActorImpl>>,
}

impl Bus {
    fn new() -> Self {
        let (sender, receiver) = tokio::sync::mpsc::unbounded_channel::<Arc<dyn Any + Send + Sync>>();
        Bus {
            sender,
            receiver,
            actors: Vec::new(),
        }
    }

    fn register_actor(&mut self, actor: Box<dyn ActorImpl>) {
        self.actors.push(actor);
    }

    fn emit<M: 'static + Any + Send + Sync>(&self, msg: M) {
        let _ = self.sender.send(Arc::new(msg));
    }

    async fn recv(&mut self) -> Option<Arc<dyn Any + Send + Sync>> {
        self.receiver.recv().await
    }

    async fn start_all_actors(&mut self) -> Result<()> {
        for actor in &mut self.actors {
            actor.start().await?;
        }
        Ok(())
    }

    async fn stop_all_actors(&mut self) -> Result<()> {
        for actor in &mut self.actors {
            actor.stop().await?;
        }
        Ok(())
    }

    async fn handle_all_actors(&mut self, msg: Arc<dyn Any + Send + Sync>) -> Result<()> {
        for actor in &mut self.actors {
            actor.handle(&msg).await?;
        }
        Ok(())
    }
}

#[tokio::main]
async fn main() -> Result<()> {
    logger::init();
    info!("Starting Limero example...");

    let mut bus = Bus::new();
    let sender = bus.sender.clone();

    let actor = Actor1 {
        name: "Actor1".to_string(),
        sender: sender.clone(),
    };

    let actor2 = Actor1 {
        name: "Actor2".to_string(),
        sender: sender.clone(),
    };
    bus.register_actor(Box::new(actor));
    bus.register_actor(Box::new(actor2));
    bus.emit("Hello, Limero!".to_string());
    bus.emit(Ps4Event::default());
    bus.start_all_actors().await?;

    loop {
        tokio::select! {
            Some(msg) = bus.receiver.recv() => {
                bus.handle_all_actors(msg).await?;
            }
            else => {
                break;
            }
        }
    }

    bus.stop_all_actors().await?;
    info!("Limero example finished.");

    Ok(())
}
