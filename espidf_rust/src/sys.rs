struct Sys {
    pub_sub_events : Sink<SessionEvent>,
    pub_sub_cmds : Sink<SessionCmd>,
}

impl Sys {
    fn new(session_cmds :SinkRef<SessionCmd> , ) -> Sys {
        Sys {}
    }

    async fn run(&self) {
        println!("Sys is running");
    }
}