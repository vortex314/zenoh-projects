I 00:09:09.357 |        main.cpp: 189 |  free heap size: 27028
I 00:09:09.965 |   ps4_actor.cpp: 133 | custom: device connected: 0x3ffd0dbc
  #0  0x3ffd0dbc in ?? at /home/lieven/workspace/bluepad32/src/components/bluepad32/uni_hid_device.c:55

I 00:09:09.972 |        main.cpp: 135 | DEVICE_CONNECTED
I 00:09:09.977 |   ps4_actor.cpp: 147 | custom: device ready: 0x3ffd0dbc
  #0  0x3ffd0dbc in ?? at /home/lieven/workspace/bluepad32/src/components/bluepad32/uni_hid_device.c:55

I 00:09:09.980 |        main.cpp: 141 | DEVICE_READY
I 00:09:10.014 |   ps4_actor.cpp: 133 | custom: device connected: 0x3ffd0dbc
  #0  0x3ffd0dbc in ?? at /home/lieven/workspace/bluepad32/src/components/bluepad32/uni_hid_device.c:55

I 00:09:10.020 |        main.cpp: 135 | DEVICE_CONNECTED
I 00:09:10.024 |   ps4_actor.cpp: 147 | custom: device ready:
assert failed: btstack_run_loop_base_add_timer btstack_run_loop.c:112 (0)


