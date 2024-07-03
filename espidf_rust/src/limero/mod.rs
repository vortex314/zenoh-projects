#![allow(unused_imports)]
#![allow(dead_code)]
#[cfg(feature = "esp32")]
use {
    alloc::boxed::Box, alloc::string::String, embassy_sync::channel::Channel,
    embassy_sync::channel::DynamicReceiver, embassy_sync::channel::DynamicSender,
    embassy_sync::channel::Receiver, embassy_sync::channel::Sender,
};
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

use alloc::collections::BTreeMap;
use alloc::sync::Arc;
use alloc::vec::Vec;
use embassy_sync::blocking_mutex::raw::RawMutex;
use core::marker::PhantomData;
use core::ops::Shr;
use core::result::Result;
use embassy_sync::blocking_mutex::raw::{CriticalSectionRawMutex, NoopRawMutex};
use embassy_time::Duration;
use embassy_time::Instant;
use log::error;
use log::{debug, info};

pub mod timer;
use timer::*;

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

pub struct Sink<M, const N: usize> {
    channel: Arc<Channel<CriticalSectionRawMutex, M, N>>,
}

impl<M, const N: usize> Sink<M, N>
where
    M: Clone + Send + Sync + 'static,
{
    pub fn new() -> Self {
        let channel = Arc::new(Channel::new());
        Sink { channel }
    }
    pub async fn read(&mut self) -> Option<M> {
        Some(self.channel.receive().await)
    }
    pub fn sink_ref(&self) -> SinkRef<M> {
        SinkRef {
            channel: self.channel.clone(),
        }
    }
}

trait DynSender<T> {
    fn try_send(&self, message: T) -> Result<(), String>;
}

impl <M,T,const N:usize> DynSender<T> for Channel<M,T,N> where M : RawMutex{
    fn try_send(&self, message: T) -> Result<(),String> {
        match self.try_send(message) {
            Ok(()) => Ok(()),
            Err(_) => Err(String::from("Channel full")),
        }
    }
}

pub struct SinkRef<M>
where
    M: Clone + Send + Sync + 'static,
{
    channel: Arc<dyn DynSender<M> + Send + Sync>,
}

impl<M> SinkTrait<M> for SinkRef<M>
where
    M: Clone + Send + Sync,
{
    fn push(&self, message: M) {
        let _ = self.channel.try_send(message);
    }
}


/*
impl<M, const N: usize> SinkTrait<M> for Sink<M, N>
where
    M: Clone + Send + Sync,
    dyn SinkTrait<M>: Send + Sync,
{
    fn push(&self, m: M) {
        let _r = self.channel.try_send(m);
    }
}*/





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

pub fn connect<T, U>(
    src: &mut dyn SourceTrait<T>,
    func: fn(T) -> Option<U>,
    sink: SinkRef<U>,
) where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowFunction::new(func);
    flow.subscribe(Box::new(sink));
    src.subscribe(Box::new(flow));
}

pub fn connect_map<T, U>(
    src: &mut dyn SourceTrait<T>,
    func: fn(T) -> Option<U>,
    sink: SinkRef<U>,
) where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowMap::new(func);
    flow.subscribe(Box::new(sink));
    src.subscribe(Box::new(flow));
}


/*
pub trait ActorTrait<T, U,const N:usize>
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    fn run(&mut self);
    fn command_sink(&self) -> SinkRef<T,N>;
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


trait Actor<C,E> where C:Clone+Send+Sync+'static,E:Clone+Send+Sync+'static{
    async fn run(&mut self);
    fn command(&self) -> Sender<C>;
    fn subscribe(&mut self, sink_ref : Sender<E>);
}

*/




