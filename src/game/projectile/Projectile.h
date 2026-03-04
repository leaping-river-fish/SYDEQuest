#pragma once
#include "core/Types.h"

class Projectile {
public:
    static constexpr float WIDTH = 8.0f;
    static constexpr float HEIGHT = 8.0f;
    static constexpr float SPEED = 350.0f;
    static constexpr float MAX_LIFETIME = 2.0f;  // Seconds before auto-destroy
    static constexpr float FRAME_TIME = 0.0417f; // 24 FPS animation (1/24 = 0.0417)
    static constexpr int TOTAL_FRAMES = 12;      // 12 frames in sprite sheet
    
    Vec2 position;
    Vec2 velocity;
    float lifetime;
    bool shouldDestroy;
    bool movingRight;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    Projectile(Vec2 startPos, bool movingRight);
    
    void update(float deltaTime);
    Rect getCollider() const;
};
