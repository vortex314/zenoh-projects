cmake_minimum_required(VERSION 3.13.0)
project(test_local VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g ")

find_package(OpenSSL REQUIRED)
add_definitions(-DLINUX -std=c++17)
add_executable(test_local)

target_sources(test_local PRIVATE 
    src/main.cpp 
 #   ../mc-esp32/include/value.h
    ../mc-esp32/src/value.cpp
    ) 

# added paho.mqtt.c/src for paho-mqtt-c header files
include_directories(src  
    include 
    ../mc-esp32/include
    )

#Where to find the lib "libpaho-mqtt3as.a", libssl and libcrypto
# link_directories(${CMAKE_SOURCE_DIR}/paho.mqtt.c/src /usr/local/lib)

# added libpaho-mqtt3as.a, libssl.a and libcrypto.a as we build in static mode
target_link_libraries(test_local 
    -lpthread 
    -pthread 
 )