use std::{any::TypeId, time::Duration};

use log::info;
use socket2::Type;
use tokio::{sync::mpsc, time::sleep};

mod eventbus;
use eventbus::EventBus;
mod logger;

use crate::eventbus::{MessageEnvelope, MessageFilter};

#[derive(Clone, Debug)]
struct PoseUpdate {
    x: f32,
    y: f32,
    heading: f32,
}

#[derive(Clone, Debug)]
enum TimerPlanner {
    SmallInterval,
    BigInterval,
}

struct PlannerActor {
    id: String,
    bus: EventBus,
    rx: mpsc::Receiver<MessageEnvelope>,
    tx: mpsc::Sender<MessageEnvelope>,
}

impl PlannerActor {
    fn new(id: impl Into<String>, bus: EventBus) -> Self {
        let (tx, rx) = mpsc::channel::<MessageEnvelope>(10);
        Self {
            id: id.into(),
            bus,
            rx,
            tx,
        }
    }

    async fn on_poseupdate(&mut self, msg: &PoseUpdate) {
        info!("[{}] Planner got pose: {:?}", self.id, msg);
    }

    async fn on_timer(&mut self, msg: &TimerPlanner) {
        info!("[{}] Timer fired: {:?}", self.id, msg);
    }

    async fn start(&mut self) {
        // Timer sends to planner every 2s
        let t1 = self.bus.spawn_timer(
            Some("bus".to_string()) ,
            Some("planner".into()),
            TimerPlanner::SmallInterval,
            Duration::from_secs(1),
        );
        let t2 = self.bus.spawn_timer(
            Some("bus".to_string()),
            Some("planner".into()),
            TimerPlanner::BigInterval,
            Duration::from_secs(5),
        );

        self.bus.subscribe_filtered(
            MessageFilter::new(
                None,
                None,
                vec![TypeId::of::<TimerPlanner>(), TypeId::of::<PoseUpdate>()],
            ),
            self.tx.clone(),
        );
    }

    async fn run(&mut self) {
        while let Some(env) = self.rx.recv().await {
            if let Some(msg) = env.downcast::<PoseUpdate>() {
                self.on_poseupdate(msg).await;
            } else if let Some(msg) = env.downcast::<TimerPlanner>() {
                self.on_timer(msg).await;
            }
        }
    }

    async fn handle<T>(&mut self, src: Option<String>, dst: Option<String>, msg: T) {
    
    }
}

#[tokio::main]
async fn main() {
    logger::init();
    let bus = EventBus::new(32);

    let mut planner = PlannerActor::new("planner", bus.clone());

    planner.start().await;

    tokio::spawn(async move {
        planner.run().await;
    });

    // Broadcast PoseUpdate (all listeners of that type will see it)
    for i in 0..5 {
        bus.publish(
            Some("robot".into()),
            None, // broadcast
            PoseUpdate {
                x: i as f32,
                y: i as f32 * 0.5,
                heading: 90.0,
            },
        );
        sleep(Duration::from_secs(1)).await;
    }
    sleep(Duration::from_secs(10)).await;

    // Direct message just to planner
    bus.publish(
        Some("controller".into()),
        Some("planner".into()),
        PoseUpdate {
            x: 42.0,
            y: 99.0,
            heading: 180.0,
        },
    );
}
