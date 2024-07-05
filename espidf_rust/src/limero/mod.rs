#![allow(unused_imports)]
#![allow(dead_code)]
#[cfg(feature = "esp32")]
use {
    alloc::boxed::Box, alloc::string::String, embassy_sync::channel::Channel,
    embassy_sync::channel::DynamicReceiver, embassy_sync::channel::DynamicSender,
    embassy_sync::channel::Receiver, embassy_sync::channel::Sender,
};

use alloc::collections::BTreeMap;
use alloc::sync::Arc;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::marker::PhantomData;
use core::ops::Shr;
use core::result::Result;
use embassy_sync::blocking_mutex::raw::RawMutex;
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

pub trait SourceTrait<M> where M : Clone + Send + Sync{
    fn subscribe(&mut self, sink_ref:SinkRef<M>);
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
    M: Clone + Send + Sync ,
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

impl<M, T, const N: usize> DynSender<T> for Channel<M, T, N>
where
    M: RawMutex,
{
    fn try_send(&self, message: T) -> Result<(), String> {
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

pub struct Source<T> where T: Clone + Send + Sync+ 'static{
    sinks: Vec<SinkRef<T>>,
}

impl<T> Source<T> where T: Clone + Send + Sync{
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
    fn subscribe(&mut self, sink_ref: SinkRef<T>) {
        self.sinks.push(sink_ref);
    }
}

struct FlowState<T, U> where U:Clone+Sync+Send+'static ,T:Clone+Sync+Send {
    func: fn(T) -> Option<U>,
    sink_ref:SinkRef<U>,
}

impl<T,U> FlowState <T,U> where T: Clone + Send + Sync, U: Clone + Send + Sync{
    pub fn new(func: fn(T) -> Option<U>,sink_ref : SinkRef<U>) -> Self {
        Self {
            func,
            sink_ref,
        }
    }
    pub fn push(&self, t: T) {
        (self.func)(t).map(|u| self.src.emit(u));
    }
}

impl<T,U> DynSender<T> for FlowState<T,U> where T: Clone + Send + Sync, U: Clone + Send + Sync{
    fn try_send(&self, t: T) -> Result<(), String> {
        self.push(t);
        Ok(())
    }
}

pub struct FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync+'static,
{
    flow_state: Arc<FlowState<T,U>>,
}


impl<T, U> FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    pub fn new(func: fn(T) -> Option<U>,sink_ref : SinkRef<U>) -> Self
    where
        T: Clone + Send + Sync,
        U: Clone + Send + Sync,
    {
        FlowFunction {
            flow_state: Arc::new(FlowState {
                func,
                sink_ref,
            }),
        }
    }
    fn sink_ref(&self) -> SinkRef<T> where T: Clone + Send + Sync{
        SinkRef { channel: self.flow_state.clone() }
    }

    pub fn push(&self, t: T) {
        self.flow_state.push(t);
    }
}

impl<T, U> SourceTrait<U> for FlowFunction<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync,
{
    fn subscribe(&mut self, sink_ref    : SinkRef<U>) {
        self.flow_state.src.subscribe(sink_ref);
    }
}



impl<C, E> DynSender<C> for FlowFunction<C, E>
where
    C: Clone + Send + Sync + 'static,
    E: Clone + Send + Sync + 'static,
{
    fn try_send(&self, message: C) -> Result<(), String> {
        self.push(message);
        Ok(())
    }
}

pub struct FlowMap<T, U>
where
    T: Clone + Send + Sync,
    U: Clone + Send + Sync+'static,
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
    fn subscribe(&mut self, sink: SinkRef<U>) {
        self.src.subscribe(sink);
    }
}

/*impl<T> Shr<Box<dyn SinkTrait<T>>> for &mut dyn SourceTrait<T> {
    type Output = ();
    fn shr(self, sink: SinkRef<T>) -> () {
        (*self).subscribe(sink);
    }
}*/

pub fn connect<T, U>(src: &mut dyn SourceTrait<T>, func: fn(T) -> Option<U>, sink_ref: SinkRef<U>)
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowFunction::new(func,sink_ref);
    src.subscribe(flow.sink_ref());
}

/*pub fn connect_map<T, U>(src: &mut dyn SourceTrait<T>, func: fn(T) -> Option<U>, sink: SinkRef<U>)
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let mut flow = FlowMap::new(func);
    flow.subscribe(sink);
    src.subscribe(flow.sink_ref());
}*/
/* 
pub trait ActorTrait<C, E>
where
    C: Clone + Send + Sync + 'static,
    E: Clone + Send + Sync + 'static,
{
    fn command_sink(&self) -> SinkRef<C>;
    fn add_event_listener(&mut self, sink_ref: SinkRef<E>);
}

pub fn connect_actors<C1, E1, C2, E2>(
    src: &mut dyn ActorTrait<C1, E1>,
    func: fn(E1) -> Option<C2>,
    dst: &dyn ActorTrait<C2, E2>,
) where
    C1: Clone + Send + Sync + 'static,
    E1: Clone + Send + Sync + 'static,
    C2: Clone + Send + Sync + 'static,
    E2: Clone + Send + Sync + 'static,
{
    let mut flow = FlowFunction::new(func);
    flow.subscribe(Box::new(dst.command_sink()));
    src.add_event_listener(flow.sink_ref());
}*/

/*
trait Actor<C,E> where C:Clone+Send+Sync+'static,E:Clone+Send+Sync+'static{
    async fn run(&mut self);
    fn command(&self) -> Sender<C>;
    fn subscribe(&mut self, sink_ref : Sender<E>);
}

*/
