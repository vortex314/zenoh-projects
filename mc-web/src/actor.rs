use std::collections::HashMap;

use axum::serve::Listener;
use log::info;
use tokio::{
    io::AsyncReadExt,
    sync::mpsc::{channel, Receiver, Sender},
};

use crate::value::Value;
// type Callback<T> = Box<dyn Fn(T) + Send + Sync + 'static>;

#[derive(Clone)]
struct ActorRefRepo {
    actor_refs: Vec<ActorRef>,
}

impl ActorRefRepo {
    fn find_by_name(&self, name: String) -> Option<ActorRef> {
        for actor_ref in self.actor_refs {
            if actor_ref.name == name {
                return Some(actor_ref.clone());
            }
        }
        None
    }
}

#[derive(Clone)]
struct ActorRef {
    sender: Option<Sender<Value>>,
    name: String,
}

impl ActorRef {
    pub fn new(name: String) -> Self {
        ActorRef {
            name,
            sender: repo.find_by_name(name),
        }
    }
    pub async fn tell(&mut self, v: Value) {
        if self.sender.is_none() {
            self.sender = repo.find_by_name(self.name);
        }
        self.sender.map(|sender| {
            sender.send(v);
        });
    }
}

type Callback = Box<dyn Fn(&Value) + Send + Sync + 'static>;
pub struct Actor {
    name: &str,
    actor: Box<dyn ActorImpl>,
    sender: Sender<Value>,
    receiver: Receiver<Value>,
    stop: bool,
}

impl Actor {
    pub fn new(name: &str, actor: Box<dyn ActorImpl>) -> Self {
        let (sender, receiver) = channel(10);
        Actor {
            name,
            actor: actor,
            sender,
            receiver,
            stop: false,
        }
    }

    pub async fn tell(&self, v: Value) {
        self.sender.send(v).await;
    }

    pub fn sender(&self) -> Sender<Value> {
        self.sender.clone()
    }

    pub fn actor_ref(&self) -> ActorRef {
        ActorRef {
            name: self.name.to_string(),
            sender: Some(self.sender.clone()),
        }
    }

    pub async fn run(&mut self) {
        self.actor.on_start();
        loop {
            let v = self.receiver.recv().await;
            self.actor
                .on_cmd(&v.unwrap())
                .iter()
                .for_each(|v| self.emit(v));
            if self.stop {
                break;
            }
        }
        self.actor.on_stop();
    }
}

pub trait ActorImpl {
    fn on_cmd(&mut self, cmd: &Value) -> Vec<Value>;
    fn on_timer(&mut self, timer_id: u32) -> Vec<Value>;
    fn on_start(&mut self);
    fn on_stop(&mut self);
}

pub struct Peer {
    opponent: Option<ActorRef>,
}

impl Peer {
    pub fn new() -> Self {
        Peer {
            opponent:None,
        }
    }
}

impl ActorImpl for Peer {
    fn on_cmd(&mut self, cmd: &Value) -> Vec<Value> {
        cmd["opponent"].handle<Value::Object>()
        if cmd["opponent"].is_string() {
            self.opponent = Some(ActorRef::new(cmd["opponent"].as_string().unwrap()));
        }
        let mut v: Value = Value::object();
        v["pi"] = Value::Float(3.14159);
        v["input"] = cmd.clone();
        let mut vec = Vec::new();
        vec.push(v);
        vec
    }
    fn on_timer(&mut self, _timer_id: u32) -> Vec<Value> {
        Vec::new()
    }
    fn on_start(&mut self) {}
    fn on_stop(&mut self) {}
}

pub fn test_actors() {
    let pong = Actor::new("pong", Peer::new());
    let ping = Actor::new("ping", Peer::new());
    let msg = Value::object();
    msg["opponent"] = "pong".into();
    ping.tell(msg);
    let msg = Value::object();
    msg["cmd"] = "start".into();
    ping.tell(msg);
    loop {
        select! {
            _ = ping.run() =>{},
            _= pong.run()=>{},
        }
    }
}
