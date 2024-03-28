# straight forward proxy for USB , ESP-NOW and BLE
- Implements  mqtt sn based 
- uses CBOR Array for serialization, optional values can be null
- uses COBS ( Consistent Overhead Byte Stuffing) for encoding
- uses CRC16 for error checking
Pre-defined topics are used to communicate with the server
## Features
- [x] USB
- [x] ESP-NOW
- [x] BLE

## Pre-defined topics
- 1. Session MTU - default 128
- 2. Session AliveTime - default 60
- 3. Session SupportFragmentation - default false
- 4. Session MaxMessageSize - default 128
- 5. Session WillTopic - default null
- 6. Session WillMessage - default null
- 7. Session WillQOS - default 0
- 8. Session WillRetain - default false
- 9. Session Client Id - default link Id "/dev/ttyUSB0","BLE-ED:4D:4D:4D:4D:4D","ESP-NOW:4D:4D:4D:4D:4D"
// generate CBOR COBS code for the above messages






Client  ---------------> SessionHandler ---------------> Server
 ----- Connect ----------> 
 <----- Connect ACK ------
 <----- request Session xxx  ----------
----- Publish Session xxx + QOS ---------->
<----- request Register xxx  ----------
----- Publish Session  xxx + QOS ---------->
<----- Publish Topic Msg xxx + QOS ---------->

## Link for ESP-NOW and BLE
----> send(MAC1,bytes) -->
<---- recv(MAC2,bytes) <--
Gateway if CRC is ok , just forward the message to the server or client

