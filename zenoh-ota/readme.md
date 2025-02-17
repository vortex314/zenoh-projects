
# USe OTA ( Over the Air updates ) via Zenoh
## Features
- Send new Image to ESP32 ( or other )
- Send in 1 Publish message using PSRAM on ESP32 CAM
- works together with OtaActor ( zenoh-esp32 ) sub-project 
- Send a local firmware.bin image to a topic on zenoh 
- Use Zenoh as a transport media
## backlog
- expect reply on own topic if OTA succeeded
- Send in multiple chunks and with feedback via queryable for ESP32 with less RAM
- discover ota ready micro-controllers
## Releases
### Release 1
- one single +/- 1MB image , supposes a PSRAM amount at ESP32 side
- no confirmation
### Release ...
