cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(platformer)

pico_sdk_init()

add_executable(platformer
    main.cpp
)

target_link_libraries(platformer pico_stdlib)

pico_add_extra_outputs(platformer)