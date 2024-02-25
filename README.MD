# An evaluation project of Zenoh 
## Features
Zenoh is feature rich and would permit my own design to be much more efficient
- datat routing based
- timestamping
- discovery
- query
- speed
- optimized protocol

Looks good on paper, letś go.

## Observations
- Memory size of executable is comparable to Redis ( which is my alternative )
- Should support shared memory

## Regrets
### Serial doesn´t work or is not fully implemented yet.
See zenoh-serial README.MD
Do I again implement a proxy ?

### Redis implementation stack
- redis server
- web socket proxy
- serial proxy 
- udp proxy
### Zenoh stack
- zenohd
- storage plugin for static data
- serial proxy


