use crate::limero::ActorTrait;
use crate::limero::Sink;
use crate::limero::SinkRef;
use crate::limero::Source;
use crate::limero::SourceTrait;
use crate::limero::SinkTrait;
use embassy_executor::Spawner;
use embassy_futures::select;
use log::info;
use embassy_time::Instant;
use embassy_futures::select::select;
use embassy_futures::select::Either::{First, Second};

#[derive(Clone) ]
struct PingMsg {
    value: u32,
    reply_to: SinkRef<PongMsg>,
}
#[derive(Clone) ]
struct PongMsg {
    value: u32,
}

struct Pinger {
    value: u32,
    sink: Sink<PongMsg,10>,
    ping_sink: SinkRef<PingMsg>,
}

struct Ponger {
    value: u32,
    sink: Sink<PingMsg,10>,
}

pub async fn do_test(_spawner:Spawner) {
    let mut ponger = Ponger {
        value: 0,
        sink: Sink::new(),
    };
    let mut pinger = Pinger {
        value: 0,
        sink: Sink::new(),
        ping_sink: ponger.sink_ref(),
    };
    select(pinger.run(),ponger.run()).await;

}

impl ActorTrait<PongMsg,()> for Pinger {
    async fn run(&mut self) {
        let mut x = 0;
        let mut start_time = Instant::now();
        self.ping_sink.send(PingMsg {
            value: x,
            reply_to: self.sink_ref(),
        });
        loop {

            match self.sink.next().await {
                Some(msg) => {
                    x = msg.value;
                    self.ping_sink.send(PingMsg { value: x+1 , reply_to: self.sink_ref()});
                    if x%100000 == 0 {
                        let end_time = Instant::now();
                        let elapsed = end_time - start_time;
                        let throughput = (100000 as f64 / elapsed.as_millis() as f64) * 1000.0;
                        info!("{} Elapsed time: {:?} msg/sec",end_time.as_millis(), throughput );
                        start_time = end_time;
                    }
                }
                None => {
                    info!("Pinger received None");
                }
            }
        }
    }
    fn sink_ref(&self) -> SinkRef<PongMsg> {
        self.sink.sink_ref()
    }
    fn add_listener(&mut self, _sink: SinkRef<()>) {
    }
}

impl ActorTrait<PingMsg,()> for Ponger {
    async fn run(&mut self) {
        loop {
            match self.sink.next().await {
                Some(msg) => {
                    self.value = msg.value;
                    msg.reply_to.send(PongMsg { value: self.value });
                }
                None => {
                    info!("Ponger received None");
                }
            }
        }
    }
    fn sink_ref(&self) -> SinkRef<PingMsg> {
        self.sink.sink_ref()
    }
    fn add_listener(&mut self, _sink: SinkRef<()>) {
    }
}
