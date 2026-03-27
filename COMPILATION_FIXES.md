# Compilation Fixes Applied (Round 2)

This document lists all compilation errors that were encountered during the Pico build and the fixes applied.

## Critical Issues Fixed in Round 2

## Issue 1: Vec2/Rect Constructor Ambiguity

**Error**: `call of overloaded 'Vec2(int, int)' is ambiguous`

**Cause**: When using integer literals like `Vec2(100, 100)`, the compiler couldn't decide between:
- `Vec2(fixed_t, fixed_t)` constructor
- `Vec2(float, float)` constructor

**Fix**: Added explicit constructors to handle all cases:
- Added `Vec2(int, int)` constructor that converts ints to fixed_t via float
- Added `Rect(int, int, int, int)` constructor similarly
- Made the `fixed_t` constructors `explicit` to prevent unwanted conversions
- Updated all integer literal constructions to use float literals (e.g., `Vec2(100.0f, 100.0f)`)

**Files Modified**:
- `Shared/core/types.h` - Added int constructors
- `Shared/game/player/Player.cpp` - Changed `Vec2(100, 100)` to `Vec2(100.0f, 100.0f)`
- `Shared/game/level/Level.cpp` - Changed spawn point initialization
- `Shared/game/Game.cpp` - Changed UI rect constructions
- `Shared/game/objective/Objective.cpp` - Changed src rect construction

## Issue 2: Missing size_t Declaration

**Error**: `'size_t' has not been declared` in RangedEnemy.h

**Cause**: The EntityPool forward declaration used `size_t` without including `<cstddef>`

**Fix**: Added `#include <cstddef>` to RangedEnemy.h

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.h`

## Issue 3: RangedEnemy Update Method Signature Mismatch

**Error**: `no declaration matches 'void RangedEnemy::update(float, const Level&, const Player&, int)'`

**Cause**: The implementation in RangedEnemy.cpp only had the `std::vector` signature, not the conditional `EntityPool` signature for Pico builds.

**Fix**: Wrapped the method implementation with `#ifdef PLATFORM_PICO` to provide both signatures:
```cpp
#ifdef PLATFORM_PICO
void RangedEnemy::update(..., EntityPool<EnemyProjectile, 60>& projectiles) {
#else
void RangedEnemy::update(..., std::vector<EnemyProjectile>& projectiles) {
#endif
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`

## Issue 4: Missing abs() Declaration

**Error**: `'abs' was not declared in this scope` in RangedEnemy.cpp

**Cause**: Missing `<cstdlib>` include for `abs()` function

**Fix**: Added `#include <cstdlib>` and `#include <cmath>` to ensure abs() is available

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`

## Issue 5: Mixed Type Arithmetic (fixed_t with float)

**Error**: `call of overloaded 'Vec2(fixed_t, float)' is ambiguous`

**Cause**: Velocity initialization mixed `fixed_t` constants (MOVE_SPEED) with float literals (0.0f)

**Fix**: Wrapped velocity initialization in explicit Vec2 constructor:
```cpp
// Before:
velocity(startFacingRight ? MOVE_SPEED : -MOVE_SPEED, 0.0f)

// After:
velocity(Vec2(startFacingRight ? MOVE_SPEED : -MOVE_SPEED, 0.0f))
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`
- `Shared/game/enemy/BasicEnemy.cpp`

## Issue 6: EntityPool Method Access Errors

**Error**: `request for member 'size' in '((Game*)this)->Game::projectiles', which is of non-class type 'int'`

**Cause**: EntityPool initially had `.count` member but code was calling `.size()` method (matching std::vector API)

**Fix**: Added proper methods to EntityPool:
- `size()` method returning `MaxCount` (for iteration bounds)
- `clear()` method for bulk deactivation
- Ensured all methods match std::vector interface where used

**Files Modified**:
- `Shared/game/Game.h` - Updated EntityPool template
- `Shared/game/Game.cpp` - Simplified clearing to use `.clear()` method

## Issue 7: Position/Velocity Updates Not Using Fixed-Point Math

**Error**: Implicit float multiplication when `fixed_t` was expected

**Cause**: Position updates like `position.x += velocity.x * deltaTime` don't work with fixed-point - need FIXED_MUL macro

**Fix**: Wrapped all position/velocity updates with platform-specific math:
```cpp
#ifdef PLATFORM_PICO
    position.x += FIXED_MUL(velocity.x, floatToFixed(deltaTime));
