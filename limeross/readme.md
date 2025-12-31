```json
{
    "src":"brain",
    "dst":"",
    "msg_type":"Alive",
    "Alive":{
        "subscribe":["MotorEvent","SysEvent"] // mandatory
    }
}
```
```json
{
    "src":"ws", // => IP:PORT
    "dst":null,
    "msg_type":"Alive",
    "payload":{
        "subscribe" :["*"],
    }
}
```
```json
{
    "src":"broker",
    "Alive": {
    }
}
```
```json
{
    "src":"esp1",
    "Alive": {
        "req":["HoverboardReq"],
        "reply":["HoverboardReply"],
        "publish":["HoverboardEvent","SysEvent","WifiEvent"],
        "subscribe":[]
    }
}
```