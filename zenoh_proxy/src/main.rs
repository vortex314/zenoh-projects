#![allow(unused_imports)]
#![allow(dead_code)]
use std::sync::Arc;

use chrono::offset;

use serde::de;
use tokio::select;
use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

use log::{debug, info};


mod proxy_server;
use proxy_server::*;


use limero::*;

mod port_scanner;
use port_scanner::*;

mod transport;
#[allow(unused_imports)]
use core::result::Result;
use transport::*;


mod pubsub;
use pubsub::*;
use pubsub::PubSubCmd;
use pubsub::PubSubEvent;

fn start_proxy(event: &PortScannerEvent)  {
    match event {
        PortScannerEvent::PortAdded { port } => {
            info!("Port added : {:?}", port.port_name);
            let mut transport = Transport::new(port.clone());
            let mut pubsub_actor = ZenohPubSubActor::new();
            let mut proxy_server = ProxySession::new(pubsub_actor.handler(), transport.handler());
            /*transport.map_to(
                |ev| Some(ProxyServerCmd::TransportEvent(ev)),
                proxy_server.sink_ref(),
            );
            pubsub_actor.map_to(
                |ev| Some(ProxyServerCmd::PubSubEvent(ev)),
                proxy_server.sink_ref(),
            );*/
            tokio::spawn(async move {
                transport.run().await;
            });
            tokio::spawn(async move {
                proxy_server.run().await;
            });
            tokio::spawn(async move {
                pubsub_actor.run().await;
            });
        }
        PortScannerEvent::PortRemoved { port } => {
            info!("Port removed : {:?}", port);
        }
    }
}

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Error> {
    logger::init();
    info!("Starting Serial Proxy");

    let port_patterns = vec![PortPattern {
        name_regexp: "/dev/tty.*".to_string(),
        vid: None, // Some(4292),
        pid: None,
        serial_number: None,
    }];

    let mut port_scanner = PortScanner::new(port_patterns);
    port_scanner.for_each( start_proxy); // start a proxy when port detected

    select! {
        _ = port_scanner.run()  => {
            info!("Port scanner task finished !! ");
        }
    }
    Ok(())
}
