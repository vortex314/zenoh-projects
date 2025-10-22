/*
    Browser <------> Zenoh WebSocket Bridge
    SAVE/LOAD/DELETE/UPDATE/QUERY JSON Objects ----> 
    PUB_TXD/SUBSCRIBE to topics --->
    PUB_RXD <---- 

    Using Zenoh WebSocket protocol
*/
import bus from "./LocalBus.js";
class WS {
    constructor(url, txd, rxd) {
        this.url = url;
        this.ws = null;
        this.connected = false;
        this.subscriptions = [];
        this.txd = txd;
        this.rxd = rxd;
        this.start_time = Date.now();
        this.txd.subscribe("**", (topic, value) => {
            if (this.connected)
                this.publish(topic, value);
        });
    }

    connect() {
        if (this.ws) {
            this.ws.close();
        }
        this.ws = new WebSocket(this.url);

        this.ws.onopen = () => {
            this.connected = true;
            console.log("Connected to WebSocket server");
        };
        this.ws.onclose = () => {
            this.connected = false;
            console.log("Disconnected from WebSocket server");
        };

        this.ws.onerror = (error) => {
            console.error("WebSocket error:", error);
        };

        this.ws.onsend = (event) => {
            console.log("Message sent:", event.data);
        }
        this.ws.onmessage = (event) => {
            const message = JSON.parse(event.data);
            console.log("Message received:", message);
            if (message.type === "Publish" && this.rxd) {
                console.log("WS RXD Publish:", message.topic, message.payload);
                this.rxd.publish(message.topic, message.payload);
            }
        };
        this.connectionTimer = window.setInterval(() => {
            this.rxd.publish("src/random/1", Math.random() );
            this.rxd.publish("src/random/10", Math.random()* 10);
            this.rxd.publish("src/random/100", Math.random() *100 );
            this.rxd.publish("src/random/1000", Math.random() *1000 );
            this.rxd.publish("src/random/bool", Math.random()<0.5?true:false);
            let uptime = Date.now() - this.start_time; // get current time in msec
            this.rxd.publish("src/uptime",  uptime );
        }, 1000);
    }

    connected() {
        return this.connected;
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
            this.connected = false;
        }
    }

    sendMessage() {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            var message = { type: "Publish", topic: 'src/esp1/sys/SysInfo', payload: { uptime: 12345 } };
            this.ws.send(JSON.stringify(message));
            message = { type: "Subscribe", topic: 'src/esp1/sys/SysInfo' };
            this.ws.send(JSON.stringify(message));
            message = { type: "Load", key: 'config/device1' };
            this.ws.send(JSON.stringify(message));
            message = { type: "Save", key: 'config/device1', payload: { setting: 'value' } };
            this.ws.send(JSON.stringify(message));
        } else {
            alert('WebSocket is not connected.');
        }
    }
    subscribe(topic_pattern) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            var message = { type: "Subscribe", topic: topic_pattern };
            this.ws.send(JSON.stringify(message));
        } else {
            console.log('WebSocket is not connected.');
        }
    }
    publish(topic, payload) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            var message = { type: "Publish", topic: topic, payload: payload };
            console.log("WS Publish:", JSON.stringify(message));
            this.ws.send(JSON.stringify(message));
        } else {
            console.log('WebSocket is not connected.');
        }
    }
    save(key, payload) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            var message = { type: "Save", key: key, payload: payload };
            this.ws.send(JSON.stringify(message));
        } else {
            console.log('WebSocket is not connected.');
        }
    }
    load(key) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            var message = { type: "Load", key: key };
            this.ws.send(JSON.stringify(message));
        } else {
            console.log('WebSocket is not connected.');
        }
    }
}

// Example usage:
const web_socket = new WS('ws://localhost:8080/ws', bus.txd, bus.rxd);
web_socket.connect();

export default web_socket;


