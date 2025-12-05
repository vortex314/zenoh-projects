use anyhow::Result;
use log::info;
use std::{any::Any, sync::Arc, time::Instant};
use tokio::sync::mpsc::{ UnboundedSender};
use zenoh_linux_eventbus_rs::eventbus::{
    on_message, ActorImpl, Bus, Eventbus, ActorStart, EventbusStop,
};
use zenoh_linux_eventbus_rs::logger;

struct PingMsg {
    counter: usize,
}
struct PongMsg {
    counter: usize,
}

struct ActorA {
    count: usize,
    max: usize,
    start: Option<Instant>,

    bus: Option<Bus>,
}

impl ActorA {
    fn new(max: usize) -> Self {
        ActorA {
            count: 0,
            max,
            start: None,
            bus: None,
        }
    }
    fn on_start(&mut self, msg: &ActorStart) {
        self.bus = Some(msg.bus.clone());
        self.bus.as_ref().unwrap().emit(PingMsg { counter: 0 });
    }

    fn on_pong(&mut self, _msg: &PongMsg) {
        if self.count == 0 {
            self.start = Some(Instant::now());
        }
        self.count += 1;
        if self.count < self.max {
            self.bus.as_ref().unwrap().emit(PingMsg {
                counter: self.count,
            });
        } else {
            let elapsed = self.start.unwrap().elapsed();
            info!(
                "Completed {} ping-pong messages in {:?} msec at {} msg/sec",
                self.max,
                elapsed.as_millis(),
                (self.max as f64 / elapsed.as_secs_f64()).round()
            );
            self.bus.as_ref().unwrap().emit(EventbusStop {});
        }
    }
}

#[async_trait::async_trait]
impl ActorImpl for ActorA {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>) {
        on_message::<ActorStart, _>(msg, |m| self.on_start(m));
        on_message::<PongMsg, _>(msg, |msg| self.on_pong(msg));
    }
}

struct ActorB {
    bus : Option<Bus>,
}

#[async_trait::async_trait]
impl ActorImpl for ActorB {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>) {
        on_message::<ActorStart,_>(msg,|start| self.bus = Some(start.bus.clone()) );
        on_message::<PingMsg, _>(msg, |ping| {
            self.bus.as_ref().unwrap().emit(PongMsg {
                counter: ping.counter,
            })
        });
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn ping_pong_benchmark() -> Result<()> {
        use zenoh_linux_eventbus_rs::logger;
        logger::init();
        info!("Starting ping-pong benchmark test...");
        let mut bus = Eventbus::new();

        let actor_a = ActorA {
            count: 0,
            max: 1_000_000,
            start: None,
            bus: None,
        };
        let actor_b = ActorB {
            bus: None,
        };
        bus.register_actor(Box::new(actor_a));
        bus.register_actor(Box::new(actor_b));

        bus.run().await?;

        Ok(())
    }
}

// Helper trait for downcasting
trait AsAny {
    fn as_any(&self) -> &dyn Any;
}
impl<T: Any> AsAny for T {
    fn as_any(&self) -> &dyn Any {
        self
    }
}
