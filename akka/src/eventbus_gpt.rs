/*


Subscribers are registered as async closures using Box<dyn Fn(Box<dyn Any + Send>, Sender) -> Pin<Box<dyn Future<Output = ()> + Send>>>.
Each handler receives:

The event (Box<dyn Any + Send>)
A Sender to publish new events.


The dispatcher runs in a loop, invoking handlers concurrently.

✅ Why this is powerful:

Async closures: Handlers can await inside.
Emit new events easily: Handlers receive a Sender to publish more events.
Concurrent dispatch: Each handler runs in its own task.
Dynamic type matching: Works for any event type using TypeId.

*/

use tokio::sync::mpsc::{self, Sender, Receiver};
use std::any::{Any, TypeId};
use std::collections::HashMap;
use std::sync::Arc;
use std::future::Future;
use std::pin::Pin;
use tokio::task;

type AsyncHandler = Box<
    dyn Fn(Box<dyn Any + Send>, Sender<Box<dyn Any + Send>>) -> Pin<Box<dyn Future<Output = ()> + Send>>
    + Send
    + Sync
>;

struct EventBus {
    tx: Sender<Box<dyn Any + Send>>,
    handlers: Arc<tokio::sync::Mutex<HashMap<TypeId, Vec<AsyncHandler>>>>,
}

impl EventBus {
    fn new() -> (Self, Receiver<Box<dyn Any + Send>>) {
        let (tx, rx) = mpsc::channel(100);
        let handlers = Arc::new(tokio::sync::Mutex::new(HashMap::new()));
        (Self { tx, handlers }, rx)
    }

    async fn publish<T: Any + Send>(&self, event: T) {
        self.tx.send(Box::new(event)).await.unwrap();
    }

    async fn subscribe<T, F, Fut>(&self, handler: F)
    where
        T: Any + Send + 'static,
        F: Fn(T, Sender<Box<dyn Any + Send>>) -> Fut + Send + Sync + 'static,
        Fut: Future<Output = ()> + Send + 'static,
    {
        let type_id = TypeId::of::<T>();
        let tx_clone = self.tx.clone();

        let wrapper: AsyncHandler = Box::new(move |boxed, tx| {
            let tx_inner = tx.clone();
            if let Ok(event) = boxed.downcast::<T>() {
                Box::pin(handler(*event, tx_inner))
            } else {
                Box::pin(async {})
            }
        });

        let mut map = self.handlers.lock().await;
        map.entry(type_id).or_default().push(wrapper);
    }

    async fn run(&self, mut rx: Receiver<Box<dyn Any + Send>>) {
        let handlers = self.handlers.clone();
        let tx_clone = self.tx.clone();

        while let Some(event) = rx.recv().await {
            let type_id = (*event).type_id();
            let map = handlers.lock().await;
            if let Some(list) = map.get(&type_id) {
                for handler in list {
                    let event_clone = event.clone();
                    let tx_inner = tx_clone.clone();
                    task::spawn(handler(event_clone, tx_inner));
                }
            }
        }
    }
}

#[derive(Debug)]
struct UserCreated {
    username: String,
}

#[derive(Debug)]
struct WelcomeEmail {
    email: String,
}

#[tokio::main]
async fn main() {
    let (bus, rx) = EventBus::new();

    // Subscribe async handlers
    bus.subscribe(|user: UserCreated, tx| async move {
        println!("User created: {}", user.username);
        // Emit new event
        tx.send(Box::new(WelcomeEmail {
            email: format!("{}@example.com", user.username),
        }))
        .await
        .unwrap();
    })
    .await;

    bus.subscribe(|email: WelcomeEmail, _tx| async move {
        println!("Sending welcome email to {}", email.email);
        tokio::time::sleep(std::time::Duration::from_secs(1)).await;
        println!("Email sent!");
    })
    .await;

    // Start dispatcher loop
    tokio::spawn(bus.run(rx));

    // Publish initial event
    bus.publish(UserCreated {
        username: "alice".into(),
    })
    .await;

    tokio::time::sleep(std::time::Duration::from_secs(3)).await;
}