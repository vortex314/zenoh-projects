```mermaid
graph TD;
    A-->B;
    A-->C;
    B-->D;
    C-->D;
```
## Sample sequence diagram
Here is a Hello World example.
```uml-sequence-diagram
Title: PubSub Proxy client-server
Client->>Server: ProxyMessage::Connect { protocol_id: 1, client: "client1" }
Server->>Client: ProxyMessage::ConnAck { return_code: 0 }
Server->>Client: ProxyMessage::Publish { topic: "topic1", payload: "Hello World",qos:1 }
Client->>Server: ProxyMessage::PubAck { return_code: 0 }
Client->>Server: ProxyMessage::Subscribe { topic: "topic1", qos: 1 }
Server->>Client: ProxyMessage::SubAck { return_code: 0 }
Client->>Server: ProxyMessage::PingReq
Server->>Client : ProxyMessage::PingResp
Server->>Client: ProxyMessage::Publish { topic: "topic1", payload: "Hello World",qos:1 }
Client->>Server: ProxyMessage::Register { topic_id : 1, topic: "topic1" }
Server->>Client: ProxyMessage::RegAck { return_code: 0 }
Client->>Server: ProxyMessage::Disconnect
```