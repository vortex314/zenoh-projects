# Adapting Zenoh Pico for a new platform
## Target vanille Arduino 
- Just communication via serial

## changes
platform.h
```
#elif defined(ZENOH_ARDUINO)
#include "zenoh-pico/system/platform/arduino.h"
```

manual change config.h
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

# Adapt extra_script.py
