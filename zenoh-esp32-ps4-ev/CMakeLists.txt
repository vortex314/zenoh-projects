cmake_minimum_required(VERSION 3.16.0)

set(BLUEPAD32_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.pio/libdeps/ps4/bluepad32)
set(BTSTACK_ROOT ${BLUEPAD32_ROOT}/external/btstack)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS ${BTSTACK_ROOT}/port/esp32/components
                         ${BLUEPAD32_ROOT}/src/components
)

project(ps4)

#  list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/workspace/bluepad32/src/components )
#  list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/btstack)
# list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/bt)
# list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/workspace/bluepad32/src/components)
# list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/btstack)
# list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/bt)
# list(APPEND EXTRA_COMPONENT_DIRS .pio/libdeps/ps4/bluepad32/external/btstack/port/esp32/components )
# list(APPEND EXTRA_COMPONENT_DIRS .pio/libdeps/ps4/bluepad32/src/components)

