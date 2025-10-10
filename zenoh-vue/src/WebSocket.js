class WS {
    constructor(url) {
        this.url = url;
        this.ws = null;
        this.connected = false;
        this.subscriptions = [];
    }

    connect() {
        this.ws = new WebSocket(this.url);

        this.ws.onopen = () => {
            this.connected = true;
            console.log("Connected to Zenoh WebSocket server");
        };

        this.ws.onmessage = (event) => {
            const message = JSON.parse(event.data);
            console.log("Message received:", message);
            if ( message.reply == "LOAD") {
                messageBus.send({loaded:message.})
            } 
            if ( message.reply == "SAVE") {

            }
            if ( message.reply == "LIST") {

            }
            if ( message.request == "PUBLISH"){

            }
            if ( message.reply == "SUBSCRIBE") {

            }

            // Dispatch message to matching subscriptions
            this.subscriptions.forEach(subscription => {
                if (message.topic && message.topic.match(subscription.pattern)) {
                    subscription.callback(message);
                }
            });
        };

        this.ws.onerror = (error) => {
            console.error("WebSocket error:", error);
        };

        this.ws.send(message) = (message) => {};

        this.ws.onclose = () => {
            this.connected = false;
            console.log("Disconnected from Zenoh WebSocket server");
        };
    }

    subscribe(pattern, callback) {
        if (!this.connected) {
            class PubSub {
                constructor(url) {
                    this.url = url;
                    this.ws = null;
                    this.connected = false;
                    this.subscriptions = [];
                }
            
                connect() {
                    this.ws = new WebSocket(this.url);
            
                    this.ws.onopen = () => {
                        this.connected = true;
                        console.log("Connected to Zenoh WebSocket server");
                    };
            
                    this.ws.onmessage = (event) => {
                        const message = JSON.parse(event.data);
                        console.log("Message received:", message);
            
                        // Dispatch message to matching subscriptions
                        this.subscriptions.forEach(subscription => {
                            if (message.topic && message.topic.match(subscription.pattern)) {
                                subscription.callback(message);
                            }
                        });
                    };
            
                    this.ws.onerror = (error) => {
                        console.error("WebSocket error:", error);
                    };
            
                    this.ws.onclose = () => {
                        this.connected = false;
                        console.log("Disconnected from Zenoh WebSocket server");
                    };
                }
            
                subscribe(pattern, callback) {
                    if (!this.connected) {
                        console.error("Cannot subscribe, WebSocket not connected");
                        return;
                    }
            
                    this.subscriptions.push({ pattern, callback });
                    this.ws.send(JSON.stringify({ type:"SUBSCRIBE", topic: pattern}));
                    console.log(`Subscribed to pattern: ${pattern}`);
                }
            
                unsubscribe(pattern, callback) {
                    this.subscriptions = this.subscriptions.filter(subscription => {
                        return subscription.pattern !== pattern || subscription.callback !== callback;
                    });
            
                    if (this.connected) {
                        this.ws.send(JSON.stringify({ type:"UNSUBSCRIBE", topic:pattern}));
                        console.log(`Unsubscribed from pattern: ${pattern}`);
                    }
                }
            
                publish(topic, message) {
                    if (!this.connected) {
                        console.error("Cannot publish, WebSocket not connected");
                        return;
                    }
            
                    this.ws.send(JSON.stringify({type:"PUBLISH", topic:topic, message:message}));
                    console.log(`Published message to topic: ${topic}`);
                }
            
                disconnect() {
                    if (this.ws) {
                        this.ws.close();
                    }
                }
            }
            
            // Example usage:
            const pubSub = new PubSub("ws://localhost:8000"); // Replace with your Zenoh WebSocket URL
            pubSub.connect();
            
            // Wait for connection to establish before using commands
            setTimeout(() => {
                pubSub.subscribe("example/topic", (message) => {
                    console.log("Received message on example/topic:", message);
                });
            
                pubSub.publish("example/topic", { data: "Hello, Zenoh!" });
            
                setTimeout(() => {
                    pubSub.unsubscribe("example/topic", () => {});
                    pubSub.disconnect();
                }, 5000);
            }, 1000);  console.error("Cannot subscribe, WebSocket not connected");
            return;
        }

        this.subscriptions.push({ pattern, callback });
        this.ws.send(JSON.stringify(["SUBSCRIBE", pattern]));
        console.log(`Subscribed to pattern: ${pattern}`);
    }

    unsubscribe(pattern, callback) {
        this.subscriptions = this.subscriptions.filter(subscription => {
            return subscription.pattern !== pattern || subscription.callback !== callback;
        });

        if (this.connected) {
            this.ws.send(JSON.stringify(["UNSUBSCRIBE", pattern]));
            console.log(`Unsubscribed from pattern: ${pattern}`);
        }
    }

    publish(topic, message) {
        if (!this.connected) {
            console.error("Cannot publish, WebSocket not connected");
            return;
        }

        this.ws.send(JSON.stringify({request:"PUBLISH",topic:topic,payload:message}));
        console.log(`Published message to topic: ${topic}`);
    }

    save_json(id,json) {
        if (this.connected) {
            this.ws.send(JSON.stringify({request:"SAVE",id:id,json:json}));
            console.log(`Saving Object `,id);
        }
    }

    load_json(id ){
        if (this.connected) {
            this.ws.send(JSON.stringify({request:"LOAD",id:id}));
            console.log(`Loading Object : ${id}`);
        }
    }

    list_json(id ){
        if (this.connected) {
            this.ws.send(JSON.stringify({request:"LIST",id:id}));
            console.log(`Listing Objects : `,id);
        }
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
        }
    }
}

// Example usage:
const pubSub = new WS("ws://localhost:8000"); // Replace with your Zenoh WebSocket URL
pubSub.connect();

// Wait for connection to establish before using commands
setTimeout(() => {
    pubSub.subscribe("example/topic", (message) => {
        console.log("Received message on example/topic:", message);
    });

    pubSub.publish("example/topic", { data: "Hello, Zenoh!" });

    setTimeout(() => {
        pubSub.unsubscribe("example/topic", () => {});
        pubSub.disconnect();
    }, 5000);
}, 1000);