import mitt from 'mitt'

const emitter = mitt()

export default emitter

class PubSubClass {
    constructor(host, port, path, emitter) {
        console.log("PubSubClass", host, port, path, emitter);
        this.start_time = Date.now();
        this.host = host;
        this.port = port;
        this.socketPath = path;
        this.emitter = emitter || null;
        this.emitter.on('publish', msg => {
            console.log("PUBLISH", msg.topic, JSON.stringify(msg.value));
        });
        this.emitter.on('subscribe', (topic) => {
            this.subscribe(topic);
        });
        this.emitter.on('unsubscribe', (topic) => {
            this.unsubscribe(topic);
        });
        this.connected = false;
        // send data on timer
        this.connectionTimer = window.setInterval(() => {
            let v = Math.random() * 100;
            this.emitter.emit('published', { topic:"src/mtr1/motor.rpm_target", value: v });
            this.emitter.emit('published', { topic:"src/mtr1/motor.rpm_measured", value: v * 0.9 });
            // get current time in msec
            let uptime = Date.now() - this.start_time;
            this.emitter.emit('published', { topic:"src/mtr1/sys.uptime", value: uptime });
        }, 3000);


    }
    publish(topic, value) {
        this.emitter.emit("publish", { topic, value });
    }
    subscribe(topic) {
        console.log("SUBSCRIBE",topic);
    }
    unsubscribe(topic) {
        console.log("UNSUBSCRIBE",topic);
    }
    listen(topic, callback) {
        console.log("LISTEN", topic);
        this.emitter.on("published", (msg)=> {
            if (topic === msg.topic || topic === "*") {
                callback(msg);
            }
        });
    }
}

export const PubSub = new PubSubClass("limero.be", 7447, "/redis",mitt());

