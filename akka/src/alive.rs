use actix::Actor;
use actix::Message;
use tokio::time::Instant;

use crate::multicast::McEvent;
use crate::value::Value;
use actix::Context;
use actix::Handler;

struct Alive {
    pub object: String,
    pub last_updated: Instant,
}

pub struct AliveActor {
    objects: Vec<Alive>,
}

#[derive(Debug, Message)]
#[rtype(result = "()")]
pub enum AliveEvent {
    Alive(String, bool),
}

impl AliveActor {
    pub fn new() -> Self {
        let mut objects = Vec::new();
        objects.push(Alive {
            object: "esp1/motor".to_string(),
            last_updated: Instant::now(),
        });
        AliveActor { objects }
    }
}

impl Actor for AliveActor {
    type Context = Context<Self>;

    fn started(&mut self, _: &mut Context<Self>) {
        // spawn task for expiration, check every sec
        
    }
}

impl Handler<McEvent> for AliveActor {
    type Result = ();

    fn handle(&mut self, msg: McEvent, _: &mut Self::Context) -> Self::Result {
        match msg {
            McEvent::Received(value) => {
                value["src"].handle(|src: &String| {
                    self.objects.iter_mut().for_each(|object| {
                        if object.object == *src {
                            object.last_updated = Instant::now();
                        }
                    });
                });
            }
            _ => {}
        }
    }
}
