
## architecture
- main 
-- uip_input
-- uip_output ? 
- uip
-- ppp.send_ip_packet
- ppp
-- ppp.serial_send 
- uart comm

MAIN -> uip_send -> UIP -> ppp.send_ip_packet -> PPP -> ppp.serial_send -> UART
UART -> process_rx_bytes -> ppp.process_bytes -> PPP -> uip.

## Build challenges
### Error: libusb_open() failed with LIBUSB_ERROR_ACCESS
- https://docs.platformio.org/en/stable/core/installation/udev-rules.html
### embedded:startup.tcl:1516: Error: ** Unable to reset target **
![maple mini pinout](doc/maplemini_pinout.png)
- root cause , defective STLINK
### library.json for lwip
- https://docs.platformio.org/en/latest/manifests/library-json/index.html
