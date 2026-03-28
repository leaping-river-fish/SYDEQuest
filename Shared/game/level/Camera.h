#pragma once
#include "../../core/types.h"

class Player;
class Level;

class Camera {
public:
    Camera(int screenWidth, int screenHeight);
    
    void follow(const Player& player, const Level& level);
    
#ifdef PLATFORM_PICO
    /** Same as PicoRenderer fixedToPixelInt: floor division in fixed space (matches drawSpriteFrame / drawTile). */
    int getOffsetX() const { return static_cast<int>(static_cast<int32_t>(position.x) >> FIXED_SHIFT); }
    int getOffsetY() const { return static_cast<int>(static_cast<int32_t>(position.y) >> FIXED_SHIFT); }
#else
    int getOffsetX() const { return static_cast<int>(position.x); }
    int getOffsetY() const { return static_cast<int>(position.y); }
#endif
    
private:
    Vec2 position;  // Top-left of viewport in world space
    int screenWidth;
    int screenHeight;
    
    static constexpr float DEADZONE_X = 80.0f;
    static constexpr float DEADZONE_Y = 60.0f;
    static constexpr float SMOOTH_SPEED = 5.0f;
};

