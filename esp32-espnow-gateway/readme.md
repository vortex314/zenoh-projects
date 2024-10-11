
# Features
Serves as a gateway between ESP-NOW WiFi and UART

Any data received from ESP-NOW is sent to UART
Any data received from UART is sent to ESP-NOW

To avoid data corruption, the data on UART is framed using COBS and CRC8

## Metadata 

No metadata is shared yet.

Metadata :
- Channel
- RSSI
- Source Mac
- Destination Mac

This serves just as a driver for the zenoh proxy service


