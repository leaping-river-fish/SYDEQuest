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

/** Calvin Young (BULLET_HELL) phase machine. */
enum class BulletHellState {
    ProjectilePhase,
    BetweenPhases,
    ChargePhase,
    VulnerablePhase
};

enum class BulletHellChargeSub {
    Warning,
    Dash
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
    static constexpr float ARENA_TILE_PX = 16.0f;
    /** Extra half-extent beyond activation radius so arena walls sit outside the activation zone (15 tiles/side). */
    static constexpr float ARENA_EXTRA_TILES_PER_SIDE = 15.0f;
    /** Circular trigger radius around each boss (world px); 10 tiles. */
    static constexpr float BOSS_ACTIVATION_RADIUS_TILES = 10.0f;
    static constexpr float BOSS_ACTIVATION_DISTANCE = BOSS_ACTIVATION_RADIUS_TILES * ARENA_TILE_PX;
    /** LASER boss only: inner arena half-extent in pixels (32×26 tiles total inside walls). */
    static constexpr float LASER_ARENA_HALF_W_PX = 16.0f * ARENA_TILE_PX;
    static constexpr float LASER_ARENA_HALF_H_PX = 13.0f * ARENA_TILE_PX;
    /** BULLET_HELL: half-width 22 tiles each side, half-height 14 tiles up/down (from arena center). */
    static constexpr float BULLET_HELL_ARENA_HALF_W_PX = 22.0f * ARENA_TILE_PX;
    static constexpr float BULLET_HELL_ARENA_HALF_H_PX = 14.0f * ARENA_TILE_PX;
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
    /** Seconds of standing still after activation before AI runs (countdown while > 0). */
    float postActivationIdleRemaining;
    BossType type;

    /** Summoner AI only; ignored for other boss types. */
    BossState summonerState;
    int currentWaypoint;
    float stateTimer;
    /** True after the ranged spawn for the current summoning phase (reset when entering Summoning). */
    bool summonerSpawnedThisPhase;

    /** Seconds between laser shots (lower fire rate = larger value). */
    static constexpr float LASER_FIRE_INTERVAL_SEC = 2.0f;
    static constexpr float POST_ACTIVATION_IDLE_SEC = 2.0f;

    /** LASER boss: randomized waypoint order (indices 0..4) and firing timer. */
    uint8_t laserWaypointOrder[5];
    uint8_t laserWaypointIndex;
    bool laserOrderInitialized;
    uint32_t laserRngState;
    float laserFireTimer;
    /** Seconds left standing still after reaching a waypoint (>0 = paused). */
    float laserWaypointPauseRemaining;

    /** LASER boss: arena center at spawn (collider center); walls and laser bounds use this, not current position. */
    fixed_t laserArenaCenterX;
    fixed_t laserArenaCenterY;

    /** BULLET_HELL: fixed arena center at spawn (same role as laser arena). */
    fixed_t bulletHellArenaCenterX;
    fixed_t bulletHellArenaCenterY;

    BulletHellState bulletHellState;
    float bulletHellStateTimer;
    int chargeAttempts;
    BulletHellChargeSub bulletHellChargeSub;
    /** Normalized direction for current dash (locked at end of warning). */
    float chargeDirX;
    float chargeDirY;
    /** Player center (world px) when warning ended — dash runs past this point. */
    float dashLockPlayerX;
    float dashLockPlayerY;
    /** Total travel distance along chargeDir (distance to lock + overshoot). */
    float dashTotalDistance;
    float dashDistanceTraveled;

    /** Shrapnel pieces spawned when a boss projectile hits solid tile (fixed at 4). */
    static constexpr int kBulletHellShrapnelCount = 6;
    /** Dash speed (moderate — was tuned down from very fast values). */
    static constexpr float kBulletHellChargeSpeedPx = 480.0f;
    /** Extra distance past the player along the dash ray (stops just behind the player vs the boss). */
    static constexpr float kBulletHellDashPastPlayerPx = 40.0f;
    static constexpr float kBulletHellSpreadRadians = 0.14f;  // ~8 degrees between the 3 shots

    BossEnemy();
    BossEnemy(Vec2 spawn, BossType bossType);

    void update(float deltaTime, const Level& level, const Player& player);
    bool takeDamage(int amount);
    Rect getCollider() const;

    /** Four thin axis-aligned walls forming a closed rectangle around the boss (world space). */
    void computeArenaWalls(Rect out[4]) const;

    void updateLaser(float deltaTime, const Level& level, const Player& player, Game& game);

    void updateBulletHell(float deltaTime, const Level& level, const Player& player, Game& game);

    BulletHellState getBulletHellState() const { return bulletHellState; }
    BulletHellChargeSub getBulletHellChargeSub() const { return bulletHellChargeSub; }
};
