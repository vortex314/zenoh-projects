```
I 02:03:47.756 |        main.cpp: 194 |  free heap size: 19212 biggest block : 18432 
I 02:03:48.769 |        main.cpp: 194 |  free heap size: 17416 biggest block : 16384 
I 02:03:49.788 |        main.cpp: 194 |  free heap size: 17304 biggest block : 16384 
I 02:03:50.822 |        main.cpp: 194 |  free heap size: 21000 biggest block : 19456 
ASSERT_PARAM(8192 0), in rwbt.c at line 360
Guru Meditation Error: Core  0 panic'ed (IllegalInstruction). Exception was unhandled.
Memory dump at 0x4008eeac: f01d020c 00004136 f01d0000
Core  0 register dump:
PC      : 0x4008eeb3  PS      : 0x00060134  A0      : 0x80088376  A1      : 0x3ffc2670  
A2      : 0x00000000  A3      : 0x00002000  A4      : 0x00000000  A5      : 0x3ff9e7d2  
A6      : 0x00000168  A7      : 0xfffffffc  A8      : 0x8000814b  A9      : 0x3ffc25e0  
A10     : 0x00000000  A11     : 0x3ffc2603  A12     : 0x3ffc25af  A13     : 0x00000030  
A14     : 0x00000000  A15     : 0x3ffc25b4  SAR     : 0x00000004  EXCCAUSE: 0x00000000  
EXCVADDR: 0x00000000  LBEG    : 0x4000c2e0  LEND    : 0x4000c2f6  LCOUNT  : 0x00000000  


Backtrace: 0x4008eeb0:0x3ffc2670 0x40088373:0x3ffc2690 0x40055749:0x3ffc26b0 0x4008c03b:0x3ffc26d0 0x40081255:0x3ffc26f0 0x400811c1:0x3ffc2710 0x400811a3:0x00000000 |<-CORRUPTED




ELF file SHA256: e45ca4550

I (23005) esp_core_dump_flash: Save core dump to flash...
I (23013) esp_core_dump_flash: Erase flash 28672 bytes @ 0x180000
I (23416) esp_core_dump_flash: Write end offset 0x6e04, check sum length 4
I (23416) esp_core_dump_flash: Core dump has been saved to flash.
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
I (31) boot: compile time Jan 14 2025 01:25:13
I (31) boot: Multicore bootloader
I (35) boot: chip revision: v1.0
I (39) boot.esp32: SPI Speed      : 40MHz
I (44) boot.esp32: SPI Mode       : DIO
I (48) boot.esp32: SPI Flash Size : 4MB
I (53) boot: Enabling RNG early entropy source...
I (58) boot: Partition Table:
I (62) boot: ## Label            Usage          Type ST Offset   Length
I (69) boot:  0 nvs              WiFi data        01 02 00009000 00005000
I (76) boot:  1 otadata          OTA data         01 00 0000e000 00002000
I (84) boot:  2 app0             OTA app          00 10 00010000 00170000
I (91) boot:  3 coredump         Unknown data     01 03 00180000 00020000
I (99) boot: End of partition table
I (103) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=33a90h (211600) map
I (184) esp_image: segment 1: paddr=00043ab8 vaddr=3ffbdb60 size=06270h ( 25200) load
I (194) esp_image: segment 2: paddr=00049d30 vaddr=40080000 size=062e8h ( 25320) load
I (204) esp_image: segment 3: paddr=00050020 vaddr=400d0020 size=f0cc4h (986308) map
I (542) esp_image: segment 4: paddr=00140cec vaddr=400862e8 size=1695ch ( 92508) load
I (593) boot: Loaded app from partition at offset 0x10000
I (593) boot: Disabling RNG early entropy source...
I (605) cpu_start: Multicore app
I (614) cpu_start: Pro cpu start user code
I (614) cpu_start: cpu freq: 160000000 Hz
I (615) app_init: Application information:
I (617) app_init: Project name:     zenoh-lib
I (622) app_init: App version:      609f9e2-dirty
I (628) app_init: Compile time:     Jan 14 2025 00:27:08
I (634) app_init: ELF file SHA256:  e45ca4550...
I (639) app_init: ESP-IDF:          5.3.1
I (644) efuse_init: Min chip rev:     v0.0
I (649) efuse_init: Max chip rev:     v3.99 
I (654) efuse_init: Chip rev:         v1.0
I (659) heap_init: Initializing. RAM available for dynamic allocation:
I (666) heap_init: At 3FFAFF10 len 000000F0 (0 KiB): DRAM
I (672) heap_init: At 3FFB6388 len 00001C78 (7 KiB): DRAM
I (678) heap_init: At 3FFB9A20 len 00004108 (16 KiB): DRAM
I (684) heap_init: At 3FFD8A90 len 00007570 (29 KiB): DRAM
I (690) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (696) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (703) heap_init: At 4009CC44 len 000033BC (12 KiB): IRAM
I (711) spi_flash: detected chip: winbond
I (714) spi_flash: flash io: dio
I 00:00:00.009 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.014 |  wifi_actor.cpp:  23 | Starting WiFi actor sizeof(WifiCmd ) : 44 
I 00:00:00.021 |         actor.h:  23 | Channel created [6][4]  
I 00:00:00.027 | zenoh_actor.cpp:  16 | Starting WiFi actor sizeof(ZenohCmd ) : 48 
I 00:00:00.034 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.040 |   sys_actor.cpp:   6 | Starting Sys actor sizeof(SysCmd ) : 48 
I 00:00:00.047 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.053 |   ps4_actor.cpp:  20 | Starting PS4 actor sizeof(Ps4Cmd ) : 36 
I 00:00:00.060 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.066 |   led_actor.cpp:   8 | Starting LED actor sizeof(LedCmd ) : 16 
I (783) esp_core_dump_flash: Init core dump to flash
I (789) esp_core_dump_flash: Found partition 'coredump' @ 180000 131072 bytes
I (836) esp_core_dump_flash: Core dump data checksum is correct
I (836) esp_core_dump_flash: Found core dump 28164 bytes in flash @ 0x180000
I (839) coexist: coex firmware version: 4482466
I (846) main_task: Started on CPU0
I (849) main_task: Calling app_main()
I 00:00:00.181 |         actor.h: 289 | starting actor wifi
I 00:00:00.183 |         actor.h: 289 | starting actor zenoh
I 00:00:00.184 |         actor.h: 289 | starting actor sys
I 00:00:00.186 |         actor.h: 289 | starting actor ps4
starting actor led
I (915) uart: queue free spaces: 10
I 00:00:00.207 |   ps4_actor.cpp:  27 | btdm_controller_get_compile_version()=b022216
I (939) wifi:wifi firmware version: ccaebfaI 00:00:00.231 |   ps4_actor.cpp:  f6er num: 5
I (958) wifi:Init management short buffer num: 32
I (961) wifi:Init dynamic tx buffer num: 32I (959) BTDM_INIT: BT controller compile version [b022216]

I (972) wifi:Init static rx buffer size: 1600
I (975) wifi:I (976) BTDM_INIT: Bluetooth MAC: 30:ae:a4:ff:22:82
Init static rx buffer num: 10
I (985) phy_init: phy_version 4830,54550f7,Jun 20 2024,14:22:08
I (985) wifi:Init dynamic rx buffer num: 32
I (1001) wifi_init: rx ba win: 6
I (1004) wifi_init: accept mbox: 6
I (1006) wifi_init: tcpip mbox: 32
I (1008) wifi_init: udp mbox: 6
I (1009) wifi_init: tcp mbox: 6
I (1011) wifi_init: tcp tx win: 5760
I (1018) wifi_init: tcp rx win: 5760
I (1020) wifi_init: tcp mss: 1440
I (1027) wifi_init: WiFi IRAM OP enabled
I (1029) wifi_init: WiFi RX IRAM OP enabled
I (1087) wifi:mode : sta (30:ae:a4:ff:22:80)
I (1093) wifi:enable tsf
I (1364) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1, snd_ch_cfg:0x0
I (1369) wifi:state: init -> auth (0xb0)
I (1377) wifi:state: auth -> assoc (0x0)
I (1388) wifi:state: assoc -> run (0x10)
I (1395) wifi:<ba-add>idx:0 (ifx:0, 00:23:cd:19:fc:f5), tid:0, ssn:0, winSize:64
I 00:00:00.694 |  wifi_actor.cpp: 432 | Max AP number ap_info can hold = 10I (1412) wifi:
connected with Merckx2, aid = 3, channel 1, BW20, bssid = 00:23:cd:19:fc:f5
I (1423) wifi:security: WPA2-PSK, phy: bgn, rssi: -60
I (1430) wifi:pm start, type: 1

I (1432) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I 00:00:00.727 |  wifi_actor.cpp: 435 | Total APs scanned = 0, actual AP number ap_info holds = 0
I 00:00:00.735 |  wifi_actor.cpp: 455 | Highest RSSI = -128, AP index = 0
I 00:00:00.741 |  wifi_actor.cpp:  41 | Scanned SSID: 
W (1457) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
I (1464) wifi:I 00:00:00.754 |  wifi_actor.cpp: 148 | AP's beacon interval = 102400 us, DTIM period = 1Connecting to WiFi network...

BTstack up and running at 30:AE:A4:FF:22:82
I 00:00:00.816 |   ps4_actor.cpp:  99 | custom: on_init_complete()
I 00:00:00.824 |   ps4_actor.cpp: 231 | custom: Bluetooth enabled: 1
A9 45 EA CB 5A EF 53 2F E9 3F 7F 90 54 8D 38 75 
I (1727) wifi:<ba-add>idx:1 (ifx:0, 00:23:cd:19:fc:f5), tid:6, ssn:0, winSize:64
I 00:00:01.187 |        main.cpp: 194 |  free heap size: 54812 biggest block : 53248 
I 00:00:02.193 |        main.cpp: 194 |  free heap size: 54812 biggest block : 53248 
I (2945) esp_netif_handlers: sta ip: 192.168.0.231, mask: 255.255.255.0, gw: 192.168.0.1
I 00:00:02.312 | zenoh_actor.cpp:  52 | Connected to Zenoh.
I 00:00:02.316 | zenoh_actor.cpp: 156 | Declaring subscriber for 'dst/lm1/**'...
I 00:00:02.326 | zenoh_actor.cpp: 165 | OK
I 00:00:03.204 |        main.cpp: 194 |  free heap size: 33084 biggest block : 30720 
I 00:00:04.222 |        main.cpp: 194 |  free heap size: 34616 biggest block : 30720 
I 00:00:05.233 |        main.cpp: 194 |  free heap size: 32672 biggest block : 30720 
I 00:00:06.244 |        main.cpp: 194 |  free heap size: 32672 biggest block : 30720 
I 00:00:07.255 |        main.cpp: 194 |  free heap size: 32672 biggest block : 30720 
I 00:00:08.266 |        main.cpp: 194 |  free heap size: 32904 biggest block : 30720 
I 00:00:09.280 |        main.cpp: 194 |  free heap size: 32904 biggest block : 30720 
I 00:00:10.290 |        main.cpp: 194 |  free heap size: 32904 biggest block : 30720 
I 00:00:11.301 |        main.cpp: 194 |  free heap size: 32896 biggest block : 30720 
I 00:00:12.317 |        main.cpp: 194 |  free heap size: 34452 biggest block : 30720 
I 00:00:13.329 |        main.cpp: 194 |  free heap size: 34460 biggest block : 30720 
I 00:00:14.345 |        main.cpp: 194 |  free heap size: 32672 biggest block : 30720 
I 00:00:15.356 |        main.cpp: 194 |  free heap size: 32668 biggest block : 30720 
I 00:00:16.366 |        main.cpp: 194 |  free heap size: 32672 biggest block : 30720 
```