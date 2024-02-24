cmake_minimum_required(VERSION 3.13.0)
project(z_pub VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g ")
set(ZENOH_PICO_INCLUDE_DIRS /usr/local/include)

find_package(zenohpico REQUIRED)
# add_definitions(-DLINUX -std=c++11)
add_executable(z_pub)

target_sources(z_pub PRIVATE 
    src/z_pub.c
    ) 
    
include_directories(src  
    ${ZENOH_PICO_INCLUDE_DIRS}
    )

target_link_libraries(z_pub 
    -lzenohpico
    -lpthread 
    -lrt 
    -pthread 
    -latomic )
