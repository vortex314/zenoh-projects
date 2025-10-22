import { EventEmitter2 } from 'eventemitter2';


class LocalBus {
    constructor() {
        this.emitter = new EventEmitter2({
            wildcard: true,     // Enable wildcards
            delimiter: '/',     // Default is '.'
          });
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
            let ts = Date.now()
            console.log(`[${ts}] Topic: ${topic}, Value:`, value);
        });
    }
}

const bus = { txd:new LocalBus(), rxd:new LocalBus() };
export default  bus ;

