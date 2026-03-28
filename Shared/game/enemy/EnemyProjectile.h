#pragma once
#include "../../core/types.h"

class EnemyProjectile {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(8.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(8.0f);
    static constexpr fixed_t SPEED = TO_FIXED(120.0f);
    static constexpr fixed_t MAX_LIFETIME = TO_FIXED(3.0f);
#else
    static constexpr float WIDTH = 8.0f;
    static constexpr float HEIGHT = 8.0f;
    static constexpr float SPEED = 120.0f;
    static constexpr float MAX_LIFETIME = 3.0f;
#endif
    static constexpr float ANIM_FRAME_DURATION_SEC = 0.0417f;
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    float lifetime;
    bool shouldDestroy;
    bool movingRight;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    EnemyProjectile() : position(0.0f, 0.0f), velocity(0.0f, 0.0f), lifetime(0.0f), 
                       shouldDestroy(false), movingRight(false), currentFrame(0), animationTimer(0.0f) {}
    EnemyProjectile(Vec2 startPos, Vec2 direction);
    
    void update(float deltaTime);
    Rect getCollider() const;
};
