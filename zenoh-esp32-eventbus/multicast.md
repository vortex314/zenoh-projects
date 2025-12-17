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
### Dst and Src field 

<device>/<component>/<message_type> 

- Example : esp1/hb/HoverboardCmd as dst or esp1/hb/HoverboardEvent as src for event

