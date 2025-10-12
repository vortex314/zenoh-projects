import { EventEmitter2 } from 'eventemitter2';


class LocalBus {
    constructor() {
        this.emitter = new EventEmitter2({
            wildcard: true,     // Enable wildcards
            delimiter: '/',     // Default is '.'
          });
        this.start_time = Date.now();
        this.emitter.on('subscribe', (topic) => {
            // WS subscribe
        });
        this.emitter.on('unsubscribe', (topic) => {
            // WS unsubscribe
        });
        this.emitter.on('save_request', (topic, value) => {
            // WS save
        });
        this.emitter.on('load_request', (topic) => {
            // WS load
        });
        this.connected = false;
        // send data on timer
        this.connectionTimer = window.setInterval(() => {
            let v = Math.random() ;
            this.emitter.emit("src/mtr1/motor.rpm_target", v * 2000 );
            this.emitter.emit("src/mtr1/motor.rpm_measured", v * 2000 );
            // get current time in msec
            let uptime = Date.now() - this.start_time;
            this.emitter.emit("src/mtr1/sys.uptime",  uptime );
        }, 3000);
        this.emitter.on("**", (value) => {
            console.log("LocalBus event",  value);
        });


    }
    publish(topic, value) {
        this.emitter.emit(topic, value);
        // send also to WS
    }
    subscribe(topic, handler) {
        this.emitter.emit('subscribe', topic);
        this.emitter.on(topic, function (value) {
                handler(this.event,value);
            }
        );
    }
    unsubscribe(topic,handler) {
        this.emitter.emit('unsubscribe', topic);
        this.emitter.off(topic,handler);
    }
}
const local_bus = new LocalBus();
export default  local_bus ;

