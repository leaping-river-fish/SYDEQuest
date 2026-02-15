cmake_minimum_required(VERSION 3.13)

# Detect platform
if(DEFINED PICO_SDK_PATH)
    set(TARGET_PLATFORM "pico")
    include(pico_sdk_import.cmake)
else()
    set(TARGET_PLATFORM "desktop")
endif()

project(platformer CXX)
set(CMAKE_CXX_STANDARD 17)

# Game sources (platform-independent)
set(GAME_SOURCES
    src/game/Game.cpp
    src/game/player/Player.cpp
    src/game/player/PlayerController.cpp
    src/game/physics/Physics.cpp
    src/game/physics/Collision.cpp
    src/game/level/Level.cpp
    src/game/level/Camera.cpp
)

if(TARGET_PLATFORM STREQUAL "desktop")
    # Desktop build
    find_package(SDL2 REQUIRED)
    
    add_executable(platformer
        ${GAME_SOURCES}
        src/platform/desktop/main_desktop.cpp
        src/platform/desktop/DesktopRenderer.cpp
        src/platform/desktop/DesktopInput.cpp
        src/platform/desktop/DesktopHaptics.cpp
        src/platform/desktop/DesktopTimer.cpp
    )
    
    target_include_directories(platformer PRIVATE 
        src 
        ${SDL2_INCLUDE_DIRS}
    )
    
    target_link_libraries(platformer ${SDL2_LIBRARIES})
    
elseif(TARGET_PLATFORM STREQUAL "pico")
    # Pico build (future)
    pico_sdk_init()
    
    add_executable(platformer
        ${GAME_SOURCES}
        src/platform/pico/main_pico.cpp
        # ... other pico sources
    )
    
    target_include_directories(platformer PRIVATE src)
    target_link_libraries(platformer pico_stdlib)
    pico_add_extra_outputs(platformer)
endif()