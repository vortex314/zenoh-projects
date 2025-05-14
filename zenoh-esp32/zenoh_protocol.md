# CDDL specs of Zenoh protocol
Thanks to Devin : https://deepwiki.com/eclipse-zenoh/zenoh/7-protocol-details

I'll create a comprehensive CDDL (Concise Data Definition Language) schema for the specific message types you requested from the Zenoh080 codec.

Answer
Based on the codebase, I'll provide detailed CDDL schemas for the eight message types you requested: hello-message, scout-message, frame-message, push-message, request-message, response-message, close-message, and keepalive-message.

## Basic Types and Common Structures
First, let's define the basic types and common structures used across all message types:
```cddl
; Basic types  
zint = uint       ; Variable-length encoded integer  
zbytes = bytes    ; Length-prefixed byte array  
zstring = text    ; Length-prefixed UTF-8 string  
  
; Common structures  
zenoh-id = bytes .size (1..16)  ; ZenohIdProto  
  
; Header byte structure (common to all messages)  
header = uint .bits header-flags  ; Single byte with flags in upper bits
```
## Scout Message
The Scout message is used for discovering other Zenoh nodes on the network:
```
; Scout message  
scout-message = {  
  header: uint .bits scout-flags,  
  version: uint,  
  flags: uint,  ; Contains whatami and zid info  
  ? zid: zenoh-id,  ; Present if I flag is set  
  ? extensions: [+ extension]  ; Present if Z flag is set  
}  
  
scout-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  ; Lower 5 bits = 0x01 (SCOUT)  
)
```
The Scout message structure is defined in scout.rs:74-79 with fields for version, WhatAmIMatcher, and an optional ZenohIdProto.

## Hello Message
The Hello message is sent in response to a Scout message to advertise a node's presence:
```cddl
; Hello message  
hello-message = {  
  header: uint .bits hello-flags,  
  version: uint,  
  flags: uint,  ; Contains whatami and zid info  
  zid: zenoh-id,  
  ? locators: [* zstring],  ; Present if L flag is set  
  ? extensions: [+ extension]  ; Present if Z flag is set  
}  
  
hello-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  L: 0x20,  ; Locators flag (0b00100000)  
  ; Lower 5 bits = 0x02 (HELLO)  
)
The Hello message implementation can be seen in hello.rs:37-43 , which shows the structure with version, whatami, zid, and locators fields.
```

## Frame Message
The Frame message is used to transport network messages:
```cddl
; Frame message  
frame-message = {  
  header: uint .bits frame-flags,  
  sn: zint,  ; Sequence number  
  ? ext-qos: qos-extension,  ; Present if Z flag is set  
  payload: zbytes  ; Contains network messages  
}  
  
frame-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  R: 0x40,  ; Reliability flag (0b01000000)  
  ; Lower 5 bits = 0x05 (FRAME)  
)  
  
qos-extension = {  
  priority: uint,  ; Priority level  
  congestion_control: uint  ; Congestion control mode  
}
```
The Frame message encoding is shown in , where we can see how the header flags are set based on reliability and QoS extensions.

## Close Message
The Close message is used to terminate connections:
```cddl
; Close message  
close-message = {  
  header: uint .bits close-flags,  
  reason: uint,  ; Reason code  
  ? extensions: [+ extension]  ; Present if Z flag is set  
}  
  
close-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  S: 0x20,  ; Session flag (0b00100000)  
  ; Lower 5 bits = 0x03 (CLOSE)  
)
```
The Close message structure is defined in close.rs:84-87 with fields for reason and session.

## KeepAlive Message
The KeepAlive message is the simplest, used to maintain connections:
```cddl
; KeepAlive message  
keepalive-message = {  
  header: uint .bits keepalive-flags,  
  ? extensions: [+ extension]  ; Present if Z flag is set  
}  
  
keepalive-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  ; Lower 5 bits = 0x04 (KEEP_ALIVE)  
)
```
The KeepAlive message is defined in keepalive.rs:84-85 and is notably empty (no fields beyond the header).

## Push Message
The Push message is used for data publication:
```cddl
; Push message  
push-message = {  
  header: uint .bits push-flags,  
  wire-expr: wire-expr,  
  ? timestamp: timestamp,  ; Present if T flag is set  
  encoding: encoding,  
  ? ext-attachment: zbytes,  ; Present if extension  
  ? ext-consolidation: consolidation,  ; Present if extension  
  payload: zbytes  
}  
  
push-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  T: 0x40,  ; Timestamp flag (0b01000000)  
  ; Lower 5 bits = 0x00 (PUSH)  
)  
  
wire-expr = {  
  scope: zint,  ; Scope ID (0 for global scope)  
  suffix: zstring,  ; Expression suffix  
  mapping: uint  ; Mapping type  
}  
  
timestamp = {  
  time: zint,  ; Time value  
  zid: zenoh-id  ; Source ID  
}  
  
encoding = {  
  id: zint,  ; Encoding ID  
  ? schema: zbytes  ; Optional schema information  
}  
  
consolidation = {  
  first: bool,  ; First flag  
  last: bool,  ; Last flag  
  reserved: uint  ; Reserved bits  
}
```
The Push message is part of the NetworkBody enum as seen in mod.rs:106-114 .

## Request Message
The Request message is used for queries:
```cddl
; Request message  
request-message = {  
  header: uint .bits request-flags,  
  id: zint,  ; Request ID  
  wire-expr: wire-expr,  
  ? ext-target: zint,  ; Present if extension  
  ? ext-budget: zint,  ; Present if extension  
  ? ext-timeout: zint,  ; Present if extension  
  ? ext-attachment: zbytes,  ; Present if extension  
  payload: zbytes  
}  
  
request-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  ; Lower 5 bits = 0x01 (REQUEST)  
)
```
The Request message is also part of the NetworkBody enum as shown in mod.rs:106-114 .

## Response Message
The Response message is sent in reply to requests:
```cddl
; Response message  
response-message = {  
  header: uint .bits response-flags,  
  rid: zint,  ; Request ID this is responding to  
  wire-expr: wire-expr,  
  ? timestamp: timestamp,  ; Present if T flag is set  
  encoding: encoding,  
  ? ext-attachment: zbytes,  ; Present if extension  
  ? ext-consolidation: consolidation,  ; Present if extension  
  payload: zbytes  
}  
  
response-flags = &(  
  Z: 0x80,  ; Extensions flag (0b10000000)  
  T: 0x40,  ; Timestamp flag (0b01000000)  
  ; Lower 5 bits = 0x02 (RESPONSE)  
)
```
The Response message is also part of the NetworkBody enum as shown in mod.rs:106-114 .

## Extension Mechanism
All messages can include extensions when the Z flag is set:
```cddl
; Extension mechanism  
extension = {  
  ext-header: uint,  ; Extension header with more flag and type  
  ext-payload: any   ; Extension-specific payload  
}
```
## Notes
This CDDL schema representation captures the structure of the eight requested message types in the Zenoh080 protocol. The Protocol Details wiki page provides additional context about the encoding rules and message structures.

The schema focuses on the structure of the messages rather than the exact binary encoding details (like the variable-length integer encoding). For a complete understanding of the binary format, you would need to refer to the RFC document mentioned in the code: "https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Serialization.md".

Each message type follows a similar pattern with a header byte containing flags and a message type identifier, followed by message-specific fields. The extension mechanism allows for future protocol extensions without breaking backward compatibility.

Wiki pages you might want to explore: https://deepwiki.com/eclipse-zenoh/zenoh/7-protocol-details