#else
    position.x += velocity.x * deltaTime;
#endif
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`
- `Shared/game/enemy/BasicEnemy.cpp`

## Issue 8: Float Variables in Fixed-Point Context

**Error**: Using `float` for intermediate calculations in edge detection and platform collision

**Cause**: Methods like `checkEdgeDetection()` used `float checkX` even on Pico builds

**Fix**: Made all intermediate variables conditional:
```cpp
#ifdef PLATFORM_PICO
    fixed_t checkX = position.x + WIDTH;
    fixed_t checkY = position.y + HEIGHT + TO_FIXED(1.0f);
#else
    float checkX = position.x + WIDTH;
    float checkY = position.y + HEIGHT + 1;
#endif
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp` - checkEdgeDetection, canDetectPlayer
- `Shared/game/enemy/BasicEnemy.cpp` - checkEdgeDetection

## Issue 9: Platform Collision Tolerance Constants

**Error**: Using float literals in platform collision checks on Pico

**Fix**: Wrapped tolerance values with `TO_FIXED()`:
```cpp
#ifdef PLATFORM_PICO
    if (enemyBottom >= platformTop - TO_FIXED(1.0f) && 
        enemyBottom <= platformTop + TO_FIXED(4.0f))
#else
    if (enemyBottom >= platformTop - 1.0f && 
        enemyBottom <= platformTop + 4.0f)
#endif
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`
- `Shared/game/enemy/BasicEnemy.cpp`

## Issue 10: Distance Calculation in canDetectPlayer

**Error**: Float arithmetic for distance calculation on Pico

**Cause**: Using `float dx * dx` instead of `FIXED_MUL(dx, dx)` for squared distance

**Fix**: Separated Pico and Desktop implementations with appropriate math:
```cpp
#ifdef PLATFORM_PICO
    fixed_t dx = playerCenter.x - enemyCenter.x;
    fixed_t dy = playerCenter.y - enemyCenter.y;
    fixed_t distSquared = FIXED_MUL(dx, dx) + FIXED_MUL(dy, dy);
    fixed_t radiusSquared = FIXED_MUL(DETECTION_RADIUS, DETECTION_RADIUS);
#else
    float dx = playerCenter.x - enemyCenter.x;
    float dy = playerCenter.y - enemyCenter.y;
    float distSquared = dx * dx + dy * dy;
    float radiusSquared = DETECTION_RADIUS * DETECTION_RADIUS;
#endif
```

**Files Modified**:
- `Shared/game/enemy/RangedEnemy.cpp`

## Summary of Changes

All compilation errors have been resolved by:

1. **Type Safety**: Adding constructors to handle all input types (int, float, fixed_t)
2. **Platform Separation**: Using `#ifdef PLATFORM_PICO` for all math operations
3. **Consistent Math**: Using FIXED_MUL/TO_FIXED macros consistently on Pico
4. **API Matching**: Ensuring EntityPool matches std::vector interface where used
5. **Include Guards**: Adding missing headers (`<cstddef>`, `<cstdlib>`, `<cmath>`)

## New Issues Fixed (Round 2)

### 11. Interface Signature Mismatches

**Error**: All Pico platform classes marked as abstract, override methods not matching interface

**Cause**: PicoRenderer, PicoInput, PicoHaptics, PicoTimer implemented with wrong signatures that didn't match the actual IRenderer, IInput, IHaptics, ITimer interfaces.

**Fix**: Complete rewrite of all Pico platform class signatures to match interfaces:

**PicoRenderer**:
- Changed `clear(uint8_t, uint8_t, uint8_t)` → `clear(Color)`
- Changed `drawRect(int, int, int, int, uint8_t, uint8_t, uint8_t)` → `drawRect(const Rect&, Color, bool)`
- Changed `drawSprite(int, int, int, int, int, int, int, bool)` → `drawSprite(int, const Rect&, const Rect&, bool)`
- Changed `drawTile(int, int, int, int)` → `drawTile(int, int, int8_t, int, int, int)`
- Changed `drawText(const char*, int, int, uint8_t, uint8_t, uint8_t)` → `drawText(const char*, int, int, Color)`

**PicoInput**:
- Changed `isKeyPressed(int)` → `isPressed(Button)`
- Changed `isKeyJustPressed(int)` → `wasJustPressed(Button)`
- Added `wasJustReleased(Button)` method
- Added `buttonToBit(Button)` helper to map Button enum to bit positions

