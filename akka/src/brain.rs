use actix::{Actor, Addr, Context, Handler};
use log::info;
use tokio::{runtime::Handle, time::Instant};

use crate::{
    alive::{AliveActor, AliveEvent},
    multicast::McEvent,
    value::Value,
};

struct Property<T> {
    pub value: T,
    pub last_updated: std::time::SystemTime,
}

impl<T> Property<T> {
    pub fn new(value: T) -> Self {
        Property {
            value,
            last_updated: std::time::SystemTime::now(),
        }
    }
    pub fn set(&mut self, value: T) {
        self.value = value;
        self.last_updated = std::time::SystemTime::now();
    }
}

struct Drive {
    pub src: String,
    pub speed: f64,
    pub direction: f64,
    pub alive: bool,
}

impl Drive {
    pub fn new(src: &str) -> Self {
        Drive {
            src: src.to_string(),
            speed: 0.0,
            direction: 0.0,
            alive: false,
        }
    }
    pub fn update_from_value(&mut self, v: &Value) {
        v["speed"].handle(|speed| {
            info!("drive speed: {}", speed);
            self.speed = *speed;
        });
        v["direction"].handle(|direction| {
            info!("received drive direction: {}", direction);
            self.direction = *direction;
        });
    }
}

pub struct Brain {
    pub drive: Drive,
    pub steer_direction: Property<f64>,
    pub tilt_angle: Property<f64>,
    pub alive_actor: Option<Addr<AliveActor>>,
}

impl Brain {
    pub fn new() -> Self {
        Brain {
            drive: Drive::new("esp1/drive"),
            steer_direction: Property::new(0.0),
            tilt_angle: Property::new(0.0),
            alive_actor: None,
        }
    }
}

impl Actor for Brain {
    type Context = Context<Self>;

    fn started(&mut self, _ctx: &mut Context<Self>) {
        self.alive_actor = Some(AliveActor::new().start());
    }
}

impl Handler<McEvent> for Brain {
    type Result = ();

    fn handle(&mut self, msg: McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            McEvent::Received(value) => {
                if value["src"].as_string().unwrap() == self.drive.src.as_str() {
                    self.drive.update_from_value(&value["pub"]);
                } else {
                    info!("Received non-drive value: {}", value);
                }
            }
            McEvent::ConnectionLost => {
                info!("Brain connection lost");
            }
            McEvent::ConnectionEstablished => {
                info!("Brain connection established");
            }
        }
    }
}

impl Handler<AliveEvent> for Brain {
    type Result = ();

    fn handle(&mut self, msg: AliveEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            AliveEvent::Alive(src, alive) => {
                if src == self.drive.src {
                    self.drive.alive = alive;
                };
            }
        }
    }
}
