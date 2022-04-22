cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(vga C CXX ASM)

pico_sdk_init()

include_directories(vga xprintf/src)

add_executable(vga main.c usb.cpp vga.c lib.c font.c xprintf/src/xprintf.c)

pico_generate_pio_header(vga ${CMAKE_CURRENT_LIST_DIR}/usb.pio)
pico_generate_pio_header(vga ${CMAKE_CURRENT_LIST_DIR}/vga.pio)

pico_enable_stdio_uart(vga 0)
pico_enable_stdio_usb(vga 1)

target_link_libraries(vga pico_stdlib hardware_dma hardware_pio) 

pico_add_extra_outputs(vga)

