


use std::any::Any;
use std::collections::HashMap;
use log::info;



trait Handler: Send {
    async fn handle(&mut self, event: &Box<dyn Any + Send>);
}
struct EventBus {
    sender: tokio_mpmc::Sender<Box<dyn Any + Send>>,
    receiver: tokio_mpmc::Receiver<Box<dyn Any + Send>>,
    my_handlers: HashMap<std::any::TypeId, Vec<Box<dyn Handler + Send>>>,
}

impl EventBus {
    fn new() -> Self {
        let (sender, receiver) = tokio_mpmc::channel::<Box<dyn Any + Send>>(1024);
        EventBus {
            sender,
            receiver,
            my_handlers: HashMap::new(),
        }
    }

    async fn emit(&self, event: Box<dyn Any + Send>) {
        let _ = self.sender.send(event).await;
    }

    fn register_handler<T: 'static + Send>(&mut self, handler: Box<dyn Handler + Send>) {
        self.my_handlers
            .entry(std::any::TypeId::of::<T>())
            .or_default()
            .push(handler);
        for (key, handlers) in &self.my_handlers {
            info!("Registered handler for type {:?} ({} handlers)", key, handlers.len());
        }
    }

    async fn run(&mut self) {
        while let Ok(event) = self.receiver.recv().await {
            let event = event.unwrap();
            info!("Event received: {:?} ({} handlers registered)", event.type_id(), self.my_handlers.keys().count());
            for handler in self
                .my_handlers
                .get_mut(&event.type_id())
                .unwrap_or(&mut Vec::new())
            {
                info!("Handling event with handler");
                handler.handle(&event);
            }
        }
    }
}

struct Actor {
    name: String,
}

fn handle_type<T, F>(event: &Box<dyn Any + Send + 'static>, mut f: F) -> bool
where
    T: 'static + Send,
    F: FnMut(&T),
{
    info!("Checking event type: {:?}", event.type_id());
   if event.is::<T>() {
        if let Some(value) = event.downcast_ref::<T>() {
            f(value);
            return true; // Downcast and processing succeeded
        }
    }
    false // Type mismatch or downcast failed
}

impl Handler for Actor {
    async fn handle(&mut self, event: &Box<dyn Any + Send + 'static>) {
        info!("{} handling event", self.name);
        handle_type(event, |s: &String| {
            self.name = s.clone();
            info!("{} received String event: {}", self.name, s);
        });
        handle_type::<i32, _>(event, |i| {
            info!("{} received i32 event: {}", self.name, i);
        });
        handle_type::<MulticastEvent, _>(event, |mi| {
            info!("{} received MulticastEvent event: {:?}", self.name, mi);
        });
    }
}