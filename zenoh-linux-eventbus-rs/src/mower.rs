struct Mower {
    bus : &Bus
}

impl Mower {
    pub fn new(bus: &Bus) -> Self {
        Mower {
            bus,
        }
    }
}

impl ActorImpl for Mower {
    async fn handle(&mut self, msg: &Arc<dyn Any + Send + Sync>)  {
        if let Some(cmd) = msg.downcast_ref::<MowerCmd>() {
            self.on_cmd(cmd);
        }
    }
}