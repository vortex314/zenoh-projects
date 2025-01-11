cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/workspace/bluepad32/src/components )
 list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/btstack)
# list(APPEND EXTRA_COMPONENT_DIRS /home/lieven/esp/esp-idf/components/bt)

project(zenoh-lib)