**PicoHaptics**:
- Changed `triggerRumble(float, float)` → `trigger(HapticEffect, int)`
- Added `stop()` method
- Added proper effect intensity mapping (Light/Medium/Heavy)

**PicoTimer**:
- Added missing `getTicks()` method

**Files Modified**:
- `Pico/PicoRenderer.h`, `Pico/PicoRenderer.cpp`
- `Pico/PicoInput.h`, `Pico/PicoInput.cpp`
- `Pico/PicoHaptics.h`, `Pico/PicoHaptics.cpp`
- `Pico/PicoTimer.h`, `Pico/PicoTimer.cpp`

### 12. Pimoroni Library Rect Name Conflict

**Error**: `reference to 'Rect' is ambiguous` in PicoRenderer.cpp

**Cause**: Pimoroni library has its own `Rect` type, conflicting with our `Rect` type

**Fix**: Qualified Pimoroni types with namespace:
```cpp
graphics->rectangle(pimoroni::Rect(x, y, w, h));
graphics->pixel(pimoroni::Point(x, y));
```

**Files Modified**: `Pico/PicoRenderer.cpp`

### 13. Pimoroni set_pixel API Mismatch

**Error**: `no matching function for call to 'set_pixel(pimoroni::Point, uint16_t&)'`

**Cause**: Pimoroni's `set_pixel` doesn't exist; need to use `set_pen` then `pixel`

**Fix**: Changed pixel drawing:
```cpp
graphics->set_pen(pixel);
graphics->pixel(pimoroni::Point(x, y));
```

**Files Modified**: `Pico/PicoRenderer.cpp`

### 14. Missing Default Constructors for EntityPool

**Error**: `no matching function for call to 'EnemyProjectile::EnemyProjectile()'`

**Cause**: EntityPool allocates entities using default constructor, but entity classes only had parameterized constructors

**Fix**: Added default constructors to all entity types:
```cpp
EnemyProjectile() : position(0.0f, 0.0f), velocity(0.0f, 0.0f), 
                   lifetime(0.0f), shouldDestroy(false), movingRight(false), 
                   currentFrame(0), animationTimer(0.0f) {}
```

**Files Modified**:
- `Shared/game/enemy/EnemyProjectile.h`
- `Shared/game/projectile/Projectile.h`
- `Shared/game/enemy/BasicEnemy.h`
- `Shared/game/enemy/RangedEnemy.h`
- `Shared/game/healthpack/HealthPack.h`
- `Shared/game/objective/Objective.h`

### 15. Camera Offset Type Conversion

**Error**: `no matching function for call to 'min(fixed_t&, float)'`

**Cause**: std::min requires both arguments to be the same type

**Fix**: Made Camera methods conditional and used proper casts/conversions:
```cpp
#ifdef PLATFORM_PICO
    int getOffsetX() const { return FROM_FIXED(position.x); }
#else
    int getOffsetX() const { return static_cast<int>(position.x); }
#endif
```

Also updated Camera::follow to use fixed-point math throughout on Pico builds.

**Files Modified**:
- `Shared/game/level/Camera.h`
- `Shared/game/level/Camera.cpp`

### 16. EntityPool clear() Method

**Error**: Manual clearing of EntityPool was repetitive and error-prone

**Fix**: Added `clear()` method to EntityPool:
```cpp
void clear() {
    for (size_t i = 0; i < MaxCount; i++) {
        active[i] = false;
    }
    count = 0;
}
```

**Files Modified**: `Shared/game/Game.h`, `Shared/game/Game.cpp`

### 17. Projectile Velocity Initialization

**Error**: `call of overloaded 'Vec2(fixed_t, int)' is ambiguous`

**Cause**: Mixed types in constructor initialization

**Fix**: Wrapped in explicit Vec2 constructor:
```cpp
velocity(Vec2(right ? SPEED : -SPEED, 0.0f))
```

**Files Modified**: `Shared/game/projectile/Projectile.cpp`

### 18. IRenderer Include Path Case Sensitivity

**Error**: `Types.h` vs `types.h` case mismatch

**Fix**: Changed to lowercase `types.h` for consistency

**Files Modified**: `Shared/core/IRenderer.h`

### 19. Missing Include in Game.cpp

**Error**: Missing `<cstdio>` for `snprintf`

