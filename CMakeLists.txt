# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(PROJECT_VER "1.9.0")

set(EXTRA_COMPONENT_DIRS "components/BSP" "components/MiddleWares")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(P_ESP32_Terminal_HA)
