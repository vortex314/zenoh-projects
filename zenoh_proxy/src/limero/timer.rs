
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
