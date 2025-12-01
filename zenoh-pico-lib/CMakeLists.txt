
cmake_minimum_required(VERSION 3.20)
set(IDF_PATH $ENV{IDF_PATH})
set(COMP ${IDF_PATH}/components)
set (CPPDEFINES "-DZENOH_ESPIDF")

# Global options you want to preset
set(GLOBAL_INCLUDES "${COMP}/esp_driver_uart/include;inc2;inc3")
set(CMAKE_BUILD_TYPE Release   )
set(CMAKE_C_COMPILER /home/lieven/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20240530/xtensa-esp-elf/bin/xtensa-esp32-elf-gcc)
set(CMAKE_SYSTEM_NAME Generic )
set(INCLUDE_PATHS "$COMP/esp_driver_uart/include;$COMP/esp_common/include;$COMP/freertos/include;$COMP/lwip/lwip/src/include/lwip;$COMP/newlib/platform_include;$COMP/soc/include;$COMP/driver/include;$COMP/esp_ringbuf/include;$COMP/esp_event/include;$COMP/heap/include;$COMP/log/include;$COMP/xtensa/include;$COMP/xtensa/esp32/include" )
set(CMAKE_C_FLAGS "${CPPDEFINES}" )   


# ESP-IDF specific settings for zenoh-pico
option(BUILD_SHARED_LIBS "Build shared libraries if ON, otherwise build static libraries" OFF)
set(ZENOH_ESPIDF ON CACHE BOOL "Build for ESP-IDF" FORCE)

project(zenoh-pico C CXX ASM)

# Bring in the real project

file(GLOB_RECURSE SOURCES 
    "zenoh-pico/src/api/*.c"
    "zenoh-pico/src/collections/*.c"
    "zenoh-pico/src/link/*.c"
    "zenoh-pico/src/net/*.c"
    "zenoh-pico/src/protocol/*.c"
    "zenoh-pico/src/session/*.c"
    "zenoh-pico/src/system/espidf/*.c"
    "zenoh-pico/src/transport/*.c"
    "zenoh-pico/src/utils/*.c"
)

# Create a static library from zenoh-pico sources
add_library(zenohpico_lib STATIC ${SOURCES})

# Add ESP-IDF specific compile definitions
target_compile_definitions(zenohpico_lib PRIVATE
    ZENOH_ESPIDF
    ESP_PLATFORM
    IDF_VER="v5.3"
    _GNU_SOURCE
    _POSIX_READER_WRITER_LOCKS
)

# Tell zenoh-pico where ESP-IDF includes are
target_include_directories(zenohpico_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/zenoh-pico/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${COMP}/newlib/platform_include
    ${COMP}/freertos/FreeRTOS-Kernel/include
    ${COMP}/freertos/FreeRTOS-Kernel/portable/xtensa/include
    ${COMP}/freertos/esp_additions/include
    ${COMP}/freertos/config/xtensa/include
    ${COMP}/esp_common/include
    ${COMP}/esp_hw_support/include
    ${COMP}/esp_system/include
    ${COMP}/xtensa/include
    ${COMP}/xtensa/esp32/include
    ${COMP}/soc/include
    ${COMP}/soc/esp32/include
    ${COMP}/hal/include
    ${COMP}/hal/esp32/include
    ${COMP}/esp_rom/include
    ${COMP}/esp_rom/include/esp32
    ${COMP}/driver/include
    ${COMP}/esp_driver_uart/include
    ${COMP}/lwip/lwip/src/include
    ${COMP}/lwip/port/include
    ${COMP}/esp_ringbuf/include
    ${COMP}/esp_event/include
    ${COMP}/heap/include
    ${COMP}/log/include
    ${COMP}/heap/include
    ${COMP}/log/include
    ${COMP}/xtensa/include
    ${COMP}/xtensa/esp32/include
    config
    ${CMAKE_CURRENT_SOURCE_DIR}/zenoh-pico/include
)

# Link ESP-IDF libs to satisfy dependencies
target_link_libraries(zenohpico_lib
    idf::freertos
    idf::esp_timer
    idf::esp_common
    idf::driver
)