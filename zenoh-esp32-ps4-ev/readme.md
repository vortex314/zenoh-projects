# Latest model 6/10/2025

## Middleware approach - back to Zenoh 

### Central schema
- Messages are JSON/CBOR/.. based on a .proto protobuf alike schema 
- All messages related to 1 robot unit are in 1 proto file
- parsing and struct are generated code : C++ or Rust, permits to align between devices and brain
- the schemas are supporting backwards compatibility by making most/all fields optional 
- the id's used should be peristent : string or hash from string
- the code generation is easily extendible with .tera templates
### Pub Sub
- The communication uses principles that are also used in MQTT, Zenoh,.. - topic , payload
- The communication uses : topic and payload 
  - The topic indicates : 
    - direction : src or dst, to ease subscription patterns 
    - device : a separate link for all communications
    - component : entity on the device
    - message_type : indicates the schema to be used for parsing the message payload
    - serialization : serialization type used JSON/CBOR/PROTO,optional , default JSON
  - payload : serialized form of the message, bunch of 
- Rationale : device can subscribe to "src/device/*", the device will put the parsed message on the local eventbus
- Rationale : the message can be deserialized just based on the topic and the message is a flat structure
- the local eventbus will see event : <message_type> <data..> ( ? will the missing topic info create an issue ? if multiple instances look for the same message_type )
- Payload is related to the full component granularity ( like ComponentInfo ), but can be occasionally limited to some frequent updated fields. Tha rationale is that the effort ( cpu cycles ) to communicate small granularity ( single field ) is almost the same as communicate ( component granularity )
- messages that are request / reply should contain a reply field which becomes a dst field
### Peer-to-peer
- independent devices should be able to communicate directly without a central broker
- Rationale : avoid SPOF
### Subscription
- The messages frequency locally are determined by components themselves
- Subscriptions and frequency are handled by eventbridge or middleware ( zenoh ) 
### Alive detection
- messages from components are themselves indications of being alive, they should be frequent enough to address the use case
- Rationale : there should not be a association between devices alive and component alve to maintain
### Future performance
- the ultimate binary compression could be where :
  - topics become u32 hashes
  - field names become u16 hashes
  - enum becomes u8 values
### Examples
- example : "src/esp1/motor/MotorInfo/JSON" -> {"rpm":12233,"temp":40.5}
```proto
message MotorInfo {
   int32 rpm = 1;
   float temp = 2;
}
```
- generated code Rust
```rust
struct MotorInfo {
    rpm : Option<i32> ,
    temp : Option<f32>
}
impl MotorInfo {
    fn deserialize(buffer:Vec<u8>) -> Result<MotorInfo> {};
    fn serialize(&self) -> Result<Vec<u8>> {};
}
```
- generated code C++
```c++
class MotorInfo : public Msg {
    public:
    static constexpr const char *id = "MotorInfo";     
    inline const char *type_id() const override { return id; }; 
    static const uint32_t ID = 33380;

    std::optional<uint32_t> rpm;
    std::optional<float> temp;

    Result<Bytes> serialize();
    static Result<MotorInfo*> deserialize(Bytes payload); // pointer to ease sending  or maybe unique_pointer
}
```



# Previous reflections 

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

# Multicast hello , subscribes and heartbeat
- From IP/PORT
- if ip and port not given , take the UDP address.

```json
{
    "src":"brain",
    "subs":["motor"]
}
```
# Subscribe
- To IP/Port
- From IP/PORT

```json
{
    "dst":"motor",
    "src":"brain",
    "subscribe":{
    }
}
```

# Publish
- To IP/Port - of known subscribers
- From IP/port
```json
{
    "src":"esp1_sys",
    "SysInfo": {
        "free_heap":126700,
        "uptime":23566,
        "cpu":"esp32"
    }
}
```
```json
{
    "dst":"esp1_motor",
    "MotorCmd": {
        "rpm":2345
    },
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
    "dst":"brain",
    "src":"esp1/sys",
    "rep":{
        "action":"rebooting"
    }
}
```

Bluepad32 for ESP‑IDF requires and uses BTstack—not ESP-IDF’s native Bluedroid or NimBLE stack.
### Integration Overview

Bluepad32 includes an external/btstack directory, which must be patched and integrated into your project. After cloning the repo you run:
Shellcd external/btstack/port/esp32IDF_PATH=../../../../src ./integrate_btstack.pyShow more lines
This installs BTstack as a component in your ESP-IDF project. [github.com]
The build instructions emphasize patching BTstack before compiling Bluepad32. [bluepad32....thedocs.io]

### Why not Bluedroid or NimBLE?

BTstack is tailored by Bluepad32 to support HID host functionality (gamepads, mice, keyboards) over both Classic and BLE.
ESP-IDF’s built-in stacks — Bluedroid (supports Classic + BLE) or NimBLE (BLE-only) — are not used. Instead, BTstack runs on a dedicated BTstack task in ESP-IDF. [bluepad32....thedocs.io], [bluepad32....thedocs.io]

