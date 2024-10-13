# Zenoh Protocol Documentation

Based on the provided Wireshark dissector code, here's an overview of the Zenoh protocol structure:

## 1. Transport Layer

The Zenoh protocol starts with a transport layer, represented by the `TransportMessage` struct. This layer handles the following message types:

- InitSyn
- InitAck
- OpenSyn
- OpenAck
- Close
- KeepAlive
- Frame
- Fragment
- OAM (Operation and Management)
- Join

### Key Transport Message Components:

- **InitSyn/InitAck**: Used for initializing a connection, including version negotiation, entity type (whatami), Zenoh ID (zid), and various extensions.
- **OpenSyn/OpenAck**: Used for opening a session, including lease time, initial sequence number, and extensions.
- **Frame**: Contains actual payload data (NetworkMessages) along with reliability and sequence number information.
- **Fragment**: Used for handling large messages that need to be split into multiple parts.
- **Join**: Used for joining a session, including version, entity type, and various parameters.

## 2. Zenoh Layer

The Zenoh layer builds on top of the transport layer and defines several message types:

- Put
- Del
- Query
- Reply
- Err

### Key Zenoh Message Components:

- **Put**: Used to publish data, including timestamp, encoding, and payload.
- **Del**: Used to delete data, including timestamp.
- **Query**: Used to request data, including parameters and consolidation info.
- **Reply**: Used to respond to queries.
- **Err**: Used to indicate errors.

## 3. Network Layer

The Network layer is the highest level, encompassing both Transport and Zenoh layers. It defines the following message types:

- Push
- Request
- Response
- ResponseFinal
- Interest
- Declare
- OAM

### Key Network Message Components:

- **Push**: Used to push data to the network, including wire expression, QoS, timestamp, and payload.
- **Request**: Used to make requests, including request ID, wire expression, and various extensions.
- **Response**: Used to respond to requests, including request ID, wire expression, and payload.
- **Interest**: Used to express interest in certain data, including mode and options.
- **Declare**: Used for various declarations (subscribers, queryables, etc.) and undeclarations.

## 4. Declarations

The protocol supports various declarations and undeclarations:

- DeclareKeyExpr / UndeclareKeyExpr
- DeclareSubscriber / UndeclareSubscriber
- DeclareQueryable / UndeclareQueryable
- DeclareToken / UndeclareToken

These are used to manage resources and subscriptions within the Zenoh network.

## 5. Common Components

Throughout the protocol, several common components are used:

- WireExpr: Wire expressions for efficient routing
- QoS: Quality of Service parameters
- Timestamp: Timing information
- NodeId: Identification for nodes in the network
- Various extension types (ext_qos, ext_auth, ext_mlink, etc.)

## Conclusion

The Zenoh protocol is designed with a layered approach, starting from the Transport layer, moving up to the Zenoh layer, and finally to the Network layer. It provides mechanisms for efficient data sharing, querying, and resource management in distributed systems. The protocol is flexible, with various extensions and options available at different levels to cater to diverse use cases and requirements.
