# The impossible task of creating a rust wrapper for zenoh-pico on ESP32
# ----------------------------- I give up ! ------------------------------

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

- clone zenoh-pico
```
option(BUILD_SHARED_LIBS "Build shared libraries if ON, otherwise build static libraries" OFF)
```

```sh
set -v
mkdir build
cd build
rm -rf ../build/*
export CPPDEFINES="-DZENOH_ESPIDF"
echo $CPPDEFINES

cmake -B . -S ..  -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=/home/lieven/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-gcc \
    -DCMAKE_SYSTEM_NAME=Generic \
    -DCMAKE_C_FLAGS="-DZENOH_ESPIDF -mlongcalls -Wno-frame-address -Wno-implicit-function-declaration -Wno-format -Wno-unused-function -Wno-unused-variable -Wno-return-type -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast $CPPDEFINES" 
make VERBOSE=1 -j1
```
set -v
mkdir build
cd build
rm -rf ../build/*
export CPPDEFINES="-DZENOH_ESPIDF"
echo $CPPDEFINES

set(ESP_INCLUDES "/home/lieven/.espressif/components")

cmake -B . -S ..  -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=/home/lieven/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-gcc \
    -DCMAKE_SYSTEM_NAME=Generic \
    -DCMAKE_C_FLAGS="-DZENOH_ESPIDF -mlongcalls -Wno-frame-address -Wno-implicit-function-declaration -Wno-format -Wno-unused-function -Wno-unused-variable -Wno-return-type -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast $CPPDEFINES" 
make VERBOSE=1 -j1

## Approach 2 - rebuild zenoh-pico using esp-idf components

## Approach 3 - using platformio 