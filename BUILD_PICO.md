# Building for Raspberry Pi Pico

This guide explains how to build and flash the platformer game to your Raspberry Pi Pico with a 320x240 ST7789 LCD display.

## Prerequisites

1. **Pico SDK**: Install the Raspberry Pi Pico SDK
   - Windows: Follow https://github.com/raspberrypi/pico-setup-windows
   - Set `PICO_SDK_PATH` environment variable to point to your Pico SDK installation
   - Example: `C:\Users\YourName\pico\pico-sdk`

2. **VSCode Pico Extension**: Install the "Raspberry Pi Pico" extension in VSCode
   - This provides CMake configuration and build tools for Pico projects

3. **Python Dependencies**: Install Pillow for asset conversion
   ```bash
   pip install Pillow
   ```

4. **Hardware**: 
   - Raspberry Pi Pico (RP2040)
   - ST7789-based 320x240 LCD display (e.g., Pimoroni Pico Display)
   - Wired SPI connection on pins: SPI0 (GP16-GP20)

## Project Structure

```
SYDEQuest/
├── CMakeLists.txt           # Dual-platform build configuration
├── pico_sdk_import.cmake    # Pico SDK integration
├── Desktop/                 # SDL2-based desktop implementation
├── Pico/                    # Pico-specific implementation
│   ├── main_pico.cpp
│   ├── PicoRenderer.{h,cpp}
│   ├── PicoInput.{h,cpp}
│   ├── PicoHaptics.{h,cpp}
│   ├── PicoTimer.{h,cpp}
│   └── assets/              # Auto-generated RGB565 sprite headers
├── Shared/                  # Platform-agnostic game logic
│   ├── core/                # Types, interfaces
│   └── game/                # Entities, physics, levels
├── assets/                  # Original PNG sprites
├── levels/                  # CSV level files
└── tools/                   # Asset conversion scripts
    ├── png_to_rgb565.py
    └── csv_to_c.py
```

## Building Steps

### 1. Convert Assets to Flash Headers

Run the asset conversion scripts to generate C headers for sprites and levels:

```bash
# Convert sprites to RGB565 format
python tools/png_to_rgb565.py assets/AlternatingWalk.png Pico/assets/player_sprite.h player_sprite
python tools/png_to_rgb565.py assets/FullTerrainSpriteSheet.png Pico/assets/terrain_sprite.h terrain_sprite
python tools/png_to_rgb565.py assets/IntegralSpinLeft.png Pico/assets/enemy_basic_sprite.h enemy_basic_sprite
python tools/png_to_rgb565.py assets/PencilSpinLeft.png Pico/assets/enemy_ranged_sprite.h enemy_ranged_sprite
python tools/png_to_rgb565.py assets/Energy.png Pico/assets/projectile_sprite.h projectile_sprite
python tools/png_to_rgb565.py assets/EnergyDrink.png Pico/assets/health_pack_sprite.h health_pack_sprite
python tools/png_to_rgb565.py assets/PortalSpriteSheet.png Pico/assets/portal_sprite.h portal_sprite
python tools/png_to_rgb565.py assets/PicoSprite.png Pico/assets/objective_pico_sprite.h objective_pico_sprite
python tools/png_to_rgb565.py assets/ChargerSprite.png Pico/assets/objective_charger_sprite.h objective_charger_sprite
python tools/png_to_rgb565.py assets/EnclosureSprite.png Pico/assets/objective_enclosure_sprite.h objective_enclosure_sprite
python tools/png_to_rgb565.py assets/HapticSprite.png Pico/assets/objective_haptic_sprite.h objective_haptic_sprite
python tools/png_to_rgb565.py assets/PartsSprite.png Pico/assets/objective_parts_sprite.h objective_parts_sprite
python tools/png_to_rgb565.py assets/ScreenSprite.png Pico/assets/objective_screen_sprite.h objective_screen_sprite

# Convert level data to C strings
python tools/csv_to_c.py levels/Level1.csv Pico/assets/level1_data.h level1
python tools/csv_to_c.py levels/Level2.csv Pico/assets/level2_data.h level2
```

### 2. Configure the Build with VSCode Extension

1. Open the Command Palette (Ctrl+Shift+P)
2. Run `Pico: Configure CMake`
3. Select the Pico SDK path when prompted
4. The extension will configure CMake with the correct toolchain

### 3. Build the UF2 File

