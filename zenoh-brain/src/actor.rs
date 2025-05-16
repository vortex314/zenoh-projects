#![allow(unused_mut)]
#![allow(unused_variables)]
#![allow(dead_code)]

use std::collections::HashMap;
use tokio::select;
use tokio::sync::mpsc::Receiver;
use tokio::sync::mpsc::Sender;
use tokio::time::Duration;
use tokio::time::Instant;

use log::error;
use log::info;

use anyhow::Result;

struct Timer {
    active: bool,
    repeat: bool,
    interval: Duration,
    next_expiration: Instant,
}

pub struct Timers {
    timers: HashMap<String, Timer>,
}

impl Timers {
    pub fn new() -> Self {
        Timers {
            timers: HashMap::new(),
        }
    }

    fn add_timer(&mut self, name: String, interval: Duration, active: bool, repeat: bool) {
        let timer = Timer {
            active,
            repeat,
            interval,
            next_expiration: Instant::now() + interval,
        };
        self.timers.insert(name, timer);
    }

    pub fn add_repeat_timer(&mut self, name: String, interval: Duration) {
        let timer = Timer {
            active: true,
            repeat: true,
            interval,
            next_expiration: Instant::now() + interval,
        };
        self.timers.insert(name, timer);
    }

    pub fn add_one_shot_timer(&mut self, name: String, interval: Duration) {
        let timer = Timer {
            active: true,
            repeat: false,
            interval,
            next_expiration: Instant::now() + interval,
        };
        self.timers.insert(name, timer);
    }

    fn remove_timer(&mut self, name: &str) {
        self.timers.remove(name);
    }
    fn stop_timer(&mut self, name: &str) {
        if let Some(timer) = self.timers.get_mut(name) {
            timer.active = false;
        }
    }
    pub fn duration_till_expiration(&self, name: &str) -> Option<Duration> {
        if let Some(timer) = self.timers.get(name) {
            if timer.next_expiration > Instant::now() {
                return Some(timer.next_expiration - Instant::now());
            }
        }
        None
    }

    fn reset_timer(&mut self, name: &str) {
        if let Some(timer) = self.timers.get_mut(name) {
            timer.next_expiration += timer.interval;
        }
    }

    pub fn lowest_duration(&self) -> Option<Duration> {
        let mut lowest = None;
        for timer in self.timers.values() {
            if timer.active {
                if let Some(lowest_duration) = lowest {
                    if timer.next_expiration < lowest_duration {
                        lowest = Some(timer.next_expiration);
                    }
                } else {
                    lowest = Some(timer.next_expiration);
                }
            }
        }
        lowest.map(|expiry| {
            if expiry > Instant::now() {
                expiry - Instant::now()
            } else {
                Duration::from_millis(0)
            }
        })
    }

    pub fn find_expired_timers(&self) -> Vec<String> {
        let mut expired_timers = Vec::new();
        let now = Instant::now();
        for (name, timer) in self.timers.iter() {
            if timer.active && timer.next_expiration <= now {
                expired_timers.push(name.clone());
            }
        }

        expired_timers
    }

    pub async fn expired_timers(&mut self) -> Vec<String> {
        // find minmum expiration time
        let mut expired_timers = self.find_expired_timers();
        if expired_timers.len() == 0 {
            // sleep for the next timer
            let next_expiration = self
                .lowest_duration()
                .unwrap_or(Duration::from_millis(1000));
            tokio::time::sleep(next_expiration).await;
            // check for expired timers again
            expired_timers = self.find_expired_timers();
        }
        for name in expired_timers.iter() {
            if let Some(timer) = self.timers.get(name) {
                if timer.repeat {
                    self.reset_timer(name);
                } else {
                    self.stop_timer(name);
                }
            }
        }
        expired_timers
    }
}

pub struct Actor<Cmd, Evt> {
    tx_cmd: Sender<Cmd>,
    pub rx_cmd: Receiver<Cmd>,
    event_handlers: Vec<Box<dyn FnMut(&Evt) + Send>>,
    pub timers: Timers,
}

impl<Cmd, Evt> Actor<Cmd, Evt> {
    pub fn sender(&self) -> Sender<Cmd>{
        self.tx_cmd.clone()
    }

    pub fn tell(&self, cmd: Cmd) {
        self.tx_cmd.try_send(cmd);
    }
    pub fn emit(&mut self, event: &Evt) {
        for handler in self.event_handlers.iter_mut() {
            handler(event);
        }
    }

    pub fn new() -> Self {
        let (tx_cmd, rx_cmd) = tokio::sync::mpsc::channel(100);
        Actor {
            tx_cmd,
            rx_cmd,
            event_handlers: Vec::new(),
            timers: Timers::new(),
        }
    }

/* 
    async fn run(&mut self) -> Result<()> {
        self.actor_impl
            .on_start()
            .await
            .iter()
            .for_each(|ev| self.emit(ev));
        loop {
            select! {
                cmd = self.rx_cmd.recv() => {
                    cmd.iter().for_each(|cmd| {
                        for event in self.actor_impl.on_cmd(&cmd).await {
                            self.emit(&event);
                            }});
                },
                expired_timers  = self.timers.expired_timers() => {
                    expired_timers.iter().for_each(|timer_name| {
                        for event in self.actor_impl.on_timer(&timer_name).await {
                            self.emit(&event);
                        }
                    });
                }
            }
        }
    }
*/
    pub fn on_event<FUNC: FnMut(&Evt) + 'static + Send>(&mut self, f: FUNC) -> () {
        self.event_handlers.push(Box::new(f));
    }
}

pub trait ActorImpl<Cmd, Evt> {
    fn tell(&self,cmd:Cmd) ;
    fn sender(&self) -> Sender<Cmd>;
    fn on_event<FUNC: FnMut(&Evt) + 'static + Send>(&mut self, f: FUNC)  ;
    async fn run(&mut self)  ;
}
/*
struct ZenohCmd {
    sleep: u32,
}

#[derive(Debug)]
struct ZenohEvent {
    slept: u32,
}

struct ZenohActor {}

impl ActorImpl<ZenohCmd, ZenohEvent> for ZenohActor {
    fn on_cmd(&mut self, _cmd: &ZenohCmd) -> Vec<ZenohEvent> {}
    fn on_timer(&mut self, _timer: &str) -> Ves<ZenohEvent> {}
}

fn main() {
    let mut zenoh_actor = Actor::new(ZenohActor {});
    zenoh_actor.tell(&ZenohCmd { sleep: 100 });
    zenoh_actor.on_event(|x| println!("{:?}", x));
    println!("Hello, world!");
}*/
