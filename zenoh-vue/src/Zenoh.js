/*jshint esversion: 6 */
import Vue from 'vue';

export const RedisState = new Vue({
    name: "RedisState",
    data() {
        return {
            connected: false
        };
    }
});

class Queue {
    constructor() {
        this.elements = {};
        this.head = 0;
        this.tail = 0;
        this.timer = null;
    }
    enqueue(element) {
        this.elements[this.tail] = element;
        this.tail++;
    }
    dequeue() {
        const item = this.elements[this.head];
        delete this.elements[this.head];
        this.head++;
        return item;
    }
    peek() {
        return this.elements[this.head];
    }
    get length() {
        return this.tail - this.head;
    }
    get isEmpty() {
        return this.length === 0;
    }
}

class RedisClass {
    constructor(host, port, socketPath) {
        this.host = host;
        this.port = port;
        this.queue = new Queue();
        this.connected = false;
        this.subscriptions = [];
        this.socketPath = socketPath;
        this.request = this.request.bind(this);
        this.onConnected = this.onConnected.bind(this);
        this.onMessage = this.onMessage.bind(this);
        this.promises = new Queue();
        this.autoConnect = true;
        this.connectionTimer = window.setInterval(() => {
            if (!this.connected && this.autoConnect) {
                this.connect();
            }
        },3000);
    }
    webSocketUrl() {
        return "ws://" + this.host + ":" + this.port + this.socketPath;
    }
    isStringArray(arr) {
        return arr instanceof Array && arr.every(x => typeof x == "string");
    }
    request(rq) {


        let promise = new Promise((resolve, reject) => {
            if (!this.isStringArray(rq)) {
                reject("Redis request error: " + rq + " is not an array");
            }
            else if (!this.connected) {
                reject("Redis not connected");
            } else {
                let rp = { cmd: rq[0], resolve: resolve, reject: reject };
                this.command(rq);
                this.promises.enqueue(rp);
                console.log("Redis request queued for answer: ", rq);
            }
        })
        return promise;
    }
    publish(channel, message) {
        this.request(["PUBLISH", channel, JSON.stringify(message)]).catch(console.log);
    }
    onConnected() {
        console.log("Redis connected");
        this.connected = true;
        RedisState.connected = true;
        this.request(["hello", "3"]).then(() => {
            //  console.log("hello response", x)
        }).catch(console.log);
        this.subscriptions.forEach(subscription => {
            this.request(["PSUBSCRIBE", subscription.pattern]).then(() => {
                //  console.log("Redis response", x)
            }).catch(console.log);
        });
        Eventbus.$emit("Redis.connected", true);
        /* this.timer = setInterval(() => {
             Redis.request(
                 ["publish", "src/hover/motor/targetAngle", Math.round((Math.random() * 180) - 90).toString()]);
             Redis.request(
                 ["publish", "src/hover/motor/measuredAngle", Math.round((Math.random() * 180) - 90).toString()]);
             Redis.request(
                 ["publish", "src/hover/motor/currentLeft", Math.round((Math.random() * 50) ).toString()]);
             Redis.request(
                 ["publish", "src/hover/motor/currentRight", Math.round((Math.random() * 50) ).toString()]);
         }, 100);*/
    }
    onDisconnected() {
        console.log("Redis disconnected");
        RedisState.connected = false;
        Eventbus.$emit("Redis.disconnected", false);
    }
    onMessage(message) {
        console.log("Redis response: ", message.data);
        try {
            var arr = JSON.parse(message.data);
            var cmd = arr[0];
            switch (cmd.toLowerCase()) {
                case "pmessage":
                    this.subscriptions.forEach(subscription => {
                        if (subscription.pattern == arr[1]) {
                            subscription.callback(arr[2], JSON.parse(arr[3]));
                        }
                    });
                    break;

                default: {
                    console.log("Redis reply", arr);
                    let rp = this.promises.dequeue();
                    if (rp.cmd.toLowerCase() != cmd.toLowerCase()) {
                        console.log("ERROR: ", rp.cmd, cmd);
                        rp.reject("Redis reply error " + rp.cmd + " != " + cmd);
                    } else {
                        rp.resolve(arr);
                    }
                    break;
                }
            }
        } catch (e) { console.log("Redis message exception", e, "on", message); }
    }
    subscribe(pattern, action) {
        this.subscriptions.push({ pattern: pattern, callback: action });
        if (this.connected) {
            this.request(["PSUBSCRIBE", pattern]).then((x) => {
                console.log("PSUBSCRIBE response", x);
            }).catch(console.log);
        }
    }
    unsubscribe(pattern, callback) {
        this.subscriptions = this.subscriptions.filter(subscription => {
            return subscription.pattern != pattern && subscription.callback != callback;
        });
        if (this.connected) {
            this.request(["PUNSUBSCRIBE", pattern]).then((x) => {
                console.log("PUNSUBSCRIBE response", x);
            }).catch(console.log);
        }
    }
    addHandler(strings, action) {
        if (strings instanceof Array) {
            strings.forEach(s => {
                this.subscribe(s, action);
            });
        }
    }
    onError(error) {
        console.log(error);
    }
    command(arr) {
        if (!this.isStringArray(arr)) {
            alert("Redis request error: " + arr + " is not an array");
            return;
        }
        if (!this.connected) {
            alert("Redis not connected");
            return
        }
        //console.log("Redis request: ", arr);
        this.ws.send(JSON.stringify(arr));
    }

    connect() {
        console.log("Redis connecting to " + this.webSocketUrl());
        this.ws = new WebSocket(this.webSocketUrl(), [
            "string",
            "foo",
        ]);
        this.ws.onopen = this.onConnected;
        this.ws.onclose = this.onDisconnected;
        this.ws.onmessage = this.onMessage;
        this.ws.onerror = this.onError;
    }
    disconnect() {
        this.ws.close();
        this.connected = false;
        RedisState.connected = false;
    }
    configure(host, port, socketPath) {
        this.host = host;
        this.port = port;
        this.socketPath = socketPath;
    }

}

export const Redis = new RedisClass("limero.local", 9000, "/redis");

export const Eventbus = new Vue()

class TimerSingletonClass {
    constructor() {
        this.tick = 1000;
        this.timers = [];
        setInterval(() => { this.nextTick(); }, 1000);
    }
    nextTick() {
        this.timers.forEach(timer => {
            if (timer.expired()) {
                timer.run();
                if (timer.isContinuous) { timer.reset(); }
                else { timer.stop(); }
            }
        });
    }
    add(timer) {
        this.timers.push(timer);
    }
}

export const MainClock = new TimerSingletonClass()

export class Timer {
    constructor(timeout, callback, isContinuous) {
        this.timeout = timeout;
        this.callback = callback;
        this.isContinuous = isContinuous;
        this.expiresAt = new Date().getTime() + timeout;
        this.isRunning = true;
        MainClock.add(this);
    }

    expired() {
        return this.isRunning && (new Date().getTime() > this.expiresAt)
    }
    run() {
        this.callback()
        if (this.isContinuous) {
            this.expiresAt = new Date().getTime() + this.timeout;
        } else {
            this.isRunning = false;
        }
    }
    stop() {
        this.isRunning = false;
    }
    start() {
        this.isRunning = true
        this.expiresAt = new Date().getTime() + this.timeout;
    }
    reset() {
        this.expiresAt = new Date().getTime() + this.timeout;
    }
    dispose() {
        MainClock.timers.splice(MainClock.timers.indexOf(this), 1);
        delete this;
    }
}

export function newKey() {
    return Math.round(Math.random() * 1000000000).toString();
}




