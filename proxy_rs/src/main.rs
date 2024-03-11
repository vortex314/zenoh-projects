use std::sync::Arc;

use tokio::sync::Mutex;
#[allow(unused_imports)]
use tokio_serial::*;

#[tokio::main(worker_threads = 1)]
async fn main() -> Result<()> {
    let active_ports = Arc::new(Mutex::new(Vec::new()));
    //    let (tx,rx) = tokio::sync::mpsc::channel(100);
    // spawn task to collect new serial USB devices
    let a_ports = active_ports.clone();
    let _port_scanner_task = tokio::spawn(async move {
        loop {
            let mut active_ports = a_ports.lock().await;
            println!("Scanning for new ports {} ", active_ports.len());

            let scanned_ports = available_ports().unwrap();
            scanned_ports.iter().for_each(|port| {
                if !active_ports.contains(port) {
                    active_ports.push(port.clone());
                    println!("New port available: {:?}", port);
                    //                   tx.try_send(port.clone()).unwrap();
                }
            });
            drop(active_ports);

            tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
        }
    });
    // spawn task to read from serial port

    let _port_remover_task = tokio::spawn(async move {
        loop {
            let mut active_ports = active_ports.lock().await;
            println!("Remove port 0");
            active_ports.remove(0);
            drop(active_ports);
            tokio::time::sleep(tokio::time::Duration::from_secs(3)).await;
        }
    });

    tokio::time::sleep(tokio::time::Duration::from_secs(50)).await;

    Ok(())
}
