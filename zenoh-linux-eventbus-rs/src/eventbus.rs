use anyhow::Result;
use basu::async_trait;
use log::info;
use std::{any::Any, sync::Arc};
use std::future::Future;

use crate::limero::LawnmowerManualCmd;
use crate::limero::Ps4Event;

#[async_trait]
pub trait ActorImpl: Send {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>);
}

#[derive(Clone)]
pub struct Bus {
    sender: tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>>,
}

impl Bus {
    pub fn new(sender: tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>>) -> Self {
        Bus { sender }
    }
    pub fn emit<M: 'static + Any + Send + Sync>(&self, msg: M) {
        let m: Arc<dyn Any + Send + Sync> = Arc::new(msg);
        if m.is::<LawnmowerManualCmd>() {
            info!("Bus emitting LawnmowerManualCmd message");
        }
        self.sender
            .send(m)
            .map_err(|e| info!("Send error {}", e))
            .ok();
    }
}

pub struct ActorStart {
    pub bus: Bus,
}
pub struct ActorStop {}
pub struct EventbusStop {}

pub struct ActorTick {}

pub struct Eventbus {
    sender: tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>>,
    receiver: tokio::sync::mpsc::UnboundedReceiver<Arc<dyn Any + Send + Sync>>,
    actors: Vec<Box<dyn ActorImpl>>,
}

impl Eventbus {
    pub fn new() -> Self {
        let (sender, receiver) =
            tokio::sync::mpsc::unbounded_channel::<Arc<dyn Any + Send + Sync>>();
        Eventbus {
            sender,
            receiver,
            actors: Vec::new(),
        }
    }

    pub fn register_actor(&mut self, actor: Box<dyn ActorImpl>) {
        self.actors.push(actor);
    }

    pub fn emit<M: 'static + Any + Send + Sync>(&self, msg: M) {
        let _ = self.sender.send(Arc::new(msg));
    }

    pub fn sender(&self) -> tokio::sync::mpsc::UnboundedSender<Arc<dyn Any + Send + Sync>> {
        self.sender.clone()
    }

    pub fn bus(&self) -> Bus {
        Bus::new(self.sender.clone())
    }

    async fn recv(&mut self) -> Option<Arc<dyn Any + Send + Sync>> {
        self.receiver.recv().await
    }

    async fn start_all_actors(&mut self) -> Result<()> {
        for actor in &mut self.actors {
            let start_msg: Arc<dyn Any + Send + Sync> = Arc::new(ActorStart {
                bus: Bus::new(self.sender.clone()),
            });
            actor.handle(&start_msg).await;
        }
        Ok(())
    }

    async fn stop_all_actors(&mut self) -> Result<()> {
        {
            for actor in &mut self.actors {
                let stop_msg: Arc<dyn Any + Send + Sync> = Arc::new(ActorStop {});
                actor.handle(&stop_msg).await;
            }
        }
        Ok(())
    }

    async fn handle_all_actors(&mut self, msg: &Arc<dyn Any + Send + Sync>) -> Result<()> {
        for actor in &mut self.actors {
            if msg.is::<LawnmowerManualCmd>() {
                info!("Eventbus LawnmowerManualCmd message received");
            }
            actor.handle(msg).await;
        }
        Ok(())
    }

    pub async fn run(&mut self) -> Result<()> {
        self.start_all_actors().await?;
        while let Some(msg) = self.recv().await {
            self.handle_all_actors(&msg).await?;
            if msg.is::<EventbusStop>() {
                break;
            }
        }
        self.stop_all_actors().await?;
        Ok(())
    }
}

//#[async_trait]


pub async fn on_message<T: 'static, F, Fut>(msg: &Arc<dyn Any + Send + Sync>, mut f: F)
where
    F: FnMut(&T) -> Fut,
    Fut: Future<Output = ()>,
{
    if msg.is::<T>() {
        let t: &T = msg.downcast_ref().unwrap();
        f(t).await;
    }
}
