#pragma once
#include "../../core/types.h"

class Level;
class Player;

/** Boss archetypes — extend AI in BossEnemy.cpp via updateSummoner / updateLaser / updateBulletHell. */
enum class BossType {
    SUMMONER,
    LASER,
    BULLET_HELL
};

class BossEnemy {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(32.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(32.0f);
#else
    static constexpr float WIDTH = 32.0f;
    static constexpr float HEIGHT = 32.0f;
#endif
    static constexpr float BOSS_ACTIVATION_DISTANCE = 300.0f;
    /** Extra half-extent beyond activation radius so arena walls sit outside the activation zone (15 tiles/side). */
    static constexpr float ARENA_EXTRA_TILES_PER_SIDE = 15.0f;
    static constexpr float ARENA_TILE_PX = 16.0f;
    /** Arena half-size in pixels (world space), from boss collider center — activation radius + padding. */
#ifdef PLATFORM_PICO
    static constexpr fixed_t ARENA_HALF_W =
        TO_FIXED(BOSS_ACTIVATION_DISTANCE + ARENA_EXTRA_TILES_PER_SIDE * ARENA_TILE_PX);
    static constexpr fixed_t ARENA_HALF_H =
        TO_FIXED(BOSS_ACTIVATION_DISTANCE + ARENA_EXTRA_TILES_PER_SIDE * ARENA_TILE_PX);
    static constexpr fixed_t ARENA_WALL_THICKNESS = TO_FIXED(16.0f);
#else
    static constexpr float ARENA_HALF_W =
        BOSS_ACTIVATION_DISTANCE + ARENA_EXTRA_TILES_PER_SIDE * ARENA_TILE_PX;
    static constexpr float ARENA_HALF_H =
        BOSS_ACTIVATION_DISTANCE + ARENA_EXTRA_TILES_PER_SIDE * ARENA_TILE_PX;
    static constexpr float ARENA_WALL_THICKNESS = 16.0f;
#endif

    Vec2 position;
    int health;
    bool active;
    bool activated;
    BossType type;

    BossEnemy();
    BossEnemy(Vec2 spawn, BossType bossType);

    void update(float deltaTime, const Level& level, const Player& player);
    bool takeDamage(int amount);
    Rect getCollider() const;

    /** Four thin axis-aligned walls forming a closed rectangle around the boss (world space). */
    void computeArenaWalls(Rect out[4]) const;

private:
    void updateSummoner(float deltaTime, const Level& level, const Player& player);
    void updateLaser(float deltaTime, const Level& level, const Player& player);
    void updateBulletHell(float deltaTime, const Level& level, const Player& player);
};
