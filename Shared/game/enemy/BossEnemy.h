#pragma once
#include "../../core/types.h"

class Level;
class Player;
class Game;

/** Boss archetypes — extend AI in BossEnemy.cpp via updateLaser / updateBulletHell; SUMMONER is driven from Game. */
enum class BossType {
    SUMMONER,
    LASER,
    BULLET_HELL
};

enum class BossState {
    Moving,
    Summoning
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
    /** LASER boss only: inner arena half-extent in pixels (32×26 tiles total inside walls). */
    static constexpr float LASER_ARENA_HALF_W_PX = 16.0f * ARENA_TILE_PX;
    static constexpr float LASER_ARENA_HALF_H_PX = 13.0f * ARENA_TILE_PX;
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
    int maxHealth;
    bool active;
    bool activated;
    BossType type;

    /** Summoner AI only; ignored for other boss types. */
    BossState summonerState;
    int currentWaypoint;
    float stateTimer;
    /** True after the ranged spawn for the current summoning phase (reset when entering Summoning). */
    bool summonerSpawnedThisPhase;

    /** Seconds between laser shots (lower fire rate = larger value). */
    static constexpr float LASER_FIRE_INTERVAL_SEC = 2.0f;

    /** LASER boss: randomized waypoint order (indices 0..4) and firing timer. */
    uint8_t laserWaypointOrder[5];
    uint8_t laserWaypointIndex;
    bool laserOrderInitialized;
    uint32_t laserRngState;
    float laserFireTimer;

    BossEnemy();
    BossEnemy(Vec2 spawn, BossType bossType);

    void update(float deltaTime, const Level& level, const Player& player);
    bool takeDamage(int amount);
    Rect getCollider() const;

    /** Four thin axis-aligned walls forming a closed rectangle around the boss (world space). */
    void computeArenaWalls(Rect out[4]) const;

    void updateLaser(float deltaTime, const Level& level, const Player& player, Game& game);

private:
    void updateBulletHell(float deltaTime, const Level& level, const Player& player);
};
