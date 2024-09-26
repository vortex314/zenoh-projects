
#![allow(async_fn_in_trait)]


use log::error;
pub mod timer;
pub use timer::Timer;
pub use timer::Timers;

pub mod logger;
pub use logger::init as init_logger;
use tokio::sync::mpsc;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;


pub trait Handler<T>: Send  {
    fn handle(&mut self, item: &T);
}

pub type Endpoint<T> = Box<dyn Handler<T>>;

#[derive(Clone)]
pub struct HandlerFunction<C,F>  where F: FnMut(&C) -> (),C:Send   {
    func: F,
    v: core::marker::PhantomData<C>,
}

impl <C,F> HandlerFunction<C,F>  where F: FnMut(&C) -> (), C:Send {
    pub fn new(func: F) -> Self where F: FnMut(&C) -> (), C:Send {
        Self { func, v: core::marker::PhantomData }
    }
}

impl<C,F> Handler<C> for HandlerFunction<C,F> where F: FnMut(&C) -> ()+Send,C:Send {
    fn handle(&mut self, cmd: &C) {
        (self.func)(cmd);
    }
}

pub trait Actor<CMD, EVENT> {
    async fn run(&mut self);
    fn handler(&self) -> Box<dyn Handler<CMD>>;
    fn add_listener(&mut self, handler: Box<dyn Handler<EVENT>>);
    fn map_to<CMD2>(&mut self, func: fn(&EVENT) -> Option<CMD2>, handler: Box<dyn Handler<CMD2>>)
    where
        CMD2: 'static,
        EVENT: 'static,
    {
        struct MapHandler<E, C> {
            func: fn(&E) -> Option<C>,
            handler: Box<dyn Handler<C>>,
        }

        impl<E, C> Handler<E> for MapHandler<E, C> {
            fn handle(&mut self, event: &E) {
                match (self.func)(event) {
                    Some(cmd) => self.handler.handle(&cmd),
                    None => {}
                }
            }
        }

        let map_handler = MapHandler::<EVENT, CMD2> { func, handler };

        self.add_listener(Box::new(map_handler));
    }
    fn for_each(&mut self, func: fn(&EVENT) -> ())
    where
        EVENT: 'static,
        Self: Actor<CMD, EVENT>,
    {
        struct EventHandlerImpl<C> {
            func: fn(&C) -> (),
        }

        impl<C> Handler<C> for EventHandlerImpl<C> {
            fn handle(&mut self, cmd: &C) {
                (self.func)(cmd);
            }
        }

        let handler = EventHandlerImpl::<EVENT> { func };

        self.add_listener(Box::new(handler));
    }
    fn for_each_event<F>(&mut self, func: Box<F>)
    where
        EVENT: 'static,
        Self: Actor<CMD, EVENT>,
        F: FnMut(&EVENT) -> () + Send + 'static,
    {
        struct EventHandlerImpl<E>  {
            func: Box<dyn FnMut(&E) -> () + Send + 'static>,
        }

        impl<E> Handler<E> for EventHandlerImpl<E> {
            fn handle(&mut self, event: &E) {
                (self.func)(event);
            }
        }

        let handler = EventHandlerImpl::<EVENT> { func };

        self.add_listener(Box::new(handler));
    }

}

pub trait ActorExt<CMD, EVENT,F> {
    fn for_each_event(&mut self, func: Box<F>)
    where
        EVENT: 'static,
        Self: Actor<CMD, EVENT>,
        F: FnMut(&EVENT) -> () + Send + 'static,
    {
        struct EventHandlerImpl<E>  {
            func: Box<dyn FnMut(&E) -> () + Send + 'static>,
        }

        impl<E> Handler<E> for EventHandlerImpl<E> {
            fn handle(&mut self, event: &E) {
                (self.func)(event);
            }
        }

        let handler = EventHandlerImpl::<EVENT> { func };

        self.add_listener(Box::new(handler));
    }
}


pub struct CmdQueue<T> {
    sender : Sender<T>,
    receiver : Receiver<T>,
}

impl<T> CmdQueue<T>
where
    T: 'static + Clone + Send,
{
    pub fn new(capacity: usize) -> Self {
        let (sender,receiver) = mpsc::channel(capacity);
        Self { sender,receiver}
    }

    pub async fn next(&mut self) -> Option<T> {
        self.receiver.recv().await
    }

    pub fn handle(&self, cmd: &T) {
        if let Err(x)  = self.sender.try_send(cmd.clone()) {
            error!("Failed to send command directly via handle {:?}",x);
        }
    }

    pub fn handler(&self) -> Box<dyn Handler<T>> {
        struct HandlerImpl<E>
        where
            E: 'static,
        {
            sender: Sender<E>,
        }

        impl<'ch,E> Handler<E> for HandlerImpl<E>
        where
            E: Clone + Send,
        {
            fn handle(&mut self, event: &E) {
                let r = self.sender.try_send(event.clone());
                if r.is_err() {
                    error!("Failed to send event");
                }
            }
        }

        let handler = HandlerImpl::<T> {
            sender: self.sender.clone(),
        };

        Box::new(handler)
    }

    pub fn sender(&self) -> Sender<T> {
        self.sender.clone()
    }
}

pub struct EventHandlers<T> {
    handlers: Vec<Box<dyn Handler<T>>>,
}

impl<T> EventHandlers<T> {
    pub fn new() -> Self {
        Self {
            handlers: Vec::new(),
        }
    }
    pub fn add_listener(&mut self, handler: Box<dyn Handler<T>>) {
        self.handlers.push(handler);
    }
}

impl<T> Handler<T> for EventHandlers<T> {
    fn handle(&mut self, event: &T) {
        for handler in self.handlers.iter_mut() {
            handler.handle(event);
        }
    }
}

pub async fn async_wait_millis(millis: u32) -> () {
    tokio::time::sleep(tokio::time::Duration::from_millis(millis as u64)).await;
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        
        assert_eq!(5, 4);
    }
}
