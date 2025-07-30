# Target 
- have a robotic middleware that reaches low latency for small messages ( < 1.5K )
- it should be as small as possible in code size
- support 2 models : request/reply and publish/subscribe
- enable very small micro-controller to communicate with linux systems and with each other
- a device should announce itself via multicast udp on a regular base : 5 sec, indicating its own udp ip address and port
- a device should be able to find the central broker via these announcements
- a device can subscribe to publish events based on 'src' field
- a device can also subscribe to messages based on the 'dst' field for request and replies
- a central broker registers all devices and any message sent to it will route the message to the right destination using standard udp 
- messages are all formatted in json, see below for syntax
- developer language should be rust for linux and c++ for microcontroller
- for the linux broker use async and actix actors

# Multicast announce 
- From IP/PORT

```json
{
    "src":"motor",
    "announce":{
        "desc":"Motor control of cutter"
    }
}
```
# Subscribe
- To IP/Port
- From IP/PORT

```json
{
    "dst":"dashboard",
    "src":"motor",
    "sub":{
        "timeout":600
    }
}
```

# Publish
- To IP/Port
- From IP/port
```json
{
    "src":"esp1/sys",
    "pub": {
        "free_heap":126700,
        "uptime":23566,
        "cpu":"esp32"
    }
}
```
# Describe
- From / To 
```json
{
    "src":"dashboard",
    "dst":"",
    "desc": ["object","fields"]
}
```

# Request 
Commands
- set -- fields
- get -- fields
- desc -- object, events,fields 
- req

```json
{
    "dst":"esp1/sys",
    "src":"pclenovo/brain",
    "set":{
        "action":"reset"
    }
}
```
# Response 
```json
{
    "dst":"pclenovo/brain",
    "src":"esp1/sys",
    "rep":{
        "action":"rebooting"
    }
}
```