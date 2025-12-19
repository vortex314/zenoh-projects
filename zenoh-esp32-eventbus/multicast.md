# Multicast approach

## Rationale

After some bugs with zenoh-pico, I decided to try something more simple and straight forward - UDP for Request/Reply and UDP Multicast for Events

## Event and Req/Reply structure

### Message Layout 
- CBOR encoded
- [ dst , src , payload ] 
- dst and src can be null 
- dst = null when event is send
- dst and src are filled in for request / reply 

| dst   |  src  | payload |
|. -    |   -   |.  ....  | 
|  X    |  -    |  ....   | => 
| -     |  X    |  ....   | => Publish event
|  X    |  X    | .....   |=>  Request or reply

### Dst and Src field 

<device>/<component>/<message_type> 

- Example : esp1/hb/HoverboardCmd as dst or esp1/hb/HoverboardEvent as src for event

# Bad surprise
- apparently mukticast protocol is transmitted on the lowest bandwidth ( 1Mbps )
- while UDP unicast goes at 54Mbps
## conclusion
- The multicast should be used for scouting/discovery or non-timecritical events
### classification of messages
- announce : each component publishes its characteristics on multicast
- events : are send to a logical device , can be found via mdns or hard-coded IP
- request/reply : go via unicast UDP
- Example : compass -event-> brain <-request/reply-> drive
