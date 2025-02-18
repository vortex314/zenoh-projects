# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources} PRIV_INCLUDE_DIRS .
    PRIV_REQUIRES nvs_flash esp_psram esp_driver_i2c )
