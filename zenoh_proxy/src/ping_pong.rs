use crate::ActorTrait;
use crate::Sink;
use crate::SinkRef;
use crate::Source;
use crate::SourceTrait;
use crate::limero::SinkTrait;
use log::info;
use std::time::Instant;

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
    sink: Sink<PongMsg>,
    ping_sink: SinkRef<PingMsg>,
}

struct Ponger {
    value: u32,
    sink: Sink<PingMsg>,
}

pub async fn do_test() {
    let mut ponger = Ponger {
        value: 0,
        sink: Sink::new(100),
    };
    let mut pinger = Pinger {
        value: 0,
        sink: Sink::new(100),
        ping_sink: ponger.sink_ref(),
    };
    tokio::spawn(async move {
        pinger.run().await;
    });
    tokio::spawn(async move {
        ponger.run().await;
    });
}

impl ActorTrait<PongMsg,()> for Pinger {
    async fn run(&mut self) {
        let mut x = 0;
        let mut start_time = Instant::now();
        self.ping_sink.push(PingMsg {
            value: x,
            reply_to: self.sink_ref(),
        });
        loop {

            match self.sink.next().await {
                Some(msg) => {
                    x = msg.value;
                    self.ping_sink.push(PingMsg { value: x+1 , reply_to: self.sink_ref()});
                    if x%100000 == 0 {
                        let end_time = Instant::now();
                        let elapsed = end_time - start_time;
                        let throughput = 100000 as f64 / elapsed.as_secs_f64();
                        info!("Elapsed time: {:?} msg/sec", throughput );
                        start_time = end_time;
                    }
                }
                None => {
                    println!("Pinger received None");
                }
            }
        }
    }
    fn sink_ref(&self) -> SinkRef<PongMsg> {
        self.sink.sink_ref()
    }

}

impl ActorTrait<PingMsg,()> for Ponger {
    async fn run(&mut self) {
        loop {
            match self.sink.next().await {
                Some(msg) => {
                    self.value = msg.value;
                    msg.reply_to.push(PongMsg { value: self.value });
                }
                None => {
                    println!("Ponger received None");
                }
            }
        }
    }
    fn sink_ref(&self) -> SinkRef<PingMsg> {
        self.sink.sink_ref()
    }

}
