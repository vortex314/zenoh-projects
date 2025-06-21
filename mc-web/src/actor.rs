use std::{collections::HashMap, ops::Deref};

use axum::serve::Listener;
use log::info;
use tokio::{
    io::AsyncReadExt,
    sync::mpsc::{Receiver, Sender, channel},
};

use crate::value::Value;
// type Callback<T> = Box<dyn Fn(T) + Send + Sync + 'static>;

use once_cell::sync::Lazy;
use std::sync::{Arc, Mutex};

static REPO: Lazy<Arc<Mutex<ActorRefRepo>>> = Lazy::new(|| {
    Arc::new(Mutex::new(ActorRefRepo {
        actor_refs: Vec::new(),
    }))
});


#[derive(Clone)]
struct ActorRefRepo {
    actor_refs: Vec<ActorRef>,
}

impl ActorRefRepo {
    fn find_by_name(&self, name: &str) -> Option<ActorRef> {
        for actor_ref in self.actor_refs.iter() {
            if actor_ref.name.as_str() == name as &str {
                return Some(actor_ref.clone());
            }
        }
        None
    }
}

#[derive(Clone)]
pub struct ActorRef {
    sender: Option<Sender<Value>>,
    name: String,
}


impl ActorRef {
    pub fn name(&self) -> &str {
        &self.name
    }
    pub fn sender(&self) -> Option<Sender<Value>> {
        self.sender.clone()
    }
    pub fn create_new(name: &str) -> Self {
        ActorRef {
            sender: None,
            name: name.to_string(),
        }
    }
    pub fn new(name: &str) -> Self {
        // Create a new ActorRef with the given name 
        REPO
            .lock()
            .unwrap()
            .find_by_name(name)
            .unwrap_or_else(|| ActorRef::create_new(name))
    }

    pub async fn tell(&mut self, v: Value) {
        if self.sender.is_none() {
            self.sender = REPO
                .lock()
                .unwrap()
                .find_by_name(&self.name)
                .and_then(|actor_ref| actor_ref.sender.clone());
        }
        if let Some(s) = self.sender.as_ref() {
            let _ = s.try_send(v);
        } else {
            info!("ActorRef {} not found", self.name);
        }
    }
}

pub struct Actor<T: ActorImpl> {
    name: String,
    actor: T,
    sender: Sender<Value>,
    receiver: Receiver<Value>,
    stop: bool,
}

impl<T: ActorImpl> Actor<T> {
    pub fn new(name: &str, actor: T) -> Self {
        let (sender, receiver) = channel(10);
        REPO.lock().unwrap().actor_refs.push(ActorRef {
            name: name.to_string(),
            sender: Some(sender.clone()),
        });
        info!("Actor {} created", name);
        // Register the actor in the repository
        info!("Actor {} registered in repository", name);

        Actor {
            name: name.to_string(),
            actor,
            sender,
            receiver,
            stop: false,
        }
    }

    pub async fn tell(&self, v: Value) {
        let _ = self.sender.send(v).await;
    }

    pub fn sender(&self) -> Sender<Value> {
        self.sender.clone()
    }

    pub fn actor_ref(&self) -> ActorRef {
        ActorRef {
            name: self.name.clone(),
            sender: Some(self.sender.clone()),
        }
    }

    pub async fn run(&mut self) {
        self.actor.on_start().await;
        loop {
            let v = self.receiver.recv().await;
            info!("Actor {} received command: {:?}", self.name, v);
            if let Some(val) = v {
                self.actor.on_cmd(&val).await;
            }
            if self.stop {
                break;
            }
        }
        self.actor.on_stop().await;
    }
}
pub trait ActorImpl: Send {
    async fn on_cmd(&mut self, cmd: &Value);
    async fn on_timer(&mut self, timer_id: u32);
    async fn on_start(&mut self);
    async fn on_stop(&mut self);
}

pub struct Peer {
    opponent: Option<ActorRef>,
}

impl Peer {
    pub fn new() -> Self {
        Peer { opponent: None }
    }
}
impl ActorImpl for Peer {
    async fn on_cmd(&mut self, cmd: &Value) {
        // Example logic, adjust as needed
        if cmd["opponent"].is_string() {
            self.opponent = Some(ActorRef::new(
                cmd["opponent"].as<String>().unwrap_or_else(|| "unknown".to_string(),
            ));
        }
        if cmd["cmd"].is_string() && cmd["cmd"].as_string().unwrap() == "start" {
            self.opponent
                .as_mut()
                .map(|opponent| opponent.tell(Value::object()));
        }

        // Do something with the command
    }
    async fn on_timer(&mut self, _timer_id: u32) {
        // Timer logic here
    }
    async fn on_start(&mut self) {}
    async fn on_stop(&mut self) {}
}

pub async fn test_actors() {
    let mut pong = Actor::new("pong", Peer::new());
    let mut ping = Actor::new("ping", Peer::new());
    let mut msg = Value::object();
    msg["opponent"] = "pong".into();
    ping.tell(msg).await;
    let mut msg = Value::object();
    msg["cmd"] = "start".into();
    ping.tell(msg).await;
    loop {
        tokio::join!(ping.run(), pong.run());
    }
}
