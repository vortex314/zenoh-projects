# Adapting Zenoh Pico for a new platform

## Adapt zenoh-pico/system/common/platform.h
- force include of an external 
```
#else
#include "generic.h"
//#error "Unknown platform"
#endif
```
## Adapt include/zenoh-pico/config.h 
- manual change config.h,isn't there a better way ?
```
/* #undef Z_FEATURE_UNSTABLE_API */
#define Z_FEATURE_MULTI_THREAD 1
#define Z_FEATURE_PUBLICATION 1
#define Z_FEATURE_SUBSCRIPTION 1
#define Z_FEATURE_QUERY 1
#define Z_FEATURE_QUERYABLE 1
#define Z_FEATURE_LIVELINESS 1
#define Z_FEATURE_RAWETH_TRANSPORT 0
#define Z_FEATURE_INTEREST 1
#define Z_FEATURE_LINK_TCP 0
#define Z_FEATURE_LINK_BLUETOOTH 0
#define Z_FEATURE_LINK_WS 0
#define Z_FEATURE_LINK_SERIAL 0
#define Z_FEATURE_LINK_SERIAL_USB 0
#define Z_FEATURE_SCOUTING_UDP 0
#define Z_FEATURE_LINK_UDP_MULTICAST 0
#define Z_FEATURE_LINK_UDP_UNICAST 0
#define Z_FEATURE_MULTICAST_TRANSPORT 0
#define Z_FEATURE_UNICAST_TRANSPORT 0
#define Z_FEATURE_FRAGMENTATION 1
#define Z_FEATURE_ENCODING_VALUES 1
#define Z_FEATURE_TCP_NODELAY 0
#define Z_FEATURE_LOCAL_SUBSCRIBER 0
#define Z_FEATURE_SESSION_CHECK 1
#define Z_FEATURE_BATCHING 1
#define Z_FEATURE_MATCHING 1
#define Z_FEATURE_RX_CACHE 0
#define Z_FEATURE_AUTO_RECONNECT 1
```

## Adapt extra_script.py

- adapt SRC_FILTER to avoid all other platforms if an unknown one is selected 
```
else:
    SRC_FILTER = ["+<*>",
                    "-<tests/>",
                    "-<example/>",
                    "-<system/arduino/esp32>",
                    "-<system/arduino/opencr>",
                    "-<system/emscripten/>",
                    "-<system/espidf>",
                    "-<system/freertos_plus_tcp/>",
                    "-<system/rpi_pico/>",
                    "-<system/mbed/>",
                    "-<system/unix/>",
                    "-<system/flipper/>",
                    "-<system/windows/>",
                    "-<system/zephyr/>"]
    CPPDEFINES = ["ZENOH_ARDUINO_STM32", "ZENOH_C_STANDARD=99", "Z_FEATURE_MULTI_THREAD=0"]
```
## Adapt build script to include a extra include generic.h 
```
[env:stm32_stm32cube]
platform = ststm32
board = nucleo_f401re
framework = stm32cube
monitor_speed = 115200

lib_deps = 
	lib/zenoh-pico
	lib/stm32_stm32cube

build_flags = 
	-D__GLIBCXX_ASSERTIONS
	-DZENOH_GENERIC
	-DZENOH_DEBUG=3
	-std=gnu++17
	-Wno-error=deprecated-declarations
	-Ilib/stm32_stm32cube
	-g
;	-Os

build_unflags = 
	-std=gnu++11
```

# Adapt 
https://controllerstech.com/stm32-uart-1-configure-uart-transmit-data/

## References 
- [Description of STM32F4 HAL and low-layer drivers](https://www.st.com/content/ccc/resource/technical/document/user_manual/2f/71/ba/b8/75/54/47/cf/DM00105879.pdf/files/DM00105879.pdf/jcr:content/translations/en.DM00105879.pdf)
- [Description of STM32F37xx/38xx Standard Peripheral Library](https://www.st.com/resource/en/user_manual/um1565-description-of-stm32f37xx38xx-standard-peripheral-library-stmicroelectronics.pdf)