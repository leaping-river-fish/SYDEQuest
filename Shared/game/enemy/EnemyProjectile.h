#pragma once
#include "../../core/types.h"
#include <cstdint>

/** Ranged enemies use Integral sprite; bullet-hell boss uses Rustball / RustShrapnel art. */
enum class EnemyProjectileKind : uint8_t {
    Ranged,
    BulletHellMain,
    BulletHellShard
};

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

    /** If > 0, on solid tile hit spawn this many child projectiles (boss bullet hell); children use 0. */
    int shrapnelCount;

    EnemyProjectileKind kind;

    EnemyProjectile()
        : position(0.0f, 0.0f),
          velocity(0.0f, 0.0f),
          lifetime(0.0f),
          shouldDestroy(false),
          movingRight(false),
          currentFrame(0),
          animationTimer(0.0f),
          shrapnelCount(0),
          kind(EnemyProjectileKind::Ranged) {}

    EnemyProjectile(Vec2 startPos, Vec2 direction, int shrapnelPieces = 0,
                    EnemyProjectileKind projectileKind = EnemyProjectileKind::Ranged);
    
    void update(float deltaTime);
    Rect getCollider() const;
};
