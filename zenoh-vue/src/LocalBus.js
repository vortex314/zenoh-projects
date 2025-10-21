import { EventEmitter2 } from 'eventemitter2';


class LocalBus {
    constructor() {
        this.emitter = new EventEmitter2({
            wildcard: true,     // Enable wildcards
            delimiter: '/',     // Default is '.'
          });
        this.start_time = Date.now();
        this.connected = false;
        // send data on timer


    }

    publish(topic, value) {
        this.emitter.emit(topic, value);      
    }
    subscribe(topic, handler) {
        this.emitter.on(topic, function (value) {
                handler(this.event,value);
            }
        );
    }
    unsubscribe(topic,handler) {
        this.emitter.off(topic,handler);
    }

    log_on() {
        this.emitter.onAny((topic, value) => {
            let uptime = Date.now() - this.start_time; // get current time in msec
            console.log(`[${uptime} ms] Topic: ${topic}, Value:`, value);
        });
    }
}

const bus = { txd:new LocalBus(), rxd:new LocalBus() };
export default  bus ;

