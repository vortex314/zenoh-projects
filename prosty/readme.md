# reflections on linking eventbridges remotely
- The way of serialization should be generated code
  - handle JSON ( field names ), CBOR and PB ( field id's )
- the way to test message type should be generated code 
  - JSON check if field "MsgType" exist and is a Map
  - CBOR check if field MsgTypeId exist and is a map
  - PB check if a field with right pb_id exist
  - dependencies : serdes libs, Value ,
- the components should only see POJO messages ?
  - The local eventbus should be capable of transferring any msg type
- these POJO's should be local exploitable if needed, consumer could be local
- fields can be optional or mandatory 
- should message types also contain src & dst ? or in envelope ?
- should all internal messages be capable of serialization ?
-- the code generator can handle this , should not increase code size if not used
- should there be a limited list of supported types ? enum type in rust or std::variant in cpp
- how to recognize types of interest ? only src/dst or also msgtype
-- src/dst should ne enough
- how to avoid muliple parsing until msgtype is found without embedding blobs
-- intermediate parsing to value once : JSON OK, CBOR OK, PROTOBUF 
- should dst and src be string or can it be u32 ?
```cpp
struct MotorCmd : public Msg {
    inline uint32_t type_id() { return 2742; }
    constexpr uint32_t ID = fnv("MotorCmd")
    std::string dst;
    std::optional<int32_t> speed;
    std::optional<int32_t> direction;
}
struct MotorInfo : public Msg {
    atd::string src;
    std::optional<float> temp;
}
Result<MotorCmd> MotorCmd_from_value(Value&); // value is complete envelope ?
Result<Value> MotorInfo_to_value(MotorInfo&); 

MotorInfo_extract(Value&).do([&](MotorInfo* mi) { send( mi ))}); // check src , MotorInfo field => value to MotorInfo 

MotorInfo* mi = new MotorInfo(ref().name());
mi->temp = 37.6;

send ( mi );

```

```rust
struct MotorCmd {
    Option<i32> speed;
    Option<i32> direction;
}
impl ValueSerdes for MotorCmd {

}
```

optional values
deserialize values
convert types
# Bridge does serialization to polymorf value, Componet gets a API to access message - API 
- JSON,CBOR -> Value struct -> ConponentInfo struct 
- |- Bridge ---------------| |---- Component ----------------|
```cpp
    Value rxd = Rxd.value;
    auto cmd = MotorCmd::from_value(&Value);
    if ( cmd.hasSpeed() ) set_speed(cmd.speed());
    cmd.handle_speed([](auto i){ set_speed(i);});
    optional<i32> speed = cmd.get_speed();

    auto info = MotorInfo();
    info.speed( _speed );
    auto v = info.to_value();
    emit( new Txd(name(),v);)

```
## Pro 
- bridge doesn't have to know all message types
- no final translation to Component specific structure 
## Con
- no full deserialization check for mandatory fields 

# Bridge does serialization to polymorf value, Component maps this to own structure - POJO 
- JSOn,CBOR -> Value struct -> Map to Component Msg structure 
- |- Bridge ---------------| |---- Component ----------------|
```cpp
    auto cmd = MotorCmd::from_value(v);

    auto value = MotorInfo::to_value();
```
## Pro
- overall serialization gets checked 
## Con
- un-used fields get handled also -> bigger code 
- extra step 
- from & to_value need to know serilozation JSON vs CBOR 

# Bridge does deserialization based on known formats to target struct
- JSON,CBOR -> Component struct 
- |-------- Bridge ------------| 
## Pro
- deserilization cemtralized 
## Con
- registry of deser and ser
- how to recognize different schemas to call right deserilizer




src,dst,payload 

- pro
-- bridge defines serialization ( hmmm indexed by str or i32 )

- src = esp1/sys 
- src = esp1/sys/SysInfo
- src = mcu1/SysInfo

- CBOR ultra compact :

0=SRC_ID, SYS_INFO_ID = { 0=37298178312,1="ESP32-DEVKIT-1" }
1 = DST_ID, SYS_CMD_ID = { 0=94128471 }

- JSON

{ "src":"esp1/sys", "SysInfo" : { "uptime":37298178312 ,"board":"ESP32-DEVKIT-1"}}
{ "dst":"esp1/sys", "SysCmd" : { "uptime":37298178312 ,"board":"ESP32-DEVKIT-1"}}


- Rxd

value.with_speed([&](int speed){ setCcontrol(speed); }
value.set_speed(123.5);
value.clear();
float speed = value.get_speed();
bool value.has_speed();

Toggle value.get_button();

Msg has_SysInfo()-> bool

```json
{
  "src/esp1/sys/SysInfo/JSON" : {
    "uptime":49729374,
    "free_heap":130000
  },
  "dst/esp1/sys/SysCmd/JSON": {
    "reboot":true,
    "set_time":983487289
  }
}
```
- subscribe to "dst/esp1/*" on "esp1" device / eventbus - dst/src / DEVICE / COMPONENT / MSG_TYPE / FORMAT
- Home assistant only permits subsription to message type
