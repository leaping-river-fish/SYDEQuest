#pragma once
#include "core/Types.h"

class EnemyProjectile {
public:
    static constexpr float WIDTH = 8.0f;
    static constexpr float HEIGHT = 8.0f;
    static constexpr float SPEED = 120.0f;
    static constexpr float MAX_LIFETIME = 3.0f;
    static constexpr float FRAME_TIME = 0.0417f; // 24 FPS animation (1/24 = 0.0417)
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    float lifetime;
    bool shouldDestroy;
    bool movingRight;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    EnemyProjectile(Vec2 startPos, Vec2 direction);
    
    void update(float deltaTime);
    Rect getCollider() const;
};