Option A: Using VSCode Extension
1. Open Command Palette (Ctrl+Shift+P)
2. Run `Pico: Compile Project`
3. The `.uf2` file will be generated in `build/` directory

Option B: Using Command Line (requires arm-none-eabi-gcc in PATH)
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### 4. Flash to Pico

1. Hold BOOTSEL button on your Pico
2. Connect Pico to PC via USB (while holding BOOTSEL)
3. Pico will mount as a USB drive (e.g., `E:\` or `RPI-RP2`)
4. Copy `build/platformer.uf2` to the drive
5. Pico will automatically reboot and run the game

## Hardware Configuration

The game expects the following SPI pin configuration for the ST7789 display:

- **SPI Port**: SPI1
- **SCK (Clock)**: GP10
- **MOSI (Data)**: GP11
- **CS (Chip Select)**: GP9
- **DC (Data/Command)**: GP8
- **RST (Reset)**: GP15
- **BL (Backlight)**: GP13

These pins are defined in `Pico/PicoRenderer.cpp`. Adjust if your hardware uses different pins.

### Input Configuration

Button inputs use GPIO pins 0-7 with pull-up resistors (active-low):

| Button | Pin | Description |
|--------|-----|-------------|
| Up | GP0 | D-pad Up |
| Down | GP1 | D-pad Down |
| Left | GP2 | D-pad Left |
| Right | GP3 | D-pad Right |
| A | GP4 | Jump button |
| B | GP5 | Fire button |
| Start | GP6 | Pause/Start |
| Select | GP7 | Select/Back |

Haptic motor (optional): GP12 (PWM output)

All pin assignments can be adjusted in `Pico/PicoInput.h` and `Pico/PicoHaptics.h`.

## Memory Optimizations Applied

The Pico build includes several RAM optimizations:

1. **Fixed-Point Math**: All floats replaced with 16.16 fixed-point (`int32_t`)
2. **EntityPool**: Fixed-size object pools instead of `std::vector` (eliminates heap fragmentation)
3. **Static Arrays**: Level data and spawn points use fixed-size arrays
4. **Bitfield Packing**: Boolean flags packed into `uint8_t` bitfields
5. **Entity Culling**: Enemies/objects only update when near player (configurable distance)
6. **Flash Storage**: All sprites and level data stored in Flash, not RAM

### Entity Limits

- Projectiles: 40
- Basic Enemies: 25
- Ranged Enemies: 20
- Enemy Projectiles: 60
- Health Packs: 1
- Objectives: 1
- Portals: 8

### Heap Configuration

- Heap size: 256KB (`PICO_HEAP_SIZE=0x40000`)
- Configured in `CMakeLists.txt`

## Debugging

Enable USB serial output for debugging:

```c
stdio_init_all();  // Already in main_pico.cpp
printf("Debug message\n");
```

Connect a serial terminal (115200 baud) to view debug output:
- Windows: PuTTY, TeraTerm
- Linux/Mac: `screen /dev/ttyACM0 115200`

## Troubleshooting

### Build Errors

**Error: "Pimoroni Pico Graphics not found"**
- CMake will automatically fetch the library via `FetchContent`
- Ensure internet connection is available during first build

**Error: "PICO_SDK_PATH not set"**
- Set environment variable: `setx PICO_SDK_PATH "C:\path\to\pico-sdk"`
- Restart terminal/VSCode after setting

**Error: "arm-none-eabi-gcc not found"**
- Install ARM GCC toolchain from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- Add to PATH or let VSCode extension handle it

### Runtime Issues

**Display shows nothing**
- Verify SPI pin connections match `PicoRenderer.cpp`
- Check backlight is enabled (GP20 high)
- Verify display is ST7789 320x240

**Game runs slowly**
- Reduce `ACTIVATION_DISTANCE` in `Shared/game/Game.h`
- Lower entity limits in `Game.h`
- Disable USB stdio if not debugging

**Out of memory crashes**
- Reduce entity limits
- Check asset sizes (large spritesheets use more Flash)
- Profile with `picotool info -a build/platformer.elf`

## Performance Tuning

Adjust these constants in `Shared/game/Game.h`:

```cpp
#define ACTIVATION_DISTANCE TO_FIXED(400)    // Lower for fewer active enemies
#define DEACTIVATION_DISTANCE TO_FIXED(450)  // Hysteresis to prevent flickering
```

Monitor frame time using `PicoTimer` and adjust game logic or reduce entity counts as needed.
