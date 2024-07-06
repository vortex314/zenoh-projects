#![allow(unused_imports)]
#![allow(dead_code)]

use {
    std::borrow::BorrowMut,
    std::cell::RefCell,
    std::collections::BTreeMap,
    std::io::Write,
    std::pin::pin,
    std::rc::Rc,
    std::sync::Arc,
    std::thread::sleep,
    std::time::{Duration, Instant},
    std::vec::Vec,
    std::{ops::Shr, pin::Pin},
};

use minicbor::decode::info;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;
use tokio::sync::Mutex;

use core::result::Result;
use log::error;
use log::{debug, info};
use std::any::type_name;
use std::error::Error;
use std::marker::PhantomData;
use std::sync::Mutex as StdMutex;
use std::sync::RwLock;

mod timer;
pub use timer::*;

pub trait SinkTrait<M>: Send + Sync {
    fn push(&self, m: M);
}

pub trait SourceTrait<M>: Send + Sync+'static
where
    M: Clone + Send + Sync+'static,
{
    fn add_listener(&mut self, sink: SinkRef<M>);
    fn map_to<U>(&mut self,func: fn(M) -> Option<U>,sink_ref:SinkRef<U>) where U: Clone + Send + Sync +'static
    {
        let flow = FlowFunction::new(func,sink_ref);
        self.add_listener(flow.sink_ref());
    }
    fn for_each(&mut self,func: fn(M) -> ()) where M: Clone + Send + Sync 
    {
        let sink_func = SinkFunction::new(func);
        self.add_listener(sink_func.sink_ref());
    }
    /*fn filter<U>(&mut self,func: fn(M) -> bool,sink_ref:SinkRef<U>) where U: Clone + Send + Sync
    {
        let g = |m:M| -> Option<M> { if func(m) {Some(m)} else {None} };
        let flow = FlowFunction::new(g,sink_ref);
        self.add_listener(flow.sink_ref());
    }*/
}
pub trait Flow<T, U>: SinkTrait<T> + SourceTrait<U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync+'static,
{
}

pub struct Sink<M> {
    rx: Receiver<M>,
    tx: Sender<M>,
}

/// Non blocking send
trait DynSender<T> {
    fn send(&self, t: T);
}

#[derive(Clone)]
pub struct SinkRef<M>
where
    M: Clone + Send + Sync,
{
    sender: Arc<dyn DynSender<M> + Send + Sync>,
}

impl<M> Sink<M>
where
    M: Clone + Send + Sync + 'static,
{
    pub fn new(size: usize) -> Self {
        let (tx, rx) = tokio::sync::mpsc::channel(size);
        Sink { tx, rx }
    }
    pub async fn next(&mut self) -> Option<M> {
        self.rx.recv().await
    }
    pub fn sink_ref(&self) -> SinkRef<M> {
        SinkRef {
            sender: Arc::new(self.tx.clone()),
        }
    }
}

impl<M> SinkTrait<M> for Sink<M>
where
    M: Clone + Send + Sync,
{
    fn push(&self, m: M) {
        match self.tx.try_send(m) {
            Ok(_) => (),
            Err(e) => error!("Sink::push() failed {}", e),
        }
    }
}

impl<T> DynSender<T> for Sender<T> {
    fn send(&self, t: T) {
        match self.try_send(t) {
            Ok(_) => (),
            Err(e) => error!("Send error '{}' for {} ", e, type_name::<T>()),
        }
    }
}

impl<M> SinkTrait<M> for SinkRef<M>
where
    M: Clone + Send + Sync,
{
    fn push(&self, message: M) {
        self.sender.send(message);
    }
}
pub struct Source<T>
where
    T: Clone + Send + Sync,
{
    sink_refs: Vec<SinkRef<T>>,
}

impl<T> Source<T>
where
    T: Clone + Send + Sync,
{
    pub fn new() -> Self {
        Self {
            sink_refs: Vec::new(),
        }
    }
    pub fn emit(&self, m: T)
    where
        T: Clone + Send + Sync,
    {
        for sink_ref in self.sink_refs.iter() {
            sink_ref.push(m.clone());
        }
    }
}

impl<T> SourceTrait<T> for Source<T>
where
    T: Clone + Send + Sync+'static,
{
    fn add_listener(&mut self, sink_ref: SinkRef<T>) {
        self.sink_refs.push(sink_ref);
    }
}

struct FlowState<T, U>
where
    U: Clone + Sync + Send + 'static,
    T: Clone + Sync + Send,
{
    func: fn(T) -> Option<U>,
    sink_ref: SinkRef<U>,
}

impl<T, U> FlowState<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    pub fn new(func: fn(T) -> Option<U>, sink_ref: SinkRef<U>) -> Self {
        Self { func, sink_ref }
    }
    pub fn push(&self, t: T) {
        (self.func)(t).map(|u| self.sink_ref.push(u));
    }
}

impl<T, U> DynSender<T> for FlowState<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn send(&self, t: T) {
        self.push(t);
    }
}

#[derive(Clone)]
pub struct SinkFunction<M> {
    func : Arc< fn(M) -> ()>
}

impl<M:'static> SinkFunction<M> {
    pub fn new(func: fn(M) -> ()) -> Self {
        SinkFunction {
            func: Arc::new(func),
        }
    }
    fn sink_ref(&self) -> SinkRef<M> where M: Clone + Send + Sync{
        SinkRef { sender: Arc::new((*self).clone()) }
    }
}

impl<M> SinkTrait<M> for SinkFunction<M> {
    fn push(&self, _message: M) {
        (self.func)(_message);
    }
}

impl<M> DynSender<M> for SinkFunction<M>{
    fn send(&self, _message: M) {
        (self.func)(_message);
    }
}
pub struct FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync + 'static,
{
    flow_state: Arc<FlowState<T, U>>,
}

impl<T, U> FlowFunction<T, U>
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync,
{
    pub fn new(func: fn(T) -> Option<U>, sink_ref: SinkRef<U>) -> Self
    where
        T: Clone + Send + Sync,
        U: Clone + Send + Sync,
    {
        FlowFunction {
            flow_state: Arc::new(FlowState { func, sink_ref }),
        }
    }
    fn sink_ref(&self) -> SinkRef<T>
    where
        T: Clone + Send + Sync,
    {
        SinkRef {
            sender: self.flow_state.clone(),
        }
    }

    pub fn push(&self, t: T) {
        self.flow_state.push(t);
    }
}

impl<C, E> DynSender<C> for FlowFunction<C, E>
where
    C: Clone + Send + Sync + 'static,
    E: Clone + Send + Sync + 'static,
{
    fn send(&self, message: C) {
        self.push(message);
    }
}




pub trait ActorTrait<T, U> // : SinkTrait<T> + SourceTrait<U>
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    async fn run(&mut self);
    fn sink_ref(&self) -> SinkRef<T>;
    fn add_listener(&mut self, sink: SinkRef<U>);
}


