use anyhow::Result;
use basu::async_trait;
use log::info;
use std::{any::Any, sync::Arc};
use std::future::Future;
use tokio::sync::mpsc::UnboundedSender;
use tokio::sync::mpsc::UnboundedReceiver;
use tokio::sync::mpsc::unbounded_channel;

#[async_trait]
pub trait ActorImpl: Send {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>);
}

#[derive(Clone)]
pub struct Bus {
    sender: UnboundedSender<Arc<dyn Any + Send + Sync>>,
}

impl Bus {
    pub fn new(sender: UnboundedSender<Arc<dyn Any + Send + Sync>>) -> Self {
        Bus { sender }
    }
    pub fn emit<M: 'static + Any + Send + Sync>(&self, msg: M) {
        let m: Arc<dyn Any + Send + Sync> = Arc::new(msg);
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
    sender: UnboundedSender<Arc<dyn Any + Send + Sync>>,
    receiver: UnboundedReceiver<Arc<dyn Any + Send + Sync>>,
    actors: Vec<Box<dyn ActorImpl>>,
}

impl Eventbus {
    pub fn new() -> Self {
        let (sender, receiver) =
            unbounded_channel::<Arc<dyn Any + Send + Sync>>();
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

    pub fn sender(&self) -> UnboundedSender<Arc<dyn Any + Send + Sync>> {
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
    
pub fn just<T, F>(t: Option<T>, mut f: F)
where
    F: FnMut(T),
{
    if let Some(value) = t {
        f(value);
    }
}

pub fn as_message<T: 'static + Any + Send + Sync>(msg: &Arc<dyn Any + Send + Sync>) -> Option<&T> {
    if msg.is::<T>() {
        Some(msg.downcast_ref::<T>().unwrap())
    } else {
        None
    }
}