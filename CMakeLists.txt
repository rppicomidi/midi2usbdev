cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)

project(midi2usbdev C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/midi_uart_lib)

add_executable(midi2usbdev
    midi2usbdev.c
    usb_descriptors.c
)

pico_enable_stdio_uart(midi2usbdev 1)

target_include_directories(midi2usbdev PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(midi2usbdev midi_uart_lib tinyusb_device tinyusb_board ring_buffer_lib pico_stdlib)

pico_add_extra_outputs(midi2usbdev)

