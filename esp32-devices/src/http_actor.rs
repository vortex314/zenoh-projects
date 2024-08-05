use limero::ActorTrait;

enum TcpCmd {
    Connect,
    Disconnect,
    Send,
    Receive,
}

enum TcpEvent {
    Connected,
    Disconnected,
    DataReceived,
    DataSent,
}

struct TcpActor {
    cmds : Sink<TcpCmd, 3>,
    events : Source<TcpEvent>,
    stack: &'static mut SmoltcpStack<'static>,
    rx_buffer: &'static mut [u8],
    tx_buffer: &'static mut [u8],
}

impl TcpActor {
    pub fn new(stack: &'static mut SmoltcpStack<'static>) -> Self {
        let mut rx_buffer = Box::leak(Box::new([0; 4096]);
        let mut tx_buffer = Box::leak(Box::new([0; 4096]);
        TcpActor {
            stack,
            rx_buffer,
            tx_buffer,
        }
    }
    fn do_http(&mut self ) {
        loop {
            Timer::after(Duration::from_millis(1_000)).await;
        
            let mut socket = TcpSocket::new(&stack, &mut rx_buffer, &mut tx_buffer);
        
            socket.set_timeout(Some(embassy_time::Duration::from_secs(10)));
        
            let remote_endpoint = (Ipv4Address::new(142, 250, 185, 115), 80);
            info!("connecting...");
            let r = socket.connect(remote_endpoint).await;
            if let Err(e) = r {
                info!("connect error: {:?}", e);
                continue;
            }
            info!("connected!");
            let mut buf = [0; 1024];
            loop {
                use embedded_io_async::Write;
                let r = socket
                    .write_all(b"GET / HTTP/1.0\r\nHost: www.mobile-j.de\r\n\r\n")
                    .await;
                if let Err(e) = r {
                    info!("write error: {:?}", e);
                    break;
                }
                let n = match socket.read(&mut buf).await {
                    Ok(0) => {
                        info!("read EOF");
                        break;
                    }
                    Ok(n) => n,
                    Err(e) => {
                        info!("read error: {:?}", e);
                        break;
                    }
                };
                info!("{}", core::str::from_utf8(&buf[..n]).unwrap());
                info!(" heap_free: {}",ALLOCATOR.free());
            }
            Timer::after(Duration::from_millis(3000)).await;
        }
    }
}

impl ActorTrait<HttpCmd,HttpEvent> for TcpActor {
 async fn run(&mut self) {
    loop {
        let _cmd = self.cmds.receive().await;
    }
}
fn add_listener(&mut self, listener: SinkRef<TcpEvent>) {
    self.events.connect(listener);
}

}

