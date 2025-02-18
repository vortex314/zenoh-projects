I 00:55:05.204 |        main.cpp: 189 |  free heap size: 25956
I 00:55:06.215 |        main.cpp: 189 |  free heap size: 24400
I 00:55:07.224 |        main.cpp: 189 |  free heap size: 22356
ASSERT_PARAM(8192 0), in rwbt.c at line 360
Guru Meditation Error: Core  0 panic'ed (IllegalInstruction). Exception was unhandled.
Memory dump at 0x4008eeac: f01d020c 00004136 f01d0000
Core  0 register dump:
PC      : 0x4008eeb3  PS      : 0x00060934  A0      : 0x80088662  A1      : 0x3ffc2670  
A2      : 0x00000000  A3      : 0x00002000  A4      : 0x00000000  A5      : 0x3ff9e7d2  
A6      : 0x00000168  A7      : 0xfffffffc  A8      : 0x8000814b  A9      : 0x3ffc25e0  
A10     : 0x00000000  A11     : 0x3ffc2603  A12     : 0x3ffc25af  A13     : 0x00000030  
A14     : 0x00000000  A15     : 0x3ffc25b4  SAR     : 0x00000004  EXCCAUSE: 0x00000000  
EXCVADDR: 0x00000000  LBEG    : 0x4000c2e0  LEND    : 0x4000c2f6  LCOUNT  : 0x00000000  


Backtrace: 0x4008eeb0:0x3ffc2670 0x4008865f:0x3ffc2690 0x40055749:0x3ffc26b0 0x40088d5f:0x3ffc26d0 0x40081255:0x3ffc26f0 0x400811c1:0x3ffc2710 0x400811a3:0x00000000 |<-CORRUPTED




ELF file SHA256: d9cfb4e1a

I (7002) esp_core_dump_flash: Save core dump to flash...
I (7009) esp_core_dump_flash: Erase flash 28672 bytes @ 0x180000
I (7214) esp_core_dump_flash: Write end offset 0x6e04, check sum length 4
I (7214) esp_core_dump_flash: Core dump has been saved to flash.
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
I (31) boot: Multicore bootloader
I (35) boot: chip revision: v0.0
I (39) boot.esp32: SPI Speed      : 40MHz
I (44) boot.esp32: SPI Mode       : DIO
I (48) boot.esp32: SPI Flash Size : 2MB
I (53) boot: Enabling RNG early entropy source...
I (58) boot: Partition Table:
I (62) boot: ## Label            Usage          Type ST Offset   Length
I (69) boot:  0 nvs              WiFi data        01 02 00009000 00005000
I (76) boot:  1 otadata          OTA data         01 00 0000e000 00002000
I (84) boot:  2 app0             OTA app          00 10 00010000 00170000
I (91) boot:  3 coredump         Unknown data     01 03 00180000 00020000
I (99) boot: End of partition table
I (103) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=33ad8h (211672) map
I (184) esp_image: segment 1: paddr=00043b00 vaddr=3ffbdb60 size=06274h ( 25204) load
I (194) esp_image: segment 2: paddr=00049d7c vaddr=40080000 size=0629ch ( 25244) load
I (204) esp_image: segment 3: paddr=00050020 vaddr=400d0020 size=f31e0h (995808) map
I (545) esp_image: segment 4: paddr=00143208 vaddr=4008629c size=169a8h ( 92584) load
I (596) boot: Loaded app from partition at offset 0x10000
I (596) boot: Disabling RNG early entropy source...
I (609) cpu_start: Multicore app
I (617) cpu_start: Pro cpu start user code
I (617) cpu_start: cpu freq: 160000000 Hz
I (617) app_init: Application information:
I (620) app_init: Project name:     zenoh-lib
I (625) app_init: App version:      a01acb9-dirty
I (631) app_init: Compile time:     Jan 12 2025 00:06:49
I (637) app_init: ELF file SHA256:  d9cfb4e1a...
I (642) app_init: ESP-IDF:          5.3.1
I (647) efuse_init: Min chip rev:     v0.0
I (651) efuse_init: Max chip rev:     v3.99 
I (656) efuse_init: Chip rev:         v0.0
I (661) heap_init: Initializing. RAM available for dynamic allocation:
I (668) heap_init: At 3FFAFF10 len 000000F0 (0 KiB): DRAM
I (674) heap_init: At 3FFB6388 len 00001C78 (7 KiB): DRAM
I (680) heap_init: At 3FFB9A20 len 00004108 (16 KiB): DRAM
I (687) heap_init: At 3FFD8AB0 len 00007550 (29 KiB): DRAM
I (693) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (699) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (706) heap_init: At 4009CC44 len 000033BC (12 KiB): IRAM
I (714) spi_flash: detected chip: gd
I (716) spi_flash: flash io: dio
W (720) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I 00:00:00.022 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.027 |  wifi_actor.cpp:  19 | Starting WiFi actor sizeof(WifiCmd ) : 44 
I 00:00:00.034 |         actor.h:  23 | Channel created [6][4]  
I 00:00:00.040 | zenoh_actor.cpp:  16 | Starting WiFi actor sizeof(ZenohCmd ) : 48 
I 00:00:00.047 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.053 |   sys_actor.cpp:   6 | Starting Sys actor sizeof(SysCmd ) : 48 
I 00:00:00.060 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.066 |   ps4_actor.cpp:  20 | Starting PS4 actor sizeof(Ps4Cmd ) : 36 
I 00:00:00.073 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.079 |   led_actor.cpp:   8 | Starting LED actor sizeof(LedCmd ) : 16 
I (799) esp_core_dump_flash: Init core dump to flash
I (805) esp_core_dump_flash: Found partition 'coredump' @ 180000 131072 bytes
I (851) esp_core_dump_flash: Core dump data checksum is correct
I (851) esp_core_dump_flash: Found core dump 28164 bytes in flash @ 0x180000
I (855) coexist: coex firmware version: 4482466
I (861) main_task: Started on CPU0
I (864) main_task: Calling app_main()
I 00:00:00.194 |         actor.h: 289 | starting actor wifi
I 00:00:00.197 |         actor.h: 289 | starting actor zenoh
I 00:00:00.198 |         actor.h: 289 | starting actor sys
I 00:00:00.199 |         actor.h: 289 | starting actor ps4
I (912) wifi:starting actor ledwifi driver task: 3ffdfc70, prio:23, stack:6656, core=0
I (936) uart: queue free spaces: 10
I 00:00:00.249 |   ps4_actor.cpp:  75 | I (960) wifi:custom: init()wifi firmware version: ccaebfa

