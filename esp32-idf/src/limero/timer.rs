


use std::collections::BTreeMap;
use std::time::{Duration, Instant};

use log::error;
use log::info;

use super::async_wait_millis;
const FOREVER : Duration = Duration::from_millis(0xFFFFFFFF);
#[derive(Debug, Clone, Copy)]
pub struct Timer {
    expires_at: Instant,
    re_trigger: bool,
    interval: Duration,
    active: bool,
    id: u32,
}

impl Timer {
    pub fn new(id: u32, interval: Duration) -> Self {
        Timer {
            expires_at: Instant::now() + interval,
            re_trigger: false,
            interval,
            active: false,
            id,
        }
    }
    pub fn new_repeater(id: u32, interval: Duration) -> Self {
        Timer {
            expires_at: Instant::now() + interval,
            re_trigger: true,
            interval,
            active: true,
            id,
        }
    }
    pub fn id(&self) -> u32 {
        self.id
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
    pub fn reload(&mut self) {
        if self.active {
            if self.re_trigger {
                self.expires_at = Instant::now() + self.interval;
            } else {
                self.active = false;
            }
        }
    }
    pub fn expired(&self) -> bool {
        Instant::now() >= self.expires_at && self.active
    }
    pub fn wait_time(&self) -> Duration {
        if self.active && self.expires_at > Instant::now() {
            self.expires_at - Instant::now()
        } else {
            Duration::from_millis(0)
        }
    }
}

pub struct Timers {
    timers: BTreeMap<u32, Timer>,
}

impl Timers {
    pub fn new() -> Self {
        Timers {
            timers: BTreeMap::new(),
        }
    }
    pub fn add_timer(&mut self, timer: Timer) {
        info!("adding timer {:?}", timer);
        self.timers.insert(timer.id, timer);
    }
    pub fn remove_timer(&mut self, id: u32) {
        self.timers.remove(&id);
    }
    pub fn expired_list(&mut self) -> Vec<u32> {
        let mut expired = Vec::new();
        for (id, timer) in self.timers.iter_mut() {
            if timer.check() {
                expired.push(*id);
            }
        }
        expired
    }
    pub fn set_interval(&mut self, id: u32, interval: Duration) {
        if let Some(timer) = self.timers.get_mut(&id) {
            timer.set_interval(interval);
        }
    }
    pub fn stop(&mut self, id: u32) {
        if let Some(timer) = self.timers.get_mut(&id) {
            timer.stop();
        }
    }
    pub async fn alarm(&mut self) -> u32 {
        let mut lowest_timer: Option<&mut Timer> = None;
        for (_id, timer) in self.timers.iter_mut() {
            if timer.expired() {
                timer.reload();
                return timer.id();
            }
            if timer.active {
                match lowest_timer {
                    Some(ref tim) => {
                        if timer.active && timer.expires_at < tim.expires_at {
                            lowest_timer.replace(timer);
                            }
                        }
                    None => { 
                        lowest_timer.replace(timer);
                    }
                }
            }

        }
        if lowest_timer.is_some() {
            let timer = lowest_timer.unwrap();
            if timer.wait_time() < Duration::from_secs(0) {
               error!("negative duration");// sleep forever 
            }
            async_wait_millis(timer.wait_time().as_millis() as u32).await;
            if timer.expired() {
                timer.reload();
                return timer.id();
            } else {
                0
            }
        } else {
            async_wait_millis(10000000).await;
            0
        }
    }
}