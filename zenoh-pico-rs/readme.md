# The impossible task of creating a rust wrapper for zenoh-pico on ESP32

## Elements involved
- zenoh-pico github 
- esp-idf
- sdkconfig => sdkconfig.h
- sdkconfig.h included in FreeRTOSConfig.h
- all zenoh-pico settings
- all menuconfig/sdkconfig settings
- build.rs that should handle C sources build and generate a wrapper
- all the include files that differ per ESP32 variant specified in build.rs

## Approach 1 - rebuild zenoh-pico with CC

## Approach 2 - rebuild zenoh-pico using esp-idf components

## Approach 3 - using platformio 