# zenoh-vue

## 

## Websocket protocol
- assumption : the protocol uses the lowest granularity of the topics and is always JSON serialized
- example : src/esp1/motor/MotorInfo/rpm = 2324 or src/esp2/gps/GpsInfo = { "lon":37.44, "lat":4.0 }
```json
{
    "type":"pub", // "topics","sub","unsub","save_dashboard","load_dashboard","list_dashboard"
    "topic":"dst/esp1/sys/SysCmd", // "dst/server/dashboard/ListCmd", "dst/server/broker/BrokerCmd"
    "payload": { "reboot" :true },
    "message_topics":{"topics":["src/esp1/sys/SysInfo","src/esp1/motor/MotorInfo"]}
}
```
## Recommended IDE Setup

[VSCode](https://code.visualstudio.com/) + [Volar](https://marketplace.visualstudio.com/items?itemName=Vue.volar) (and disable Vetur).

## Customize configuration

See [Vite Configuration Reference](https://vite.dev/config/).

## Project Setup

```sh
npm install
```

### Compile and Hot-Reload for Development

```sh
npm run dev
```

### Compile and Minify for Production

```sh
npm run build
```
