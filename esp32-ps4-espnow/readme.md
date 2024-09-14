# how to get a PS4 controller connect via esp-now 
- make sure you have 5.1.4 esp-idf installed
```
git clone -b v5.1.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32
```
- make sure no other versions of xtensa compiler or esp-idf versions conflict ( rust )
- clone project https://github.com/ricardoquesada/bluepad32
```
- goto examples esp32
- execute instructions of readme 
-- install btstack 
- go to 

```
bp32> list_devices
Connected devices:
idx=0:
	btaddr: 8C:41:F2:D2:E5:48
	bt: handle=129 (ACL), hids_cid=0, ctrl_cid=0x0042, intr_cid=0x0043, cod=0x00000508, flags=0x00003b00, incoming=0
	model: vid=0x054c, pid=0x05c4, model='DualShock 4', name='Wireless Controller'
	battery: 76 / 255, type=gamepad
	DS4: FW version 0x51, HW version 0x3100

bp32> 
```

