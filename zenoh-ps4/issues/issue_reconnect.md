# assert failed: xQueueSemaphoreTake queue.c:1713 (pxQueue->uxItemSize == 0)
## scenario

Zenoh router drops and ESP32 tries to reconnect

## log
```
I 00:00:17.327 |        main.cpp: 184 |  free heap size: 35684
I 00:00:18.336 |        main.cpp: 184 |  free heap size: 37688
I 00:00:19.073 | zenoh_actor.cpp: 257 | Failed to publish message
I 00:00:19.081 | zenoh_actor.cpp: 257 | =-100 src/zenoh_actor.cpp:257
I 00:00:19.085 | zenoh_actor.cpp:  83 | Failed to publish message, disconnect and reconnect
I 00:00:19.089 | zenoh_actor.cpp: 119 | Closing Zenoh Session...
I 00:00:19.345 |        main.cpp: 184 |  free heap size: 43736
I 00:00:20.349 |        main.cpp: 184 |  free heap size: 43648

assert failed: xQueueSemaphoreTake queue.c:1713 (pxQueue->uxItemSize == 0)


Backtrace: 0x40082a5e:0x3ffe1000 0x40090145:0x3ffe1020 0x40095e3a:0x3ffe1040 0x401b969d:0x3ffe1160 0x401279f1:0x3ffe11a0 0x40127b93:0x3ffe11c0 0x400e1a99:0x3ffe11e0 0x400e158e:0x3ffe1200 0x400ddc4f:0x3ffe1220 0x400dc5d6:0x3ffe1380 0x400dc5eb:0x3ffe13a0 0x400db935:0x3ffe13c0 0x400dbe7b:0x3ffe13e0 0x400d6068:0x3ffe14a0 0x400d6101:0x3ffe14e0 0x400904f2:0x3ffe1500
  #0  0x40082a5e in panic_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/panic.c:463
  #1  0x40090145 in esp_system_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/port/esp_system_chip.c:92
  #2  0x40095e3a in __assert_func at /home/lieven/.platformio/packages/framework-espidf/components/newlib/assert.c:80
  #3  0x401b969d in xQueueSemaphoreTake at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1713 (discriminator 1)
  #4  0x401279f1 in pthread_mutex_lock_internal at /home/lieven/.platformio/packages/framework-espidf/components/pthread/pthread.c:697
  #5  0x40127b93 in pthread_mutex_lock at /home/lieven/.platformio/packages/framework-espidf/components/pthread/pthread.c:727
  #6  0x400e1a99 in _z_mutex_lock at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/system.c:139
  #7  0x400e158e in _z_session_mutex_lock at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/include/zenoh-pico/session/utils.h:40
      (inlined by) _z_get_subscription_by_id at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/session/subscription.c:124
  #8  0x400ddc4f in _z_undeclare_subscriber at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/net/primitives.c:292
  #9  0x400dc5d6 in _z_subscriber_drop at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/api/api.c:1404
  #10 0x400dc5eb in z_subscriber_drop at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/api/api.c:1408 (discriminator 1)
  #11 0x400db935 in z_drop(z_moved_subscriber_t*) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/include/zenoh-pico/api/macros.h:462
      (inlined by) ZenohActor::disconnect() at src/zenoh_actor.cpp:131
  #12 0x400dbe7b in ZenohActor::on_cmd(ZenohCmd&) at src/zenoh_actor.cpp:84
  #13 0x400d6068 in Actor<ZenohEvent, ZenohCmd>::loop() at src/actor.h:244
  #14 0x400d6101 in Actor<ZenohEvent, ZenohCmd>::start()::{lambda(void*)#1}::operator()(void*) const at src/actor.h:230
      (inlined by) Actor<ZenohEvent, ZenohCmd>::start()::{lambda(void*)#1}::_FUN(void*) at src/actor.h:227
  #15 0x400904f2 in vPortTaskWrapper at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134





ELF file SHA256: e98d311b6

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:7176
load:0x40078000,len:15564
ho 0 tail 12 room 4
load:0x40080400,len:4
load:0x40080404,len:3904
entry 0x40080638
I (31) boot: ESP-IDF 5.3.0 2nd stage bootloader
I (31) boot: compile time Jan 11 2025 16:12:53
I (31) boot: Multicore bootloader
I (35) boot: chip revision: v0.0
I (39) boot.esp32: SPI Speed      : 40MHz
I (44) boot.esp32: SPI Mode       : DIO
I (48) boot.esp32: SPI Flash Size : 2MB
I (53) boot: Enabling RNG early entropy source...
```