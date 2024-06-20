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
use std::error::Error;
use std::marker::PhantomData;
use std::sync::Mutex as StdMutex;
use std::sync::RwLock;

pub trait SinkTrait<M>: Send + Sync  {
    fn push(&self, m: M);
}

pub trait SourceTrait<M>: Send + Sync {
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<M>>);
}
pub trait Flow<T, U>: SinkTrait<T> + SourceTrait<U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
}

pub struct Sink<M> {
    rx: Receiver<M>,
    tx: Sender<M>,
}

impl<M> Sink<M>
where
    M: Clone + Send + Sync + 'static,
{
    pub fn new(size: usize) -> Self {
        let (tx, rx) = tokio::sync::mpsc::channel(size);
        Sink { tx, rx }
    }
    pub async fn read(&mut self) -> Option<M> {
        self.rx.recv().await
    }
    pub fn sink_ref(&self) -> SinkRef<M> {
        SinkRef::new(self.tx.clone())
    }
}

impl<M> SinkTrait<M> for Sink<M>
where
    M: Clone + Send + Sync,
{
    fn push(&self, m: M) {
        let _r = self.tx.try_send(m);
    }
}

#[derive(Clone)]
pub struct SinkRef<M> {
    sender: Sender<M>,
}

impl<M> SinkRef<M> {
    fn new(sender: Sender<M>) -> Self {
        SinkRef { sender }
    }
}

impl<M> SinkTrait<M> for SinkRef<M>
where
    M: Clone + Send + Sync,
{
    fn push(&self, message: M) {
        self.sender.try_send(message).unwrap();
    }
}
pub struct Source<T> {
    sinks: Vec<Box<dyn SinkTrait<T>>>,
}

impl<T> Source<T> {
    pub fn new() -> Self {
        Self { sinks: Vec::new() }
    }
    pub fn emit(&self, m: T)
    where
        T: Clone + Send + Sync,
    {
        for sink in self.sinks.iter() {
            sink.push(m.clone());
        }
    }
}

impl<T> SourceTrait<T> for Source<T>
where
    T: Clone + Send + Sync,
{
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<T>>) {
        self.sinks.push(sink);
    }
}

pub struct FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    func: fn(T) -> Option<U>,
    src: Source<U>,
    l: PhantomData<T>,
}

impl<T, U> FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    pub fn new(func: fn(T) -> Option<U>) -> Self
    where
        T: Clone + Send + Sync,
        U: Clone + Send + Sync,
    {
        FlowFunction {
            func,
            src: Source::new(),
            l: PhantomData,
        }
    }
}

impl<T, U> SourceTrait<U> for FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<U>>) {
        self.src.add_listener(sink);
    }
}

impl<T, U> SinkTrait<T> for FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn push(&self, t: T) {
        (self.func)(t).map(|u| self.src.emit(u));
    }
}

pub struct FlowMap<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    func: fn(T) -> Option<U>,
    src: Source<U>,
    l: PhantomData<T>,
}

impl<T, U> FlowMap<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    pub fn new(func: fn(T) -> Option<U>) -> Self {
        FlowMap {
            func,
            src: Source::new(),
            l: PhantomData,
        }
    }
}

impl<T, U> SinkTrait<T> for FlowMap<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn push(&self, t: T) {
        (self.func)(t).map(|u| self.src.emit(u));
    }
}

impl<T, U> SourceTrait<U> for FlowMap<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn add_listener(&mut self, sink: Box<dyn SinkTrait<U>>) {
        self.src.add_listener(sink);
    }
}

impl<T> Shr<Box<dyn SinkTrait<T>>> for &mut dyn SourceTrait<T> {
    type Output = ();
    fn shr(self, sink: Box<dyn SinkTrait<T>>) -> () {
        (*self).add_listener(sink);
    }
}

pub fn connect<T,U>(src: &mut dyn SourceTrait<T>, func : fn(T)->Option<U>, sink: SinkRef<U>) where T:Clone+Send+Sync+'static, U:Clone+Send+Sync+'static {
    let mut flow = FlowFunction::new(func);
    flow.add_listener(Box::new(sink));
    src.add_listener(Box::new(flow));
}

pub fn connect_map<T,U>(src: &mut dyn SourceTrait<T>, func : fn(T)->Option<U>, sink: SinkRef<U>) where T:Clone+Send+Sync+'static, U:Clone+Send+Sync+'static {
    let mut flow = FlowMap::new(func);
    flow.add_listener(Box::new(sink));
    src.add_listener(Box::new(flow));
}

pub struct Timer {
    expires_at: Instant,
    re_trigger: bool,
    interval : Duration,
    active: bool,
    id : u32,
}

impl Timer {
    pub fn new(interval: Duration) -> Self {
        Timer {
            expires_at: Instant::now() + interval,
            re_trigger: false,
            interval,
            active: false,
            id : 0,
        }
    }
    pub fn set_interval(&mut self, interval: Duration) {
        self.interval = interval;
    }
    pub fn start(&mut self) {
        self.active = true;
        self.expires_at = Instant::now() + self.interval;
    }
    pub fn stop(&mut self) {
        self.active = false;
    }
    pub fn is_active(&self) -> bool {
        self.active
    }
    pub fn check(&mut self) -> bool {
        if self.active && Instant::now() > self.expires_at {
            self.expires_at = Instant::now() + self.interval;
            return true;
        }
        false
    }
}

pub struct Timers {
    timers: BTreeMap<u32, Timer>,
    next_id: u32,
}

impl Timers {
    pub fn new() -> Self {
        Timers {
            timers: BTreeMap::new(),
            next_id: 0,
        }
    }
    pub fn add_timer(&mut self, timer: Timer) -> u32 {
        let id = self.next_id;
        self.timers.insert(id, timer);
        self.next_id += 1;
        id
    }
    pub fn remove_timer(&mut self, id: u32) {
        self.timers.remove(&id);
    }
    pub fn check(&mut self) -> Vec<u32> {
        let mut expired = Vec::new();
        for (id, timer) in self.timers.iter_mut() {
            if timer.check() {
                expired.push(*id);
            }
        }
        expired
    }
}
