#![allow(unused_imports)]
#![allow(dead_code)]
use std::sync::Arc;

use chrono::offset;

use serde::de;
use tokio::select;
use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

mod logger;
use log::{debug, info};

mod protocol;
use protocol::msg::ProxyMessage;
use protocol::*;

mod proxy_server;
use proxy_server::*;

mod limero;
use limero::*;

mod port_scanner;
use port_scanner::*;

mod transport;
use transport::*;
#[allow(unused_imports)]


use core::result::Result;

fn start_proxy(event: PortScannerEvent) -> Option<()> {
    match event {
        PortScannerEvent::PortAdded { port } => {
            info!("Port added : {:?}", port.port_name);
            let mut transport = Transport::new(port.clone());
            let mut proxy_server = ProxyServer::new( port,transport.sink_ref());
            tokio::spawn(async move {
                transport.run().await;
            });
            tokio::spawn(async move {
                proxy_server.run().await;
            });
        }
        PortScannerEvent::PortRemoved { port } => {
            info!("Port removed : {:?}", port);
        }
    }
    None
}

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<(), Error> {
    logger::init();
    info!("Starting Serial Proxy");

    let  null_sink = Sink::<()>::new(1);

    let port_patterns = vec![PortPattern {
        name_regexp: "/dev/tty.*".to_string(),
        vid: Some(4292),
        pid: None,
        serial_number: None,
    }];

    let mut port_scanner = PortScanner::new(port_patterns);
    connect(&mut port_scanner, start_proxy, null_sink.sink_ref()); // start a proxy when port detected

    select! {
        _ = port_scanner.run()  => {
            info!("Port scanner task finished !! ");
        }
    }
    Ok(())
}
