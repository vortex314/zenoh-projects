# Principles

## support big and small
- CBOR Binary compact protocol 
- JSON readable 
- each device has an IP address and joins multicast
- each device has its own port for the multicast where it listens for input
- publishing happens on a common port
- a robot has a multicast address for all its internal comms based on publish/subscribe

## 
Motor ( rpm, current ) <-> brain
Drive ( speed, direction ) <-> brain
Compass 
LPS 

# Features
## Publish
## Request/Reply 
## Metadata 
- Publish information on device+objects
### esp32/sys
- uptime : msec since boot
- build : date 
- 
### Wifi
-- 
```json
{
  "topic":"src/esp32-08AE/motor",
  "publish":{
    "rpm_target":3000,
    "rpm_measured":2700.8,
    "current":12.3
  },
  "info":{
    "rpm_target":{
      "type":"Number",
      "unit":"rpm",
      "desc":"Target  motor RPM",
      "range":[0.0,3000.0],
      "operations":"RW"
    },
    "rpm_measured":{
      "type":"Number",
      "unit":"rpm",
      "desc":"Measured motor RPM",
      "range":[0.0,3500.0],
      "access":"R"
    }
  },
  "topic":"dst/esp32-08AE/motor",
  "request":{
    "from":"dst/pcthink/brain",
    "id":345893274,
    "rpm_target":2800,
    "operation":["info","pub"]
  },
  "reply":{
    "from":"dst/esp32/motor",
    "id":345893274,
    "rc":0,
    "msg":"Went fine",
    "rpm_target":2800
  }
}
```