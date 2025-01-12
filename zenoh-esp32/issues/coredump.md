```

lieven@pcpro:~/workspace/zenoh-projects/zenoh-esp32$ espcoredump.py info_corefile  -t raw  .pio/build/az-delivery-devkit-v4/firmware.elf
INFO: Invoke esptool to read image.
INFO: Retrieving core dump partition offset and size...
INFO: Core dump partition offset=1572864, size=131072
WARNING: The core dump image offset is not specified. Use partition offset: 0x180000.
INFO: esptool.py v4.7.0
Found 33 serial ports
Serial port /dev/ttyUSB1
Connecting....
Detecting chip type... Unsupported detection protocol, switching and trying again...
Connecting....
Detecting chip type... ESP32
Chip is ESP32-D0WDQ6 (revision v0.0)
Features: WiFi, BT, Dual Core, Coding Scheme None
Crystal is 40MHz
MAC: 30:ae:a4:03:ec:98
Uploading stub...
Running stub...
Stub running...
16 (100 %)
16 (100 %)
Read 16 bytes at 0x00180000 in 0.0 seconds (12.5 kbit/s)...
Hard resetting via RTS pin...

INFO: esptool.py v4.7.0
Found 33 serial ports
Serial port /dev/ttyUSB1
Connecting......
Detecting chip type... Unsupported detection protocol, switching and trying again...
Connecting..........
Detecting chip type... ESP32
Chip is ESP32-D0WDQ6 (revision v0.0)
Features: WiFi, BT, Dual Core, Coding Scheme None
Crystal is 40MHz
MAC: 30:ae:a4:03:ec:98
Uploading stub...
Running stub...
Stub running...
28132 (100 %)
28132 (100 %)
Read 28132 bytes at 0x00180000 in 2.5 seconds (88.9 kbit/s)...
Hard resetting via RTS pin...

===============================================================
==================== ESP32 CORE DUMP START ====================

Crashed task handle: 0x3ffe2a4c, name: 'wifi', GDB name: 'process 1073621580'
Crashed task is not in the interrupt context

================== CURRENT THREAD REGISTERS ===================
exccause       0xffff (InvalidCauseRegister)
excvaddr       0x0
pc             0x20000000          0x20000000
lbeg           0x4000c2e0          1073791712
lend           0x4000c2f6          1073791734
lcount         0xffffffff          4294967295
sar            0x17                23
ps             0x30                48
threadptr      <unavailable>
br             <unavailable>
scompare1      <unavailable>
acclo          <unavailable>
acchi          <unavailable>
m0             <unavailable>
m1             <unavailable>
m2             <unavailable>
m3             <unavailable>
expstate       <unavailable>
f64r_lo        <unavailable>
f64r_hi        <unavailable>
f64s           <unavailable>
fcr            <unavailable>
fsr            <unavailable>
a0             0x20000070          536871024
a1             0x0                 0
a2             0x0                 0
a3             0x0                 0
a4             0x60c20             396320
a5             0x60c23             396323
a6             0x4d7fc             317436
a7             0x0                 0
a8             0x3ff000dc          1072693468
a9             0x1                 1
a10            0x3ffbde68          1073471080
a11            0xffffffff          -1
a12            0x3fff6ff0          1073704944
a13            0x60c23             396323
a14            0x1                 1
a15            0xcdcd              52685

==================== CURRENT THREAD STACK =====================
#0  0x20000000 in ?? ()

======================== THREADS INFO =========================
  Id   Target Id          Frame 
* 1    process 1073621580 0x20000000 in ?? ()
  2    process 1073461856 0x4000bff0 in _xtos_set_intlevel ()
  3    process 1073464720 0x4008f086 in esp_cpu_wait_for_intr () at /home/lieven/.platformio/packages/framework-espidf/components/esp_hw_support/cpu.c:64
  4    process 1073466860 0x4008f086 in esp_cpu_wait_for_intr () at /home/lieven/.platformio/packages/framework-espidf/components/esp_hw_support/cpu.c:64
  5    process 1073469084 0x4000bff0 in _xtos_set_intlevel ()
  6    process 1073462580 0x40081d4c in esp_crosscore_int_send_yield (core_id=0) at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/crosscore_int.c:121
  7    process 1073596664 0x4000bff0 in _xtos_set_intlevel ()
  8    process 1073610212 0x4000bff0 in _xtos_set_intlevel ()
  9    process 1073601012 0x4000bff0 in _xtos_set_intlevel ()
  10   process 1073710408 vTaskDelay (xTicksToDelay=3333) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:1592
  11   process 1073619104 0x4000bff0 in _xtos_set_intlevel ()
  12   process 1073626024 0x4000bff0 in _xtos_set_intlevel ()
  13   process 1073648276 0x4000bff0 in _xtos_set_intlevel ()
  14   process 1073443608 0x4000bff0 in _xtos_set_intlevel ()
  15   process 1073605768 0x4000bff0 in _xtos_set_intlevel ()
  16   process 1073445456 0x4000bff0 in _xtos_set_intlevel ()
  17   process 1073704940 0x4000bff0 in _xtos_set_intlevel ()
  18   process 1073669656 0x4000bff0 in _xtos_set_intlevel ()


       TCB             NAME PRIO C/B  STACK USED/FREE
---------- ---------------- -------- ----------------
0x3ffe2a4c             wifi    23/23        1184/5460
0x3ffbba60        esp_timer    22/22         688/6812
0x3ffbc590            IDLE0      0/0         416/1364
0x3ffbcdec            IDLE1      0/0         416/1368
0x3ffbd69c             wifi      5/5         528/3552
0x3ffbbd34             main      1/1         576/6924
0x3ffdc8f8              tiT    18/18         560/3020
0x3ffdfde4              sys      5/5         528/3552
0x3ffdd9f4            zenoh      5/5         528/3456
0x3fff8548                     12/12         448/4668
0x3ffe20a0              ps4      5/5         608/5500
0x3ffe3ba8              led      5/5         528/3564
0x3ffe9294    btstack_stdio    12/12         480/1552
0x3ffb7318             ipc0      5/5         432/1064
0x3ffdec88          sys_evt    20/20         544/3084
0x3ffb7a50             ipc1    24/24         432/1056
0x3fff6fec                     18/12        1136/3976
0x3ffee618     btController    23/23         512/3580

==================== THREAD 1 (TCB: 0x3ffe2a4c, name: 'wifi') =====================
#0  0x20000000 in ?? ()

==================== THREAD 2 (TCB: 0x3ffbba60, name: 'esp_timer') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffe23a8) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf23a in xQueueGenericSend (xQueue=0x3ffe235c, pvItemToQueue=0x3ffbb8f0 <rwip_heap_env+196>, xTicksToWait=<optimized out>, xCopyPosition=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1056
#4  0x401be1a7 in queue_send_wrapper (queue=0x3ffe235c, item=0x3ffbb8f0 <rwip_heap_env+196>, block_time_tick=10) at /home/lieven/.platformio/packages/framework-espidf/components/esp_wifi/esp32/esp_adapter.c:301
#5  0x4008583c in pp_post ()
#6  0x40166d67 in ieee80211_timer_process ()
#7  0x4016f936 in pp_timer_process ()
#8  0x4016cfb8 in ?? ()
#9  0x4019f1d5 in timer_process_alarm (dispatch_method=ESP_TIMER_TASK) at /home/lieven/.platformio/packages/framework-espidf/components/esp_timer/src/esp_timer.c:456
#10 0x4019f224 in timer_task (arg=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/esp_timer/src/esp_timer.c:482
#11 0x400907f9 in vPortTaskWrapper (pxCode=0x4019f210 <timer_task>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 3 (TCB: 0x3ffbc590, name: 'IDLE0') =====================
#0  0x4008f086 in esp_cpu_wait_for_intr () at /home/lieven/.platformio/packages/framework-espidf/components/esp_hw_support/cpu.c:64
#1  0x4010a09d in esp_vApplicationIdleHook () at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/freertos_hooks.c:58
#2  0x401bfb34 in prvIdleTask (pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:4344
#3  0x400907f9 in vPortTaskWrapper (pxCode=0x401bfb28 <prvIdleTask>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 4 (TCB: 0x3ffbcdec, name: 'IDLE1') =====================
#0  0x4008f086 in esp_cpu_wait_for_intr () at /home/lieven/.platformio/packages/framework-espidf/components/esp_hw_support/cpu.c:64
#1  0x4010a09d in esp_vApplicationIdleHook () at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/freertos_hooks.c:58
#2  0x401bfb34 in prvIdleTask (pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:4344
#3  0x400907f9 in vPortTaskWrapper (pxCode=0x401bfb28 <prvIdleTask>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 5 (TCB: 0x3ffbd69c, name: 'wifi') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffb7bf4) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffb7ba8, pvBuffer=0x3ffdba0c, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x400d69d5 in Channel<WifiCmd*>::receive (timeout=<optimized out>, message=0x3ffdba0c, this=0x3ffc4318 <wifi_actor+16>) at src/actor.h:33
#5  Actor<WifiEvent, WifiCmd>::loop (this=0x3ffc4308 <wifi_actor>) at src/actor.h:294
#6  0x400d6a80 in Actor<WifiEvent, WifiCmd>::start()::{lambda(void*)#1}::operator()(void*) const (arg=0x3ffc4308 <wifi_actor>, __closure=0x0) at src/actor.h:279
#7  Actor<WifiEvent, WifiCmd>::start()::{lambda(void*)#1}::_FUN(void*) () at src/actor.h:279
#8  0x400907f9 in vPortTaskWrapper (pxCode=0x400d6a78 <Actor<WifiEvent, WifiCmd>::start()::{lambda(void*)#1}::_FUN(void*)>, pvParameters=0x3ffc4308 <wifi_actor>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 6 (TCB: 0x3ffbbd34, name: 'main') =====================
#0  0x40081d4c in esp_crosscore_int_send_yield (core_id=0) at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/crosscore_int.c:121
#1  0x401bfecd in vTaskDelay (xTicksToDelay=1000) at /home/lieven/.platformio/packages/framework-espidf/components/xtensa/include/xt_utils.h:41
#2  0x400d7d19 in app_main () at src/main.cpp:178
#3  0x401be521 in main_task (args=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/app_startup.c:208
#4  0x400907f9 in vPortTaskWrapper (pxCode=0x401be458 <main_task>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 7 (TCB: 0x3ffdc8f8, name: 'tiT') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffbda58 <rwip_heap_env+8748>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffbda0c <rwip_heap_env+8672>, pvBuffer=0x3ffdc848, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x401241f8 in sys_arch_mbox_fetch (mbox=<optimized out>, msg=0x3ffdc848, timeout=187) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/port/freertos/sys_arch.c:317
#5  0x40112863 in tcpip_timeouts_mbox_fetch (mbox=0x3ffd33c4 <tcpip_mbox>, msg=0x3ffdc848) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/tcpip.c:104
#6  0x40112931 in tcpip_thread (arg=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/tcpip.c:142
#7  0x400907f9 in vPortTaskWrapper (pxCode=0x40112904 <tcpip_thread>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 8 (TCB: 0x3ffdfde4, name: 'sys') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffb7d50) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffb7d04, pvBuffer=0x3ffdfcfc, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x400d6bdd in Channel<SysCmd*>::receive (timeout=<optimized out>, message=0x3ffdfcfc, this=0x3ffc3ef0 <sys_actor+16>) at src/actor.h:33
#5  Actor<SysEvent, SysCmd>::loop (this=0x3ffc3ee0 <sys_actor>) at src/actor.h:294
#6  0x400d6c88 in Actor<SysEvent, SysCmd>::start()::{lambda(void*)#1}::operator()(void*) const (arg=0x3ffc3ee0 <sys_actor>, __closure=0x0) at src/actor.h:279
#7  Actor<SysEvent, SysCmd>::start()::{lambda(void*)#1}::_FUN(void*) () at src/actor.h:279
#8  0x400907f9 in vPortTaskWrapper (pxCode=0x400d6c80 <Actor<SysEvent, SysCmd>::start()::{lambda(void*)#1}::_FUN(void*)>, pvParameters=0x3ffc3ee0 <sys_actor>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 9 (TCB: 0x3ffdd9f4, name: 'zenoh') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffb7cbc) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffb7c70, pvBuffer=0x3ffdd90c, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x400d6ad9 in Channel<ZenohCmd*>::receive (timeout=<optimized out>, message=0x3ffdd90c, this=0x3ffc4190 <zenoh_actor+16>) at src/actor.h:33
#5  Actor<ZenohEvent, ZenohCmd>::loop (this=0x3ffc4180 <zenoh_actor>) at src/actor.h:294
#6  0x400d6b84 in Actor<ZenohEvent, ZenohCmd>::start()::{lambda(void*)#1}::operator()(void*) const (arg=0x3ffc4180 <zenoh_actor>, __closure=0x0) at src/actor.h:279
#7  Actor<ZenohEvent, ZenohCmd>::start()::{lambda(void*)#1}::_FUN(void*) () at src/actor.h:279
#8  0x400907f9 in vPortTaskWrapper (pxCode=0x400d6b7c <Actor<ZenohEvent, ZenohCmd>::start()::{lambda(void*)#1}::_FUN(void*)>, pvParameters=0x3ffc4180 <zenoh_actor>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 10 (TCB: 0x3fff8548, name: '') =====================
#0  vTaskDelay (xTicksToDelay=3333) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:1592
#1  0x400e2ca0 in z_sleep_ms (time=<optimized out>) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/system.c:179
#2  0x400e451f in _zp_unicast_lease_task (ztu_arg=0x3fff3c04) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/transport/unicast/lease.c:110
#3  0x400e2b36 in z_task_wrapper (arg=0x3fff3e80) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/system.c:61
#4  0x400907f9 in vPortTaskWrapper (pxCode=0x400e2b2c <z_task_wrapper>, pvParameters=0x3fff3e80) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 11 (TCB: 0x3ffe20a0, name: 'ps4') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffbe118 <rwip_heap_non_ret+1468>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401c0c29 in xTaskGenericNotifyWait (uxIndexToWait=0, ulBitsToClearOnEntry=<optimized out>, ulBitsToClearOnExit=4294967295, pulNotificationValue=0x0, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:5841
#4  0x400f51b8 in btstack_run_loop_freertos_execute () at /home/lieven/esp/esp-idf/components/btstack/platform/freertos/btstack_run_loop_freertos.c:202
#5  0x400ec852 in btstack_run_loop_execute () at /home/lieven/esp/esp-idf/components/btstack/src/btstack_run_loop.c:310
#6  0x400d8a3b in Ps4Actor::on_start (this=0x3ffc3e24 <ps4_actor>) at src/ps4_actor.cpp:37
#7  0x400d6cc8 in Actor<Ps4Event, Ps4Cmd>::loop (this=0x3ffc3e24 <ps4_actor>) at src/actor.h:290
#8  0x400d6d90 in Actor<Ps4Event, Ps4Cmd>::start()::{lambda(void*)#1}::operator()(void*) const (arg=0x3ffc3e24 <ps4_actor>, __closure=0x0) at src/actor.h:279
#9  Actor<Ps4Event, Ps4Cmd>::start()::{lambda(void*)#1}::_FUN(void*) () at src/actor.h:279
#10 0x400907f9 in vPortTaskWrapper (pxCode=0x400d6d88 <Actor<Ps4Event, Ps4Cmd>::start()::{lambda(void*)#1}::_FUN(void*)>, pvParameters=0x3ffc3e24 <ps4_actor>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 12 (TCB: 0x3ffe3ba8, name: 'led') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffb7e98) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffb7e4c, pvBuffer=0x3ffe3acc, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x400d6de9 in Channel<LedCmd*>::receive (timeout=<optimized out>, message=0x3ffe3acc, this=0x3ffc3df0 <led_actor+16>) at src/actor.h:33
#5  Actor<LedEvent, LedCmd>::loop (this=0x3ffc3de0 <led_actor>) at src/actor.h:294
#6  0x400d6e88 in Actor<LedEvent, LedCmd>::start()::{lambda(void*)#1}::operator()(void*) const (arg=0x3ffc3de0 <led_actor>, __closure=0x0) at src/actor.h:279
#7  Actor<LedEvent, LedCmd>::start()::{lambda(void*)#1}::_FUN(void*) () at src/actor.h:279
#8  0x400907f9 in vPortTaskWrapper (pxCode=0x400d6e80 <Actor<LedEvent, LedCmd>::start()::{lambda(void*)#1}::_FUN(void*)>, pvParameters=0x3ffc3de0 <led_actor>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 13 (TCB: 0x3ffe9294, name: 'btstack_stdio') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffe3eac) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf449 in xQueuePeek (xQueue=0x3ffe3e60, pvBuffer=0x3ffe91d0, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:2084
#4  0x400ed1d3 in btstack_stdio_task (arg=0x0) at /home/lieven/esp/esp-idf/components/btstack/btstack_stdio_esp32.c:141
#5  0x400907f9 in vPortTaskWrapper (pxCode=0x400ed1b0 <btstack_stdio_task>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 14 (TCB: 0x3ffb7318, name: 'ipc0') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffbe118 <rwip_heap_non_ret+1468>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bfd90 in ulTaskGenericNotifyTake (uxIndexToWait=0, xClearCountOnExit=1, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:5756
#4  0x40081da0 in ipc_task (arg=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/esp_ipc.c:62
#5  0x400907f9 in vPortTaskWrapper (pxCode=0x40081d70 <ipc_task>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 15 (TCB: 0x3ffdec88, name: 'sys_evt') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffddbf0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffddba4, pvBuffer=0x3ffdebac, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x401bba50 in esp_event_loop_run (event_loop=0x3ffbdae4 <rwip_heap_env+8888>, ticks_to_run=4294967295) at /home/lieven/.platformio/packages/framework-espidf/components/esp_event/esp_event.c:560
#5  0x401bba7c in esp_event_loop_run_task (args=0x3ffbdae4 <rwip_heap_env+8888>) at /home/lieven/.platformio/packages/framework-espidf/components/esp_event/esp_event.c:105
#6  0x400907f9 in vPortTaskWrapper (pxCode=0x401bba70 <esp_event_loop_run_task>, pvParameters=0x3ffbdae4 <rwip_heap_env+8888>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 16 (TCB: 0x3ffb7a50, name: 'ipc1') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffbe118 <rwip_heap_non_ret+1468>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bfd90 in ulTaskGenericNotifyTake (uxIndexToWait=0, xClearCountOnExit=1, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/tasks.c:5756
#4  0x40081da0 in ipc_task (arg=0x1) at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/esp_ipc.c:62
#5  0x400907f9 in vPortTaskWrapper (pxCode=0x40081d70 <ipc_task>, pvParameters=0x1) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 17 (TCB: 0x3fff6fec, name: '') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3fff3fc8) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3fff3f7c, pvBuffer=0x3fff6cd8, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x401241d5 in sys_arch_mbox_fetch (mbox=<optimized out>, msg=0x3fff6cd8, timeout=0) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/port/freertos/sys_arch.c:313
#5  0x401257aa in netconn_recv_data (conn=0x3fff3f34, new_buf=0x3fff6d88, apiflags=8 '\b') at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/api_lib.c:615
#6  0x401258e1 in netconn_recv_data_tcp (conn=0x3fff3f34, new_buf=0x3fff6d88, apiflags=8 '\b') at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/api_lib.c:727
#7  0x4012598c in netconn_recv_tcp_pbuf_flags (conn=0x3fff3f34, new_buf=0x3fff6d88, apiflags=8 '\b') at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/api_lib.c:808
#8  0x40110f38 in lwip_recv_tcp (sock=0x3ffd316c <sockets>, mem=0x3fff4d6f, len=<optimized out>, flags=0) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/sockets.c:951
#9  0x40111dd3 in lwip_recvfrom (s=54, mem=0x3fff4d6f, len=1673, flags=0, from=0x0, fromlen=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/sockets.c:1218
#10 0x40111ebd in lwip_recv (s=54, mem=0x3fff4d6f, len=1673, flags=0) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/lwip/src/api/sockets.c:1283
#11 0x400ea06d in recv (flags=0, len=1673, mem=0x3fff4d6f, s=54) at /home/lieven/.platformio/packages/framework-espidf/components/lwip/include/lwip/sockets.h:38
#12 _z_read_tcp (sock=..., ptr=0x3fff4d6f <error: Cannot access memory at address 0x3fff4d6f>, len=1673) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/network.c:120
#13 0x400e60f4 in _z_f_link_read_tcp (zl=0x3fff3c08, ptr=0x3fff4d6f <error: Cannot access memory at address 0x3fff4d6f>, len=1673, addr=0x0) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/link/unicast/tcp.c:150
#14 0x400e5bf4 in _z_link_recv_zbuf (link=0x3fff3c08, zbf=0x3fff3c88, addr=0x0) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/link/link.c:138
#15 0x400e45f1 in _zp_unicast_read_task (ztu_arg=0x3fff3c04) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/transport/unicast/read.c:67
#16 0x400e2b36 in z_task_wrapper (arg=0x3fff3e38) at .pio/libdeps/az-delivery-devkit-v4/zenoh-pico/src/system/espidf/system.c:61
#17 0x400907f9 in vPortTaskWrapper (pxCode=0x400e2b2c <z_task_wrapper>, pvParameters=0x3fff3e38) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134

==================== THREAD 18 (TCB: 0x3ffee618, name: 'btController') =====================
#0  0x4000bff0 in _xtos_set_intlevel ()
#1  0x40090a1e in vPortClearInterruptMaskFromISR (prev_level=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/portmacro.h:560
#2  vPortExitCritical (mux=0x3ffed46c) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:505
#3  0x401bf58d in xQueueReceive (xQueue=0x3ffed420, pvBuffer=0x3ffee560, xTicksToWait=<optimized out>) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/queue.c:1632
#4  0x40188cd8 in queue_recv_hlevel_wrapper (queue=0x3ffb6a98, item=0x3ffee560, block_time_ms=4294967295) at /home/lieven/.platformio/packages/framework-espidf/components/bt/controller/esp32/bt.c:781
#5  0x4017c3d2 in btdm_controller_task ()
#6  0x400907f9 in vPortTaskWrapper (pxCode=0x4017c3ac <btdm_controller_task>, pvParameters=0x0) at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134


======================= ALL MEMORY REGIONS ========================
Name   Address   Size   Attrs
.rtc.text 0x400c0000 0x0 RW  
.rtc.dummy 0x3ff80000 0x0 RW  
.rtc.force_fast 0x3ff80000 0x0 RW  
.rtc_noinit 0x50000000 0x0 RW  
.rtc.force_slow 0x50000000 0x0 RW  
.rtc_fast_reserved 0x3ff82000 0x0 RW  
.iram0.vectors 0x40080000 0x403 R XA
.iram0.text 0x40080404 0x1c83f R XA
.dram0.data 0x3ffbdb60 0x6274 RW A
.ext_ram_noinit 0x3f800000 0x0 RW  
.ext_ram.bss 0x3f800000 0x0 RW  
.flash.appdesc 0x3f400020 0x100 R  A
.flash.rodata 0x3f400120 0x339d8 RW A
.flash.text 0x400d0020 0xf31e6 R XA
.iram0.data 0x4009cc44 0x0 RW  
.iram0.bss 0x4009cc44 0x0 RW  
.dram0.heap_start 0x3ffd8ab0 0x0 RW  
.coredump.tasks.data 0x3ffe2a4c 0x154 RW 
.coredump.tasks.data 0x20000000 0x70 RW 
.coredump.tasks.data 0x3ffbba60 0x154 RW 
.coredump.tasks.data 0x3ffbb7a0 0x2b0 RW 
.coredump.tasks.data 0x3ffbc590 0x154 RW 
.coredump.tasks.data 0x3ffbc3e0 0x1a0 RW 
.coredump.tasks.data 0x3ffbcdec 0x154 RW 
.coredump.tasks.data 0x3ffbcc40 0x1a0 RW 
.coredump.tasks.data 0x3ffbd69c 0x154 RW 
.coredump.tasks.data 0x3ffdb8d0 0x210 RW 
.coredump.tasks.data 0x3ffbbd34 0x154 RW 
.coredump.tasks.data 0x3ffda8a0 0x240 RW 
.coredump.tasks.data 0x3ffdc8f8 0x154 RW 
.coredump.tasks.data 0x3ffdc6c0 0x230 RW 
.coredump.tasks.data 0x3ffdfde4 0x154 RW 
.coredump.tasks.data 0x3ffdfbc0 0x210 RW 
.coredump.tasks.data 0x3ffdd9f4 0x154 RW 
.coredump.tasks.data 0x3ffdd7d0 0x210 RW 
.coredump.tasks.data 0x3fff8548 0x154 RW 
.coredump.tasks.data 0x3fff8380 0x1c0 RW 
.coredump.tasks.data 0x3ffe20a0 0x154 RW 
.coredump.tasks.data 0x3ffe1e30 0x260 RW 
.coredump.tasks.data 0x3ffe3ba8 0x154 RW 
.coredump.tasks.data 0x3ffe3990 0x210 RW 
.coredump.tasks.data 0x3ffe9294 0x154 RW 
.coredump.tasks.data 0x3ffe90a0 0x1e0 RW 
.coredump.tasks.data 0x3ffb7318 0x154 RW 
.coredump.tasks.data 0x3ffb7160 0x1b0 RW 
.coredump.tasks.data 0x3ffdec88 0x154 RW 
.coredump.tasks.data 0x3ffdea60 0x220 RW 
.coredump.tasks.data 0x3ffb7a50 0x154 RW 
.coredump.tasks.data 0x3ffb7890 0x1b0 RW 
.coredump.tasks.data 0x3fff6fec 0x154 RW 
.coredump.tasks.data 0x3fff6b70 0x470 RW 
.coredump.tasks.data 0x3ffee618 0x154 RW 
.coredump.tasks.data 0x3ffee410 0x200 RW 

===================== ESP32 CORE DUMP END =====================
===============================================================
Done!
lieven@pcpro:~/workspace/zenoh-projects/zenoh-esp32$ 
```