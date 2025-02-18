# Teting Zenoh via Serial on ESP32 US
- preliminary tests to validate Zenoh over serial 
- I need to repeat these tests because results and bugs could have evolved a lot since.



# Encountered bugs
- Version of zenohd needs to be synced with zenoh-pico - otherwise failures
- Timers inside zenoh are not matched to 100Hz resolution of FreeRTOS default timer tick
- USB ports like /dev/ttyUSBx evolve over time when plugged an un-plugged
- Zenoh broker side doesn't handle disconnect/reconnect correctly

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
After increasing the wait time between sessions to 10 sec 
```
>>> Starting Zenoh-Pico Publisher...
 Opening Zenoh Session...  
␛[0;32mI (476) uart: queue free spaces: 100␛[0m
[1970-01-01T00:00:00Z INFO ::_z_unicast_open_client] Sending Z_INIT(Syn)
[1970-01-01T00:00:00Z DEBUG ::_z_init_encode] Encoding _Z_MID_T_INIT
[1970-01-01T00:00:00Z DEBUG ::_z_init_decode] Decoding _Z_MID_T_INIT
[1970-01-01T00:00:00Z INFO ::_z_unicast_open_client] Received Z_INIT(Ack)
[1970-01-01T00:00:00Z INFO ::_z_unicast_open_client] Sending Z_OPEN(Syn)
[1970-01-01T00:00:00Z DEBUG ::_z_open_encode] Encoding _Z_MID_T_OPEN
[1970-01-01T00:00:00Z DEBUG ::_z_open_decode] Decoding _Z_MID_T_OPEN
[1970-01-01T00:00:00Z INFO ::_z_unicast_open_client] Received Z_OPEN(Ack)
OK
Declaring publisher for 'demo/example/zenoh-pico-pub'...[1970-01-01T00:00:00Z DEBUG ::_z_send_n_msg] >> send network message
[1970-01-01T00:00:00Z DEBUG ::_z_unicast_send_n_msg] >> send network message
[1970-01-01T00:00:00Z DEBUG ::_z_keyexpr_encode] Encoding _RESKEY
OK
[1970-01-01T00:00:00Z DEBUG ::_z_keyexpr_decode] Decoding _RESKEY
[1970-01-01T00:00:00Z DEBUG ::_z_keyexpr_decode] Decoding _RESKEY
[1970-01-01T00:00:00Z DEBUG ::_z_keyexpr_decode] Decoding _RESKEY
[1970-01-01T00:00:00Z INFO ::_z_unicast_handle_transport_message] Received Z_FRAME message
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:00Z DEBUG ::_z_handle_network_message] Handling _Z_N_DECLARE
[1970-01-01T00:00:02Z DEBUG ::_z_keep_alive_decode] Decoding _Z_MID_T_KEEP_ALIVE
[1970-01-01T00:00:02Z INFO ::_z_unicast_handle_transport_message] Received Z_KEEP_ALIVE message
␛[0;31mE (5426) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:␛[0m
␛[0;31mE (5426) task_wdt:  - IDLE (CPU 0)␛[0m
␛[0;31mE (5426) task_wdt: Tasks currently running:␛[0m
␛[0;31mE (5426) task_wdt: CPU 0: ␛[0m
␛[0;31mE (5426) task_wdt: CPU 1: IDLE␛[0m
␛[0;31mE (5426) task_wdt: Print CPU 0 (current core) backtrace␛[0m


Backtrace: 0x400F8552:0x3FFB06D0 0x40082721:0x3FFB06F0 0x40008541:0x3FFC9E30 0x400F3912:0x3FFC9E50 0x400D4FFD:0x3FFC9E70 0x400D509B:0x3FFC9E90 0x400DAA85:0x3FFC9EC0 0x400D6AED:0x3FFC9EF0 0x400D6668:0x3FFC9F10 0x400D545D:0x3FFC9F30 0x400D4F2D:0x3FFCA010 0x40089859:0x3FFCA030

␛[0;31mE (5426) task_wdt: Print CPU 1 backtrace␛[0m


Backtrace: 0x40084CE1:0x3FFB0CD0 0x40082721:0x3FFB0CF0 0x4000BFED:0x3FFB65A0 0x40089B0A:0x3FFB65B0 0x400F8817:0x3FFB65D0 0x400F8823:0x3FFB6600 0x400E0E29:0x3FFB6620 0x4008818D:0x3FFB6640 0x40089859:0x3FFB6660

[1970-01-01T00:00:05Z DEBUG ::_z_keep_alive_decode] Decoding _Z_MID_T_KEEP_ALIVE
[1970-01-01T00:00:05Z INFO ::_z_unicast_handle_transport_message] Received Z_KEEP_ALIVE message
 *  Terminal will be reused by tasks, press any key to close it. 
 ```

 Watchdog timeouts
 ```
 0x400f8552: task_wdt_isr at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/task_wdt.c line 186
0x400f3912: usleep at /home/lieven/.platformio/packages/framework-espidf/components/newlib/time.c line 212
0x400d4ffd: zp_sleep_us at lib/zenoh-pico/src/system/espidf/system.c line 119
0x400d509b: zp_sleep_ms at lib/zenoh-pico/src/system/espidf/system.c line 128
0x400daa85: _z_read_serial at lib/zenoh-pico/src/system/espidf/network.c line 637
0x400d6aed: _z_f_link_read_serial at lib/zenoh-pico/src/link/unicast/serial.c line 106
0x400d6668: _z_link_recv_zbuf at lib/zenoh-pico/src/link/link.c line 139
0x400d545d: _zp_unicast_read_task at lib/zenoh-pico/src/transport/unicast/read.c line 85
0x400d4f2d: z_task_wrapper at lib/zenoh-pico/src/system/espidf/system.c line 58
0x40089859: vPortTaskWrapper at /home/lieven/.platformio/packages/framework-espidf/components/freertos/port/xtensa/port.c line 142

```

