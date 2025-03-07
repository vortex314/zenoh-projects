use log::{debug, error,info};

use alloc::vec::Vec;
use alloc::collections::BTreeMap;

use embassy_time::Duration;
use embassy_time::Instant;

use anyhow::Result;

use super::async_wait_millis;
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
        let now = Instant::now();
        if self.active && self.expires_at > now {
            self.expires_at - now
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
        debug!("adding timer {:?}", timer);
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
    pub fn with_timer<F>(&mut self, id: u32, func : F ) -> Result<()> where F: Fn(&mut Timer)-> () {
        self.timers.get_mut(&id).map(|timer| func(timer)).ok_or_else(|| anyhow::anyhow!("Timer not found"))
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
    pub fn start(&mut self, id: u32) {
        if let Some(timer) = self.timers.get_mut(&id) {
            timer.start();
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