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
use core::any::type_name;
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
    fn send(&self, m: M);
}

pub trait SourceTrait<M> where M : Clone + Send + Sync +'static{
    fn add_listener(&mut self, sink_ref:SinkRef<M>);
    fn map_to<U>(&mut self,func: fn(M) -> Option<U>,sink_ref:SinkRef<U>) where U: Clone + Send + Sync 
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

pub struct Sink<M, const N: usize> {
    channel: Arc<Channel<CriticalSectionRawMutex, M, N>>,
}

trait DynSender<T> {
    fn send(&self, message: T) ;
}

#[derive(Clone)]
pub struct SinkRef<M>
where
    M: Clone + Send + Sync + 'static,
{
    sender: Arc<dyn DynSender<M> + Send + Sync>,
}





impl<M, const N: usize> Sink<M, N>
where
    M: Clone + Send + Sync ,
{
    pub fn new() -> Self {
        let channel = Arc::new(Channel::new());
        Sink { channel }
    }
    pub async fn next(&mut self) -> Option<M> {
        Some(self.channel.receive().await)
    }
    pub fn sink_ref(&self) -> SinkRef<M> {
        SinkRef {
            sender: self.channel.clone(),
        }
    }
}





impl<M, T, const N: usize> DynSender<T> for Channel<M, T, N>
where
    M: RawMutex,
{
    fn send(&self, message: T)  {
        match self.try_send(message) {
            Ok(()) => {},
            Err(_) => {error!("Send fails on channel : Queue Full for {} " ,type_name::<T>());},
        };
    }
}



impl<M> SinkTrait<M> for SinkRef<M>
where
    M: Clone + Send + Sync,
{
    fn send(&self, message: M) {
        let _ = self.sender.send(message);
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
            sink.send(m.clone());
        }
    }
}

impl<T> SourceTrait<T> for Source<T>
where
    T: Clone + Send + Sync,
{
    fn add_listener(&mut self, sink_ref: SinkRef<T>) {
        self.sinks.push(sink_ref);
    }
}

struct FlowState<T, U> where U:Clone+Sync+Send+'static ,T:Clone+Sync+Send {
    func: fn(T) -> Option<U>,
    sink_ref:SinkRef<U>,
}


impl<T,U> DynSender<T> for FlowState<T,U> where T: Clone + Send + Sync, U: Clone + Send + Sync{
    fn send(&self, t: T)  {
        (self.func)(t).map(|u| self.sink_ref.send(u));    }
}

#[derive(Clone)]
pub struct SinkFunction<M> {
    func : Arc< fn(M) -> ()>
}

impl<M> SinkFunction<M> {
    pub fn new(func: fn(M) -> ()) -> Self {
        SinkFunction {
            func: Arc::new(func),
        }
    }
    fn sink_ref(&self) -> SinkRef<M> where M: Clone + Send + Sync{
        SinkRef { sender: Arc::new(self.clone()) }
    }
}

impl<M> SinkTrait<M> for SinkFunction<M> {
    fn send(&self, _message: M) {
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
        SinkRef { sender: self.flow_state.clone() }
    }

    /*pub fn push(&self, t: T) {
        (self.flow_state.func)(t).map(|u| self.flow_state.sink_ref.push(u));
    }*/
}


impl<C, E> DynSender<C> for FlowFunction<C, E>
where
    C: Clone + Send + Sync + 'static,
    E: Clone + Send + Sync + 'static,
{
    fn send(&self, message: C)  {
        self.flow_state.send(message);
        
    }
}


/*pub fn connect<T, U>(src: &mut dyn SourceTrait<T>, func: fn(T) -> Option<U>, sink_ref: SinkRef<U>)
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    let flow = FlowFunction::new(func,sink_ref);
    src.add_listener(flow.sink_ref());
}*/

pub trait ActorTrait<T, U> // : SinkTrait<T> + SourceTrait<U>
where
    T: Clone + Send + Sync + 'static,
    U: Clone + Send + Sync + 'static,
{
    async fn run(&mut self);
    fn sink_ref(&self) -> SinkRef<T>;
    fn add_listener(&mut self, sink_ref: SinkRef<U>);
}