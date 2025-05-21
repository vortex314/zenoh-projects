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

# Answer

I'll create a CDDL (Concise Data Definition Language) declaration for the Zenoh protocol's declaration, interest, and liveliness messages based on the codebase.

## CDDL for Declare Messages

```cddl
; Declare message structure
declare = {
  ? interest_id => uint32,         ; Optional interest ID if responding to an Interest
  ext_qos => uint64,               ; QoS extension
  ? ext_tstamp => bstr,            ; Optional timestamp extension
  ext_nodeid => uint64,            ; Node ID extension
  body => declare-body             ; The actual declaration body
}

; Different types of declaration bodies
declare-body = (
  declare-keyexpr //
  declare-subscriber //
  declare-queryable //
  declare-token //
  undeclare-keyexpr //
  undeclare-subscriber //
  undeclare-queryable //
  undeclare-token //
  declare-final
)

; Token declaration
declare-token = {
  id => uint32,                    ; Token identifier
  wire_expr => wire-expr           ; Wire expression for the token
}

; Token undeclaration
undeclare-token = {
  id => uint32,                    ; Token identifier to undeclare
  ? wire_expr => wire-expr         ; Optional wire expression
}

; Wire expression structure
wire-expr = {
  scope => uint16,                 ; Scope of the expression
  ? suffix => bstr                 ; Optional suffix
}
``` [1](#1-0) 

## CDDL for Interest Messages

```cddl
; Interest message structure
interest = {
  id => uint32,                    ; Interest identifier
  options => interest-options,     ; Interest options
  ? wire_expr => wire-expr,        ; Optional wire expression (if restricted)
  ? ext_qos => uint64,             ; Optional QoS extension
  ? ext_tstamp => bstr,            ; Optional timestamp extension
  ext_nodeid => uint64             ; Node ID extension
}

; Interest options
interest-options = {
  mode => uint2,                   ; 0=Final, 1=Current, 2=Future, 3=CurrentFuture
  aggregate => bool,               ; If replies should be aggregated
  mapping => bool,                 ; Mapping ownership (sender/receiver)
  has_suffix => bool,              ; If key expression has suffix
  restricted => bool,              ; If interest is restricted to matching key expr
  tokens => bool,                  ; If interest refers to tokens
  subscribers => bool,             ; If interest refers to subscribers
  queryables => bool,              ; If interest refers to queryables
  key_exprs => bool                ; If interest refers to key expressions
}
``` [2](#1-1) 

## CDDL for Liveliness Messages

Liveliness in Zenoh is implemented using tokens, which are declared and undeclared using the declare/undeclare token messages. Liveliness queries use the interest mechanism.

```cddl
; Liveliness token declaration (same as declare-token above)
liveliness-token = {
  id => uint32,                    ; Token identifier
  wire_expr => wire-expr           ; Wire expression for the token
}

; Liveliness token undeclaration (same as undeclare-token above)
liveliness-token-undeclare = {
  id => uint32,                    ; Token identifier to undeclare
  ? wire_expr => wire-expr         ; Optional wire expression
}

; Liveliness query (using interest with tokens option)
liveliness-query = {
  id => uint32,                    ; Interest identifier
  options => {
    mode => 1,                     ; Current mode
    tokens => true,                ; Interest refers to tokens
    ; Other options as needed
  },
  wire_expr => wire-expr,          ; Wire expression to query
  ; Other fields as needed
}

; Liveliness subscription (using interest with tokens option)
liveliness-subscription = {
  id => uint32,                    ; Interest identifier
  options => {
    mode => 2 / 3,                 ; Future or CurrentFuture mode
    tokens => true,                ; Interest refers to tokens
    ; Other options as needed
  },
  wire_expr => wire-expr,          ; Wire expression to subscribe to
  ; Other fields as needed
}
``` [3](#1-2) 

## Notes

This CDDL representation is derived from the Zenoh protocol implementation in the codebase. The actual wire format may include additional flags and encoding details not fully captured in this CDDL. The protocol uses a binary encoding with specific bit flags and field layouts as shown in the code comments.

The liveliness functionality in Zenoh leverages the existing declaration and interest mechanisms, with specific options set to indicate liveliness-related operations. Liveliness tokens are declared and undeclared using the standard token declaration messages, while liveliness queries and subscriptions use interest messages with the tokens option enabled.

Wiki pages you might want to explore:
- [Session API (eclipse-zenoh/zenoh)](/wiki/eclipse-zenoh/zenoh#2.1)