```
Decoding stack results
0x40084ce1: esp_crosscore_isr at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/crosscore_int.c line 92
0x40089b0a: vPortExitCritical at /home/lieven/.platformio/packages/framework-espidf/components/freertos/port/xtensa/include/freertos/portmacro.h line 571
0x400f8817: idle_hook_cb at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/task_wdt.c line 81
0x400f8823: spi_flash_hal_init at /home/lieven/.platformio/packages/framework-espidf/components/soc/include/soc/soc_memory_types.h line 92
0x400e0e29: esp_vApplicationIdleHook at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/freertos_hooks.c line 51
0x4008818d: prvIdleTask at /home/lieven/.platformio/packages/framework-espidf/components/freertos/tasks.c line 3987
0x40089859: vPortTaskWrapper at /home/lieven/.platformio/packages/framework-espidf/components/freertos/port/xtensa/port.c line 142
```


```log
[298320]Opening Zenoh Session ... session_ok= 5 sessions_fail=0 publish_ok=5000  
session open 
publisher declared
Declaring Subscriber on 'dst/esp32/sys/**'...
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32032] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32033] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32034] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32035] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32036] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32037] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32038] Pub from Rust!)
Closing sessions_ok= 5 sessions_fail=0 publish_ok=6000  
Closing serial 2
Heap before: 298320, Heap after: 298288 delta 32 
[298288]Opening Zenoh Session ... session_ok= 6 sessions_fail=0 publish_ok=6000  
session open 
publisher declared
Declaring Subscriber on 'dst/esp32/sys/**'...
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32040] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32041] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32042] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32043] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32044] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32045] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32046] Pub from Rust!)
Closing sessions_ok= 6 sessions_fail=0 publish_ok=7000  
Closing serial 2
Heap before: 298288, Heap after: 298256 delta 32 
[298256]Opening Zenoh Session ... session_ok= 7 sessions_fail=0 publish_ok=7000  
session open 
publisher declared
Declaring Subscriber on 'dst/esp32/sys/**'...
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32048] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32049] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32050] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32051] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32052] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32053] Pub from Rust!)
 >>>>>>>>>> [Subscription listener] Received (dst/esp32/sys/message, [32054] Pub from Rust!)
Closing sessions_ok= 7 sessions_fail=0 publish_ok=8000  
```
