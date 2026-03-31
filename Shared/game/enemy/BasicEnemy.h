#pragma once
#include "../../core/types.h"

class Level;

class BasicEnemy {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(16.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(16.0f);
    static constexpr fixed_t MOVE_SPEED = TO_FIXED(80.0f);
    static constexpr fixed_t GRAVITY = TO_FIXED(980.0f);
    static constexpr fixed_t MAX_FALL_SPEED = TO_FIXED(500.0f);
#else
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 80.0f;
    static constexpr float GRAVITY = 980.0f;
    static constexpr float MAX_FALL_SPEED = 500.0f;
#endif
    static constexpr float ANIM_FRAME_DURATION_SEC = 0.0833f;
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    uint8_t health;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    // Flags
    uint8_t flags;
    static constexpr uint8_t FLAG_MOVING_RIGHT = 1 << 0;
    
    bool movingRight() const { return flags & FLAG_MOVING_RIGHT; }
    void setMovingRight(bool value) {
        if (value) flags |= FLAG_MOVING_RIGHT;
        else flags &= ~FLAG_MOVING_RIGHT;
    }
    
    BasicEnemy() : position(0.0f, 0.0f), velocity(0.0f, 0.0f), health(3),
                  currentFrame(0), animationTimer(0.0f), flags(0) {}
    BasicEnemy(Vec2 startPos, bool startMovingRight = true);
    
    void update(float deltaTime, const Level& level);
    bool takeDamage(int amount);
    Rect getCollider() const;
    
private:
    void checkEdgeDetection(const Level& level);
    void checkWallCollision(const Level& level);
    void applyGravity(float deltaTime);
    /** Swept vertical resolution (matches player) so large dt cannot tunnel floors/platforms. */
    void resolveVerticalTerrain(const Level& level, fixed_t prevY);
};