Backtrace: 0x40082a6e:0x3ffdeb50 0x400903fd:0x3ffdeb70 0x4009613e:0x3ffdeb90 0x400ec557:0x3ffdecb0 0x400ec7e3:0x3ffdecd0 0x400fa117:0x3ffdecf0 0x400fa529:0x3ffded70 0x401b3d01:0x3ffded90 0x400d8cf2:0x3ffdedb0 0x400f79ed:0x3ffdee90 0x400fa1ac:0x3ffdeeb0 0x400f7a4d:0x3ffdeed0 0x400f7f96:0x3ffdeef0 0x400f81c1:0x3ffdef10 0x400f5b7d:0x3ffdef40 0x401b8fc2:0x3ffdef60 0x40196996:0x3ffdef80 0x40199945:0x3ffdefe0 0x40199b19:0x3ffdf000 0x40199c14:0x3ffdf040 0x4019a255:0x3ffdf060 0x400ee3a5:0x3ffdf080 0x400f3e69:0x3ffdf0a0 0x400f4023:0x3ffdf0e0 0x400ecffa:0x3ffdf100 0x400f514d:0x3ffdf130 0x400ec84f:0x3ffdf160 0x400d8a38:0x3ffdf180 0x400d6cc5:0x3ffdf1a0 0x400d6d8d:0x3ffdf1e0 0x400907f6:0x3ffdf200
  #0  0x40082a6e in panic_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/panic.c:463
  #1  0x400903fd in esp_system_abort at /home/lieven/.platformio/packages/framework-espidf/components/esp_system/port/esp_system_chip.c:92
  #2  0x4009613e in __assert_func at /home/lieven/.platformio/packages/framework-espidf/components/newlib/assert.c:80
  #3  0x400ec557 in btstack_run_loop_base_add_timer at /home/lieven/esp/esp-idf/components/btstack/src/btstack_run_loop.c:112
  #4  0x400ec7e3 in btstack_run_loop_add_timer at /home/lieven/esp/esp-idf/components/btstack/src/btstack_run_loop.c:280
  #5  0x400fa117 in ds4_play_dual_rumble_now at /home/lieven/workspace/bluepad32/src/components/bluepad32/parser/uni_hid_parser_ds4.c:652
  #6  0x400fa529 in uni_hid_parser_ds4_play_dual_rumble at /home/lieven/workspace/bluepad32/src/components/bluepad32/parser/uni_hid_parser_ds4.c:563
  #7  0x401b3d01 in Ps4Actor::trigger_event_on_gamepad(uni_hid_device_s*) at src/ps4_actor.cpp:249
  #8  0x400d8cf2 in Ps4Actor::on_device_ready(uni_hid_device_s*) at src/ps4_actor.cpp:151
  #9  0x400f79ed in uni_hid_device_set_ready_complete at /home/lieven/workspace/bluepad32/src/components/bluepad32/uni_hid_device.c:248 (discriminator 1)
  #10 0x400fa1ac in uni_hid_parser_ds4_setup at /home/lieven/workspace/bluepad32/src/components/bluepad32/parser/uni_hid_parser_ds4.c:240
  #11 0x400f7a4d in uni_hid_device_set_ready at /home/lieven/workspace/bluepad32/src/components/bluepad32/uni_hid_device.c:223
  #12 0x400f7f96 in uni_bt_bredr_process_fsm at /home/lieven/workspace/bluepad32/src/components/bluepad32/bt/uni_bt_bredr.c:310
  #13 0x400f81c1 in uni_bt_bredr_on_l2cap_channel_opened at /home/lieven/workspace/bluepad32/src/components/bluepad32/bt/uni_bt_bredr.c:469
  #14 0x400f5b7d in uni_bt_packet_handler at /home/lieven/workspace/bluepad32/src/components/bluepad32/bt/uni_bt.c:486
  #15 0x401b8fc2 in l2cap_dispatch_to_channel at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:1125
  #16 0x40196996 in l2cap_emit_channel_opened at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:1166
  #17 0x40199945 in l2cap_signaling_handler_channel at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:3546
  #18 0x40199b19 in l2cap_signaling_handler_dispatch at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:3777
  #19 0x40199c14 in l2cap_acl_classic_handler at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:4837
  #20 0x4019a255 in l2cap_acl_handler at /home/lieven/esp/esp-idf/components/btstack/src/l2cap.c:4953
  #21 0x400ee3a5 in hci_emit_acl_packet at /home/lieven/esp/esp-idf/components/btstack/src/hci.c:8041
  #22 0x400f3e69 in acl_handler at /home/lieven/esp/esp-idf/components/btstack/src/hci.c:1248
  #23 0x400f4023 in packet_handler at /home/lieven/esp/esp-idf/components/btstack/src/hci.c:4814
  #24 0x400ecffa in transport_deliver_packets at /home/lieven/esp/esp-idf/components/btstack/btstack_port_esp32.c:200
  #25 0x400f514d in btstack_run_loop_freertos_execute at /home/lieven/esp/esp-idf/components/btstack/platform/freertos/btstack_run_loop_freertos.c:172
  #26 0x400ec84f in btstack_run_loop_execute at /home/lieven/esp/esp-idf/components/btstack/src/btstack_run_loop.c:310
  #27 0x400d8a38 in Ps4Actor::on_start() at src/ps4_actor.cpp:37
  #28 0x400d6cc5 in Actor<Ps4Event, Ps4Cmd>::loop() at src/actor.h:290
  #29 0x400d6d8d in Actor<Ps4Event, Ps4Cmd>::start()::{lambda(void*)#1}::operator()(void*) const at src/actor.h:282
      (inlined by) Actor<Ps4Event, Ps4Cmd>::start()::{lambda(void*)#1}::_FUN(void*) at src/actor.h:279
  #30 0x400907f6 in vPortTaskWrapper at /home/lieven/.platformio/packages/framework-espidf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:134





ELF file SHA256: 17d4c9f2a

I (13319) esp_core_dump_flash: Save core dump to flash...
I (13326) esp_core_dump_flash: Erase flash 32768 bytes @ 0x180000
I (13710) esp_core_dump_flash: Write end offset 0x7444, check sum length 4
I (13710) esp_core_dump_flash: Core dump has been saved to flash.
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
I (31) boot: ESP-IDF 5.3.1 2nd stage bootloader
I (31) boot: compile time Jan 12 2025 00:08:14