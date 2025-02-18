# issue ASSERT_PARAM(8192 0), in rwbt.c at line 360
## solution
- not easy reproducible 
- I increased the stack size - just wil guessing as the backtrace was corrupted 
- Ps4Actor::Ps4Actor() : Actor<Ps4Event, Ps4Cmd>(6120, "ps4", 5, 10)
- the time of crash is impredictable
- the source code is not available and is hidden in a binary lib : https://github.com/espressif/esp32-bt-lib 
- here stops the investigation
- no idea what the assert is comparing 0 vs 8192 
- it doesn't seem to be the same line where it crashes for others , which is 381

## log
```
I 00:08:21.834 |        main.cpp: 184 |  free heap size: 33916
I 00:08:22.845 |        main.cpp: 184 |  free heap size: 33908
I 00:08:23.870 |        main.cpp: 184 |  free heap size: 34144
I 00:08:24.876 |        main.cpp: 184 |  free heap size: 35116
I 00:08:25.886 |        main.cpp: 184 |  free heap size: 35340
I 00:08:26.897 |        main.cpp: 184 |  free heap size: 35328
I 00:08:27.906 |        main.cpp: 184 |  free heap size: 35328
I 00:08:28.913 |        main.cpp: 184 |  free heap size: 34144
I 00:08:29.921 |        main.cpp: 184 |  free heap size: 34396
I 00:08:30.945 |        main.cpp: 184 |  free heap size: 34380
I 00:08:31.953 |        main.cpp: 184 |  free heap size: 35104
ASSERT_PARAM(8192 0), in rwbt.c at line 360
Guru Meditation Error: Core  0 panic'ed (IllegalInstruction). Exception was unhandled.
Memory dump at 0x4008ebfc: f01d020c 00004136 f01d0000
Core  0 register dump:
PC      : 0x4008ec03  PS      : 0x00060034  A0      : 0x800884da  A1      : 0x3ffc2108  
A2      : 0x00000000  A3      : 0x00002000  A4      : 0x00000000  A5      : 0x3ff9e7d2  
A6      : 0x00000168  A7      : 0xfffffffc  A8      : 0x8000814b  A9      : 0x3ffc2078  
A10     : 0x00000000  A11     : 0x3ffc209b  A12     : 0x3ffc2047  A13     : 0x00000030  
A14     : 0x00000000  A15     : 0x3ffc204c  SAR     : 0x00000004  EXCCAUSE: 0x00000000  
EXCVADDR: 0x00000000  LBEG    : 0x4000c2e0  LEND    : 0x4000c2f6  LCOUNT  : 0x00000000  


Backtrace: 0x4008ec00:0x3ffc2108 |<-CORRUPTED




ELF file SHA256: 77db749cd

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
I (31) boot: compile time Jan 10 2025 22:41:09
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
I (91) boot: End of partition table
I (96) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=3146ch (201836) map
I (173) esp_image: segment 1: paddr=00041494 vaddr=3ffbdb60 size=05d0ch ( 23820) load
I (182) esp_image: segment 2: paddr=000471a8 vaddr=40080000 size=08e70h ( 36464) load
I (197) esp_image: segment 3: paddr=00050020 vaddr=400d0020 size=ecb58h (969560) map
I (529) esp_image: segment 4: paddr=0013cb80 vaddr=40088e70 size=13a94h ( 80532) load
I (575) boot: Loaded app from partition at offset 0x10000
I (575) boot: Disabling RNG early entropy source...
I (587) cpu_start: Multicore app
I (596) cpu_start: Pro cpu start user code
I (596) cpu_start: cpu freq: 160000000 Hz
I (596) app_init: Application information:
I (599) app_init: Project name:     zenoh-lib
I (604) app_init: App version:      5866267-dirty
I (610) app_init: Compile time:     Jan 11 2025 11:48:34
I (616) app_init: ELF file SHA256:  77db749cd...
I (621) app_init: ESP-IDF:          5.3.0
I (626) efuse_init: Min chip rev:     v0.0
I (630) efuse_init: Max chip rev:     v3.99 
I (635) efuse_init: Chip rev:         v0.0
I (640) heap_init: Initializing. RAM available for dynamic allocation:
I (648) heap_init: At 3FFAFF10 len 000000F0 (0 KiB): DRAM
I (653) heap_init: At 3FFB6388 len 00001C78 (7 KiB): DRAM
I (660) heap_init: At 3FFB9A20 len 00004108 (16 KiB): DRAM
I (666) heap_init: At 3FFD8180 len 00007E80 (31 KiB): DRAM
I (672) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (678) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (685) heap_init: At 4009C904 len 000036FC (13 KiB): IRAM
I (693) spi_flash: detected chip: gd
I (695) spi_flash: flash io: dio
W (699) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I 00:00:00.022 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.027 |  wifi_actor.cpp:  15 | Starting WiFi actor sizeof(WifiCmd ) : 44 
I 00:00:00.034 |         actor.h:  23 | Channel created [6][4]  
I 00:00:00.040 | zenoh_actor.cpp:  16 | Starting WiFi actor sizeof(ZenohCmd ) : 48 
I 00:00:00.047 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.053 |   sys_actor.cpp:   6 | Starting Sys actor sizeof(SysCmd ) : 48 
I 00:00:00.060 |         actor.h:  23 | Channel created [10][4]  
I 00:00:00.066 |   ps4_actor.cpp:  20 | Starting PS4 actor sizeof(Ps4Cmd ) : 36 
I (765) coexist: coex firmware version: dab85ae96
I (771) main_task: Started on CPU0
I (775) main_task: Calling app_main()
I 00:00:00.113 |         actor.h: 237 | starting actor wifi
I 00:00:00.119 |         actor.h: 237 | starting actor zenoh
I (811) wifi:wifi driver task: 3ffdf820, prio:23, stack:6656, core=0
I (828) wifi:wifi firmware version: 0caa81945
I (828) wifi:wifi certification version: v7.0
I (828) wifi:config NVS flash: enabled
I (829) wifi:config nano formating: disabled
I (833) wifi:Init data frame dynamic rx buffer num: 32
I (838) wifi:Init static rx mgmt buffer num: 5
I (842) wifi:Init management short buffer num: 32
I (846) wifi:Init dynamic tx buffer num: 32
I (850) wifi:Init static rx buffer size: 1600
I (855) wifi:Init static rx buffer num: 10
I (858) wifi:Init dynamic rx buffer num: 32
I 00:00:00.172 |         actor.h: 237 | starting actor sys
I (864) wifi_init: rx ba win: 6
starting actor ps4I (871) wifi_init: accept mbox: 6

I (876) wifi_init: tcpip mbox: 32
cp tx win: 5760
I (892) wifi_init: tcp rx win: 5760
I (892) wifi_init: tcp mss: 1440
I (892) wifi_init: WiFi IRAM OP enabled
I (893) uart: queue free spaces: 10
I (904) wifi_init: WiFi RX IRAM OP enabled
I 00:00:00.226 |   ps4_actor.cpp:  75 | I (919) phy_init: phy_version 4830,54550f7,Jun 20 2024,14:22:08
custom: init()

W (1001) phy_init: saving new calibration data because of checksum failure, mode(0)
I (1006) BTDM_INIT: BT controller compile version [f021fb7]
I (1029) BTDM_INIT: Bluetooth MAC: 24:0a:c4:81:e0:3a
I (1032) wifi:mode : sta (24:0a:c4:81:e0:38)
I (1038) wifi:enable tsf
I (1294) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1, snd_ch_cfg:0x0
I (1298) wifi:state: init -> auth (0xb0)
I (1303) wifi:state: auth -> assoc (0x0)
I (1311) wifi:state: assoc -> run (0x10)
I (1319) wifi:<ba-add>idx:0 (ifx:0, 00:23:cd:19:fc:f5), tid:0, ssn:3149, winSize:64
BTstack up and running at 24:0A:C4:81:E0:3A
I 00:00:00.668 |   ps4_actor.cpp:  98 | custom: on_init_complete()

I 00:00:00.673 |   ps4_actor.cpp: 230 | custom: Bluetooth enabled: 1

26 54 34 A9 0D E0 26 CA 3A 6E BF 49 8E D7 9E FC 
I (1653) wifi:connected with Merckx2, aid = 5, channel 1, BW20, bssid = 00:23:cd:19:fc:f5
I (1657) wifi:security: WPA2-PSK, phy: bgn, rssi: -29
I (1665) wifi:pm start, type: 1

I (1667) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (1675) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I 00:00:01.175 |        main.cpp: 184 |  free heap size: 58896
I (1985) wifi:<ba-add>idx:1 (ifx:0, 00:23:cd:19:fc:f5), tid:6, ssn:2299, winSize:64
I 00:00:02.178 |        main.cpp: 184 |  free heap size: 58796
I (3177) esp_netif_handlers: sta ip: 192.168.0.247, mask: 255.255.255.0, gw: 192.168.0.1
I 00:00:02.548 | zenoh_actor.cpp:  52 | Connected to Zenoh.
I 00:00:02.551 | zenoh_actor.cpp: 134 | Declaring subscriber for 'dst/lm1/**'...
I 00:00:02.559 | zenoh_actor.cpp: 143 | OK
I 00:00:03.188 |        main.cpp: 184 |  free heap size: 39200
I 00:00:04.196 |        main.cpp: 184 |  free heap size: 39384
I 00:00:05.204 |        main.cpp: 184 |  free heap size: 39384
I 00:00:06.212 |        main.cpp: 184 |  free heap size: 39384
I 00:00:07.223 |        main.cpp: 184 |  free heap size: 37616
I 00:00:08.235 |        main.cpp: 184 |  free heap size: 39160
```