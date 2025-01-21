# Lawn-mower robotics - Zenoh integration of micro-services

![Architecture](doc/design-architecture.drawio.png)

# Features
- The robot is composed of micro-services which communicate on a local bus via pub/sub
- The local bus can be a I2C,serial,USB or ESP-NOW, TCP, UDP,
- the protocol supports device discovery
- every device has a unique text id 
- every device has properties which have a sequential id : 0... N or a field index
- the device will broadcast or answer queries on the specifics of a property
- with ESP-NOW there is no central broker , the broadcast is sufficient for now to support the subscribe and publish features. 
- target is to use esp-now as another peer-to-peer protocol 
- a bridge will provide a gateway to the rest of the network to support dashboards, logging and a central brain
- The services themselves will be a composition of actors 
- The actors will response to ActorCmd and generate ActorEvent 
- ActorEvent will be handled by non-blocking services
- Each Actor should be independent and not aware of others
- The inter-actor glue is provided in the main code

# consumption ready
- zenoh-ps4 : [Ps4](zenoh-ps4) a bridge from bluetooth PS4 events to zenoh and back.


## Features
Zenoh is feature rich and would permit my own design to be much more efficient
- data routing based
- timestamping
- discovery
- query
- speed
- optimized protocol

Looks good on paper, letś go.

## Observations
- Memory size of executable is comparable to Redis ( which is my alternative )
- Should support shared memory 




