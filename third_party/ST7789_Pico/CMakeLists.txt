# ST7789_Pico driver (vendored from https://github.com/musicaJack/ST7789_Pico) — library only, no demos
cmake_minimum_required(VERSION 3.13)

add_library(st7789_lib STATIC
    src/st7789.cpp
    src/st7789_hal.cpp
    src/st7789_gfx.cpp
    src/st7789_font.cpp
)

target_include_directories(st7789_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(st7789_lib PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_dma
    hardware_irq
)
