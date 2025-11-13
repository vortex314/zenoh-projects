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

# sub projects
- actix-web : 
- akka
- doc : documentation about hardware 
- esp_no_std : esp32 no std rust project with embassy. NZ ( no zenoh )
- esp32-actors-rs : NZ
- esp32-dc-motor : PWM DC motor control, no capture , NZ-
- esp32-devices : Wifi MQTT rust std , NZ
- esp32-espnow-client : Wifi ESP-NOW rust std , NZ
- esp32-espnow-display : Wifi ESP-NO rust std , just show ESP-NOW events
- esp32-espnow-gateway : UART to ESP-NOW gateway
- esp32-hoverboard-rs : talk serial with hoverboard control
- esp32-idf : rust wifi mqtt NZ
- esp32-idf-dc-motor
- esp32-idf-std-motor
esp32-limero-embassy-rs
esp32-limero-std
esp32-std-motor
eventbus
hoverboard-espnow.drawio
hoverboard-linux
LICENSE-APACHE
LICENSE-MIT
limero-linux-tokio
- lwip-stm32cube : serial tcp/udp on an stm32
- mc-esp32 : multicast udp esp32
mc-web
- msg-rs : message definition and serdes 
package-lock.json
- prosty : serdes generator for C++ and rust and for JSON and CBOR based on protobuf schemas ;-)
pubsub
README.MD
rust-toolchain.toml.old
scripts
sdkconfig.defaults
serdes
stm32-udp
uip-stm32cube
vue-dashboard-app
- zenoh_on_espnow : an investigation to have zenoh in peer mode on ESP-NOW
- zenoh_proxy : serial to zenoh
- zenoh-brain : zenoh rust central brain for robot
- zenoh-broker : start zenoh broker in memory process, rust
- zenoh-camera : zenoh-pico , esp32-cam , publish images
- zenoh-dashboard-egui-rs : egui zenoh dashboard
- zenoh-esp-now : zenoh-pico to esp-now
- zenoh-esp32 : c++ zenoh-pico with OTA
- zenoh-esp32-eventbus : a eneric eventbus with typed messages ,using central generation of serializers
- zenoh-espidf_serial : zenoh-pico via serial
- zenoh-espnow
zenoh-motor-pwm
zenoh-ota
- zenoh-ps4 : PS4 controller bluetooth to zenoh
- zenoh-stm32 : Zenoh via serial on stm32
- zenoh-tiva : Zenoh via serial on lm4f120h5qr / Tiva
- zenoh-vue : zenoh configurable dashboard based on vue and gridstack.js
- zenohd : scripts to start zenohd in different modes




