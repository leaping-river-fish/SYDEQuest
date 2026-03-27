#pragma once
#include "../../core/types.h"
#include "EnemyProjectile.h"
#include <cstddef>

#ifndef PLATFORM_PICO
    #include <vector>
#else
    template<typename T, size_t MaxCount> struct EntityPool;
#endif

class Level;
class Player;

class RangedEnemy {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(16.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(16.0f);
    static constexpr fixed_t MOVE_SPEED = TO_FIXED(50.0f);
    static constexpr fixed_t GRAVITY = TO_FIXED(980.0f);
    static constexpr fixed_t MAX_FALL_SPEED = TO_FIXED(500.0f);
    static constexpr fixed_t FRAME_TIME = TO_FIXED(0.0833f);
    static constexpr fixed_t SHOOT_INTERVAL = TO_FIXED(1.0f);
    static constexpr fixed_t DETECTION_RADIUS = TO_FIXED(160.0f);
    static constexpr fixed_t SHOOT_PAUSE_TIME = TO_FIXED(0.3f);
#else
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 50.0f;
    static constexpr float GRAVITY = 980.0f;
    static constexpr float MAX_FALL_SPEED = 500.0f;
    static constexpr float FRAME_TIME = 0.0833f;
    static constexpr float SHOOT_INTERVAL = 1.0f;
    static constexpr float DETECTION_RADIUS = 160.0f;
    static constexpr float SHOOT_PAUSE_TIME = 0.3f;
#endif
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    uint8_t health;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    // Shooting state
    float shootTimer;
    float shootPauseTimer;
    
    // Flags
    uint8_t flags;
    static constexpr uint8_t FLAG_FACING_RIGHT = 1 << 0;
    static constexpr uint8_t FLAG_IS_SHOOTING = 1 << 1;
    
    bool facingRight() const { return flags & FLAG_FACING_RIGHT; }
    void setFacingRight(bool value) {
        if (value) flags |= FLAG_FACING_RIGHT;
        else flags &= ~FLAG_FACING_RIGHT;
    }
    
    bool isShooting() const { return flags & FLAG_IS_SHOOTING; }
    void setIsShooting(bool value) {
        if (value) flags |= FLAG_IS_SHOOTING;
        else flags &= ~FLAG_IS_SHOOTING;
    }
    
    RangedEnemy() : position(0.0f, 0.0f), velocity(0.0f, 0.0f), health(5),
                   currentFrame(0), animationTimer(0.0f), shootTimer(0.0f),
                   shootPauseTimer(0.0f), flags(0) {}
    RangedEnemy(Vec2 startPos, bool startFacingRight = true);
    
#ifdef PLATFORM_PICO
    void update(float deltaTime, const Level& level, const Player& player, EntityPool<EnemyProjectile, 30>& projectiles);
#else
    void update(float deltaTime, const Level& level, const Player& player, std::vector<EnemyProjectile>& projectiles);
#endif
    bool takeDamage(int amount);
    Rect getCollider() const;
    
private:
    void checkEdgeDetection(const Level& level);
    void checkWallCollision(const Level& level);
    void applyGravity(float deltaTime);
    bool hasLineOfSight(Vec2 targetPos, const Level& level) const;
    bool canDetectPlayer(const Player& player, const Level& level) const;
};
