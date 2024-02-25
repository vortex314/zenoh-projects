
# Encountered bugs

## Zenohd doesn´t understand zenoh-pico

Router side 
```
[2024-02-25T10:25:22Z DEBUG zenoh_transport::unicast::universal::rx] Transport: a058bc2c1c647aac6888b385d661c6a3. Message handling not implemented: TransportMessage { body: InitSyn(InitSyn { version: 8, whatami: Client, zid: bc1e8358b0403bcd75c8957bfba66e14, resolution: Resolution(10), batch_size: 65535, ext_qos: None, ext_shm: None, ext_auth: None, ext_mlink: None, ext_lowlatency: None, ext_compression: None }) }

```
Comes from this part in the code
```rust
pub(super) fn read_messages(&self, mut batch: RBatch, link: &Link) -> ZResult<()> {
        while !batch.is_empty() {
            let msg: TransportMessage = batch
                .decode()
                .map_err(|_| zerror!("{}: decoding error", link))?;

            log::trace!("Received: {:?}", msg);

            #[cfg(feature = "stats")]
            {
                self.stats.inc_rx_t_msgs(1);
            }

            match msg.body {
                TransportBody::Frame(msg) => self.handle_frame(msg)?,
                TransportBody::Fragment(fragment) => self.handle_fragment(fragment)?,
                TransportBody::Close(Close { reason, session }) => {
                    self.handle_close(link, reason, session)?
                }
                TransportBody::KeepAlive(KeepAlive { .. }) => {}
                _ => {
                    log::debug!(
                        "Transport: {}. Message handling not implemented: {:?}",
                        self.config.zid,
                        msg
                    );
                }
            }
        }

        // Process the received message

        Ok(())
    }


```
Client side on µC
```
 >>> Starting Zenoh-Pico Publisher...
 Opening Zenoh Session...  
␛[0;32mI (43207) uart: queue free spaces: 100␛[0m
[1970-01-01T00:00:42Z INFO ::_z_unicast_open_client] Sending Z_INIT(Syn)
[1970-01-01T00:00:42Z DEBUG ::_z_init_encode] Encoding _Z_MID_T_INIT
[1970-01-01T00:00:44Z DEBUG ::_z_keep_alive_decode] Decoding _Z_MID_T_KEEP_ALIVE
Unable to open session!
 >>> Starting Zenoh-Pico Publisher...
 Opening Zenoh Session...  
␛[0;32mI (45717) uart: queue free spaces: 100␛[0m
[1970-01-01T00:00:45Z INFO ::_z_unicast_open_client] Sending Z_INIT(Syn)
[1970-01-01T00:00:45Z DEBUG ::_z_init_encode] Encoding _Z_MID_T_INIT
[1970-01-01T00:00:46Z DEBUG ::_z_keep_alive_decode] Decoding _Z_MID_T_KEEP_ALIVE
Unable to open session!
 >>> Starting Zenoh-Pico Publisher...
 Opening Zenoh Session...  
```
## Looping threads in ESP32
When the zenoh is not present, it looks very much like the zenoh background threads are blocking. They prevent the watchdog timer to execute. 
```
␛[0;31mE (346807) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:␛[0m
␛[0;31mE (346807) task_wdt:  - IDLE (CPU 0)␛[0m
␛[0;31mE (346807) task_wdt: Tasks currently running:␛[0m
␛[0;31mE (346807) task_wdt: CPU 0: main␛[0m
␛[0;31mE (346807) task_wdt: CPU 1: IDLE␛[0m
␛[0;31mE (346807) task_wdt: Print CPU 0 (current core) backtrace␛[0m


Backtrace: 0x400F8D06:0x3FFB06D0 0x40082721:0x3FFB06F0 0x4000C04D:0x3FFB50A0 0x400F409A:0x3FFB50C0 0x400D54E1:0x3FFB50E0 0x400D557F:0x3FFB5100 0x400DB13D:0x3FFB5130 0x400D7181:0x3FFB5160 0x400D6CFC:0x3FFB5180 0x400DB358:0x3FFB51A0 0x400D5F5D:0x3FFB5240 0x400D55C5:0x3FFB5460 0x400D56B9:0x3FFB5520 0x400D2BC9:0x3FFB5540 0x400D2DB9:0x3FFB5570 0x400D2174:0x3FFB5600 0x400D1F4A:0x3FFB5620 0x40107CD8:0x3FFB5780 0x40089859:0x3FFB57A0

␛[0;31mE (346807) task_wdt: Print CPU 1 backtrace␛[0m


Backtrace: 0x40084CE1:0x3FFB0CD0 0x40082721:0x3FFB0CF0 0x4000BFED:0x3FFB65A0 0x40089B0A:0x3FFB65B0 0x400F8FCB:0x3FFB65D0 0x400F8FD7:0x3FFB6600 0x400E15B1:0x3FFB6620 0x4008818D:0x3FFB6640 0x40089859:0x3FFB6660



```

