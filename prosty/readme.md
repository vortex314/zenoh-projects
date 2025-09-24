optional values
deserialize values
convert types

JSON,CBOR -> Value struct -> ConponentInfo struct 
       Bridge             Component        
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