**Fix**: Added `#include <cstdio>` and made `<string>` conditional (Desktop only)

**Files Modified**: `Shared/game/Game.cpp`

## Additional Issues Fixed (Round 3)

### 20. Namespace Ambiguity with Pimoroni Rect

**Error**: `reference to 'Rect' is ambiguous` - compiler confused between `::Rect` and `pimoroni::Rect`

**Cause**: Pimoroni Pico Graphics library defines its own `Rect` and `Point` types. Include order matters.

**Fix**: 
- Added explicit `#include "../Shared/core/types.h"` at the top of `PicoRenderer.h` BEFORE pimoroni includes
- Qualified all pimoroni types with `pimoroni::` namespace (PicoGraphics_PenRGB565, ST7789, SPIPins, ROTATE_0, Point)
- Changed `graphics->set_pixel(Point, pixel)` to `graphics->set_pen(pixel); graphics->pixel(pimoroni::Point(x, y))`

**Files Modified**: `Pico/PicoRenderer.h`, `Pico/PicoRenderer.cpp`

### 21. PicoHaptics Unused Members

**Error**: `class 'PicoHaptics' does not have any field named 'stopTime'`

**Cause**: Constructor tried to initialize `stopTime` and `isActive` that weren't needed

**Fix**: Removed the member variables and their initialization. Simplified implementation.

**Files Modified**: `Pico/PicoHaptics.h`, `Pico/PicoHaptics.cpp`

### 22. Missing uint64_t Include

**Error**: `'uint64_t' does not name a type` in PicoHaptics.h

**Fix**: Added `#include <cstdint>` to PicoHaptics.h

**Files Modified**: `Pico/PicoHaptics.h`

### 23. Duplicate dtFixed Declaration

**Error**: `redeclaration of 'fixed_t dtFixed'` in Game.cpp

**Cause**: `dtFixed` was declared once for X movement and again for Y movement in the same scope

**Fix**: Removed second declaration, reusing the first one:
```cpp
#ifdef PLATFORM_PICO
    fixed_t dtFixed = floatToFixed(dt);
    player.position.x += FIXED_MUL(player.velocity.x, dtFixed);
#else
    player.position.x += player.velocity.x * dt;
#endif
collision.resolveHorizontal(player, level);

player.setGrounded(false);
#ifdef PLATFORM_PICO
    player.position.y += FIXED_MUL(player.velocity.y, dtFixed);  // Reuses dtFixed
#else
    player.position.y += player.velocity.y * dt;
#endif
```

**Files Modified**: `Shared/game/Game.cpp`

### 24. Vec2(fixed_t, float) Ambiguity in Constructors (Round 2)

**Error**: `call of overloaded 'Vec2(fixed_t, float)' is ambiguous` in BasicEnemy, RangedEnemy, Projectile constructors

**Cause**: When MOVE_SPEED/SPEED is `fixed_t` (on Pico), passing `0.0f` as second parameter creates ambiguity

**Fix**: Made velocity initialization conditional with `TO_FIXED(0.0f)` for Pico:
```cpp
#ifdef PLATFORM_PICO
    , velocity(Vec2((startMovingRight ? MOVE_SPEED : -MOVE_SPEED), TO_FIXED(0.0f)))
#else
    , velocity(Vec2((startMovingRight ? MOVE_SPEED : -MOVE_SPEED), 0.0f))
#endif
```

**Files Modified**:
- `Shared/game/enemy/BasicEnemy.cpp`
- `Shared/game/enemy/RangedEnemy.cpp`
- `Shared/game/projectile/Projectile.cpp`

## Testing Status (Current)

- **Linter**: Clean - NO errors reported in Shared/ or Pico/ folders
- **All Interface Signatures**: Matched correctly
- **All Default Constructors**: Added for EntityPool compatibility
- **All Type Conversions**: Fixed for Vec2/Rect construction
- **Namespace Conflicts**: Resolved with explicit pimoroni:: qualification
- **Include Order**: Fixed to prevent type ambiguities
- **Ready to Build**: All known compilation issues resolved

## Build Command

Via VSCode Pico extension:
1. Ctrl+Shift+P → "Raspberry Pi Pico: Configure CMake"
2. Ctrl+Shift+P → "Raspberry Pi Pico: Compile Project"
3. Check `build/platformer.uf2`

If build still fails, check the VSCode Output panel for specific errors and provide them for further fixes.
