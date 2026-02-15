#pragma once
#include "core/Types.h"

class Player;
class Level;

class Camera {
public:
    Camera(int screenWidth, int screenHeight);
    
    void follow(const Player& player, const Level& level);
    
    int getOffsetX() const { return (int)position.x; }
    int getOffsetY() const { return (int)position.y; }
    
private:
    Vec2 position;  // Top-left of viewport in world space
    int screenWidth;
    int screenHeight;
    
    static constexpr float DEADZONE_X = 80.0f;
    static constexpr float DEADZONE_Y = 60.0f;
    static constexpr float SMOOTH_SPEED = 5.0f;
};

