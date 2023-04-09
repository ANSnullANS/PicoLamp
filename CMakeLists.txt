cmake_minimum_required(VERSION 3.13)
set(PICO_SDK_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)
project(PicoLamp C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico)

pico_sdk_init()

add_subdirectory(/home/paco/Development/pi-pico-adafruit-neopixels/pico_neopixels build)

set(FS_DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/fsdata)
set(CMAKE_BUILD_TYPE Debug)

add_executable(PicoLamp
    PicoLamp.cpp
    Rainbow.cpp
)

target_include_directories(PicoLamp PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
)

pico_enable_stdio_usb(PicoLamp 1)
pico_enable_stdio_uart(PicoLamp 1)

target_link_libraries(PicoLamp 
    pico_stdlib
    pico_stdio
    pico_multicore
    pico_neopixel
    pico_unique_id
    pico_bootsel_via_double_reset
)

pico_add_extra_outputs(PicoLamp)