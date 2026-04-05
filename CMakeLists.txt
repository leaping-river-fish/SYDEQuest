# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

# Detect platform based on Pico SDK presence (set by VSCode extension)
if(NOT DEFINED TARGET_PLATFORM)
    if(DEFINED PICO_SDK_PATH)
        set(TARGET_PLATFORM "pico")
    else()
        set(TARGET_PLATFORM "desktop")
    endif()
endif()

if(TARGET_PLATFORM STREQUAL "pico")
    include(pico_sdk_import.cmake)
endif()

project(platformer CXX C ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Initialize Pico SDK early (required for VSCode extension)
if(TARGET_PLATFORM STREQUAL "pico")
    pico_sdk_init()
endif()

# ---------------------------
# Shared sources (platform-independent game logic)
# ---------------------------
set(SHARED_SOURCES
    Shared/game/Game.cpp
    Shared/game/player/Player.cpp
    Shared/game/player/PlayerController.cpp
    Shared/game/projectile/Projectile.cpp
    Shared/game/enemy/BasicEnemy.cpp
    Shared/game/enemy/RangedEnemy.cpp
    Shared/game/enemy/BossEnemy.cpp
    Shared/game/laser/Laser.cpp
    Shared/game/enemy/EnemyProjectile.cpp
    Shared/game/healthpack/HealthPack.cpp
    Shared/game/objective/Objective.cpp
    Shared/game/physics/Physics.cpp
    Shared/game/physics/Collision.cpp
    Shared/game/level/Level.cpp
    Shared/game/level/Camera.cpp
)

# ---------------------------
# Desktop sources (SDL2 implementation)
# ---------------------------
set(DESKTOP_SOURCES
    Desktop/main_desktop.cpp
    Desktop/DesktopRenderer.cpp
    Desktop/DesktopInput.cpp
    Desktop/DesktopHaptics.cpp
    Desktop/DesktopTimer.cpp
)

# ---------------------------
# Pico sources (Pico SDK implementation)
# ---------------------------
set(PICO_SOURCES
    Pico/main_pico.cpp
    Pico/PicoRenderer.cpp
    Pico/PicoInput.cpp
    Pico/PicoHaptics.cpp
    Pico/PicoTimer.cpp
)

# ---------------------------
# Build targets
# ---------------------------
if(TARGET_PLATFORM STREQUAL "desktop")
    # Desktop build with SDL2
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    
    add_executable(platformer ${SHARED_SOURCES} ${DESKTOP_SOURCES})
    
    target_include_directories(platformer PRIVATE 
        Shared 
        ${SDL2_INCLUDE_DIRS}
    )
    
    target_link_libraries(platformer PRIVATE 
        ${SDL2_LIBRARIES} 
        SDL2_image::SDL2_image 
        SDL2_ttf::SDL2_ttf
    )
    
elseif(TARGET_PLATFORM STREQUAL "pico")
    # Pico build (VSCode extension compatible)
    add_subdirectory(third_party/ST7789_Pico)
    
    add_executable(platformer ${SHARED_SOURCES} ${PICO_SOURCES})
    
    target_include_directories(platformer PRIVATE 
        Shared
        Pico
    )
    
    # GAME_TRACE is intentionally omitted: UART printf in Game::update can block on USB backpressure.
    # To debug freezes, add: GAME_TRACE (e.g. target_compile_definitions(... GAME_TRACE))
    target_compile_definitions(platformer PRIVATE 
        PLATFORM_PICO
        PICO_HEAP_SIZE=0x14000
    )
    
    # Link Pico SDK libraries + ST7789 display driver (st7789_lib pulls hardware_irq for DMA)
    target_link_libraries(platformer 
        pico_stdlib
        hardware_spi
        hardware_gpio
        hardware_pwm
        hardware_dma
        hardware_adc
        hardware_i2c
        st7789_lib
    )
    
    # Enable USB serial output (optional, for debugging)
    pico_enable_stdio_usb(platformer 1)
    pico_enable_stdio_uart(platformer 0)
    
    # Generate UF2 file for drag-and-drop to Pico
    pico_add_extra_outputs(platformer)
    
    # Asset generation (optional - can be run manually)
    # Uncomment when assets need to be embedded
    # set(ASSET_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    # set(ASSET_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pico/assets)
    # ... add custom commands here ...
endif()

