# spinlock_acquire spinlock.h:140 (lock->count == 0)
## Solution

Solved by setting IPC stack size higher to 1500

### log

```
assert failed: spinlock_acquire spinlock.h:140 (lock->count == 0)


Backtrace: 0x4008257a:0x3ffc87e0 0x4008969d:0x3ffc8800 0x40091562:0x3ffc8820 0x4008a746:0x3ffc8940 0x4008a030:0x3ffc8970 0x4010d8f5:0x3ffc89b0 0x4010da93:0x3ffc89d0 0x400dfec5:0x3ffc89f0 0x400e0108:0x3ffc8a10 0x400e0633:0x3ffc8a30 0x400e066d:0x3ffc8a50 0x400dae6d:0x3ffc8a70 0x400d9499:0x3ffc8c70 0x400d7d99:0x3ffc8d20 0x400d8984:0x3ffc8da0 0x400d8c84:0x3ffc8e50 0x400d41e9:0x3ffc8eb0 0x4008a4c1:0x3ffc8ed0
  #0  0x4008257a in panic_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/panic.c:463
  #1  0x4008969d in esp_system_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/port/esp_system_chip.c:92
  #2  0x40091562 in __assert_func at /home/lieven/.platformio/packages/framework-espidf/components/newlib/assert.c:80
  #3  0x4008a746 in spinlock_acquire at /home/lieven/.platformio/packages/framework-espidf/components/esp_hw_support/include/spinlock.h:140 (discriminator 1)
      (inlined by) xPortEnterCriticalTimeout at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:473 (discriminator 1)
  #4  0x4008a030 in vPortEnterCritical at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:567
      (inlined by) xQueueSemaphoreTake at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1727
  #5  0x4010d8f5 in pthread_mutex_lock_internal at /home/lieven/.platformio/packages/framework-espidf/components/pthread/pthread.c:697
  #6  0x4010da93 in pthread_mutex_lock at /home/lieven/.platformio/packages/framework-espidf/components/pthread/pthread.c:727
  #7  0x400dfec5 in _z_mutex_lock at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/system.c:139
  #8  0x400e0108 in _z_transport_tx_mutex_lock at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/include/zenoh-pico/transport/transport.h:191
  #9  0x400e0633 in _z_transport_tx_send_n_msg at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/transport/common/tx.c:222
  #10 0x400e066d in _z_send_n_msg at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/transport/common/tx.c:404
  #11 0x400dae6d in _z_write at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/net/primitives.c:240
  #12 0x400d9499 in z_put at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/api/api.c:883 (discriminator 4)
  #13 0x400d7d99 in ZenohActor::zenoh_publish_binary(char const*, std::vector<unsigned char, std::allocator<unsigned char> > const&) at src/zenoh_actor.cpp:308 (discriminator 3)
  #14 0x400d8984 in ZenohActor::on_cmd(ZenohCmd&) at src/zenoh_actor.cpp:94
  #15 0x400d8c84 in ZenohActor::run() at src/zenoh_actor.cpp:107
  #16 0x400d41e9 in app_main::{lambda(void*)#2}::_FUN(void*) at src/main.cpp:105
      (inlined by) _FUN at src/main.cpp:106
  #17 0x4008a4c1 in vPortTaskWrapper at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134




ELF file SHA256: 070cb87da

Rebooting...
ets Jun  8 2016 00:22:57
```
