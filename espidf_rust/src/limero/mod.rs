#![allow(unused_imports)]
#![allow(dead_code)]
#[cfg(feature = "linux")]
use {
    minicbor::decode::info,
    std::borrow::BorrowMut,
    std::cell::RefCell,
    std::collections::BTreeMap,
    std::error::Error,
    std::io::Write,
    std::marker::PhantomData,
    std::pin::pin,
    std::rc::Rc,
    std::sync::Arc,
    std::sync::Mutex as StdMutex,
    std::sync::RwLock,
    std::thread::sleep,
    std::time::{Duration, Instant},
    std::vec::Vec,
    std::{ops::Shr, pin::Pin},
    tokio::sync::mpsc::Receiver,
    tokio::sync::mpsc::Sender,
    tokio::sync::Mutex,
};
#[cfg(feature = "esp32")]
use {
    alloc::boxed::Box,
    alloc::string::String,
    embassy_sync::channel::Channel,
    embassy_sync::channel::DynamicReceiver,
    embassy_sync::channel::DynamicSender,
    embassy_sync::channel::Receiver,
    embassy_sync::channel::Sender,

};

use core::marker::PhantomData;
use core::ops::Shr;
use core::result::Result;
use core::time::Duration;
use alloc::collections::BTreeMap;
use alloc::vec::Vec;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_time::Instant;
use log::error;
use log::{debug, info};


pub trait SinkTrait<M>: Send + Sync {
    fn push(&self, m: M);
}

pub trait SourceTrait<M>: Send + Sync {
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<M>>);
}
pub trait Flow<T, U>: SinkTrait<T> + SourceTrait<U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
}



pub struct Sink<M> {
    channel: Channel<NoopRawMutex, M, 10>,
}

impl<M> Sink<M>
where
    M: Clone + Send + Sync + 'static,
{
    pub fn new(channel:&'static Channel<NoopRawMutex,M,10>) -> Self {
        Sink {
            channel,
        }
    }
    pub async fn read(&mut self) -> Option<M> {
        Some(self.channel.receive().await)
    }
    pub fn sink_ref(&self) -> SinkRef<M> {
        SinkRef::new(self.channel.dyn_sender())
    }
}

impl<M> SinkTrait<M> for Sink<M> where
    M: Clone + Send + Sync,dyn SinkTrait<M>:Send + Sync
{
    fn push(&self, m: M) {
        let _r = self.tx.try_send(m);
    }
}

#[derive(Clone)]
pub struct SinkRef<M> where M: Clone + Send + Sync +'static{
    sender: DynamicSender<'static,M>,
}

impl<M> SinkRef<M> where M: Clone + Send + Sync + 'static{
    pub fn new(sender: DynamicSender<'static,M>) -> Self where M: Clone + Send + Sync{
        SinkRef { sender }
    }
}



impl<M> SinkTrait<M> for SinkRef<M> where M: Clone + Send + Sync{
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
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<T>>) {
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
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<U>>) {
        self.src.subscribe(sink);
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
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<U>>) {
        self.src.subscribe(sink);
    }
}

impl<T> Shr<Box<dyn SinkTrait<T>>> for &mut dyn SourceTrait<T> {
    type Output = ();
    fn shr(self, sink: Box<dyn SinkTrait<T>>) -> () {
        (*self).subscribe(sink);
    }
}

pub fn connect<T, U>(src: &mut dyn SourceTrait<T>, func: fn(T) -> Option<U>, sink: SinkRef<U>)
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowFunction::new(func);
    flow.subscribe(Box::new(sink));
    src.subscribe(Box::new(flow));
}

pub fn connect_map<T, U>(src: &mut dyn SourceTrait<T>, func: fn(T) -> Option<U>, sink: SinkRef<U>)
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowMap::new(func);
    flow.subscribe(Box::new(sink));
    src.subscribe(Box::new(flow));
}

pub struct Timer {
    expires_at: Instant,
    re_trigger: bool,
    interval: Duration,
    active: bool,
    id: u32,
}

impl Timer {
    pub fn new(interval: Duration) -> Self {
        Timer {
            expires_at: Instant::now() + interval,
            re_trigger: false,
            interval,
            active: false,
            id: 0,
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

pub trait ActorTrait<T, U>
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    fn run(&mut self);
    fn command_sink(&self) -> SinkRef<T>;
    fn subscribe(&mut self, sink: Box<dyn SinkTrait<U>>);
}

pub fn connect_actors<T, U, V, W>(
    src: &mut dyn ActorTrait<T, U>,
    func: fn(U) -> Option<V>,
    dst: &dyn ActorTrait<V, W>,
) where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
    V: Clone + Send + Sync + 'static,
    W: Clone + Send + Sync + 'static,
{
    let mut flow = FlowFunction::new(func);
    flow.subscribe(Box::new(dst.command_sink()));
    src.subscribe(Box::new(flow));
}
