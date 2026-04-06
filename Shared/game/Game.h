#pragma once
#include <cstdint>
#include "player/Player.h"
#include "player/PlayerController.h"
#include "projectile/Projectile.h"
#include "enemy/BasicEnemy.h"
#include "enemy/RangedEnemy.h"
#include "enemy/BossEnemy.h"
#include "laser/Laser.h"
#include "enemy/EnemyProjectile.h"
#include "healthpack/HealthPack.h"
#include "objective/Objective.h"
#include "level/Level.h"
#include "level/Camera.h"
#include "physics/Physics.h"
#include "physics/Collision.h"

#ifdef PLATFORM_PICO
    #include <cstddef>
#else
    #include <vector>
#endif

class IRenderer;
class IInput;
class IHaptics;
class ITimer;

enum class GameState {
    MainMenu,
    Playing,
    GameOver,
    YouPassed
};

#ifdef PLATFORM_PICO
template<typename T, size_t MaxCount>
struct EntityPool {
    T entities[MaxCount];
    bool active[MaxCount];
    size_t count;
    
    EntityPool() : count(0) {
        for (size_t i = 0; i < MaxCount; i++) {
            active[i] = false;
        }
    }
    
    T* allocate() {
        for (size_t i = 0; i < MaxCount; i++) {
            if (!active[i]) {
                active[i] = true;
                if (i >= count) count = i + 1;
                return &entities[i];
            }
        }
        return nullptr;
    }
    
    void free(size_t index) {
        if (index < MaxCount) {
            active[index] = false;
        }
    }
    
    void clear() {
        for (size_t i = 0; i < MaxCount; i++) {
            active[i] = false;
        }
        count = 0;
    }
    
    T& operator[](size_t index) { return entities[index]; }
    const T& operator[](size_t index) const { return entities[index]; }
    
    size_t size() const { return MaxCount; }
    
    bool isActive(size_t index) const { 
        return index < MaxCount && active[index]; 
    }
};
#endif

class Game {
    friend class BossEnemy;

public:
    Game(IRenderer* renderer, IInput* input, IHaptics* haptics, ITimer* timer);
    
    void init();
    void update();
    void render();
    
    bool loadLevel(const char* levelName);
    Level& getLevel() { return level; }

#ifdef PLATFORM_PICO
    /** Reposition player and repopulate entity pools from the current Level (after loadFromBinaryData). */
    void syncEntitiesFromCurrentLevel(const char* levelNameForTracking);
#endif

private:
    // Platform interfaces
    IRenderer* renderer;
    IInput* input;
    IHaptics* haptics;
    ITimer* timer;
    
    // Game systems
    Player player;
    Level level;
    Camera camera;
    Physics physics;
    Collision collision;
    PlayerController controller;
    
    // Entities
#ifdef PLATFORM_PICO
    EntityPool<Projectile, 20> projectiles;
    EntityPool<BasicEnemy, Level::MAX_BASIC_ENEMIES> basicEnemies;
    EntityPool<RangedEnemy, Level::MAX_RANGED_ENEMIES> rangedEnemies;
    EntityPool<EnemyProjectile, 30> enemyProjectiles;
    EntityPool<HealthPack, 2> healthPacks;
    EntityPool<Objective, 1> objectives;
    EntityPool<BossEnemy, 3> bosses;
    EntityPool<Laser, 24> lasers;
    EntityPool<Rect, 12> arenaWalls;
    
    struct EntityState {
        bool isActive;
        bool wasRendered;
    };
    EntityState healthPackStates[Level::MAX_HEALTH_PACKS];
    EntityState objectiveStates[Level::MAX_OBJECTIVES];

    /** Enemy updates are not distance-culled on Pico (small pools; always simulate active slots). */
#else
    std::vector<Projectile> projectiles;
    std::vector<BasicEnemy> basicEnemies;
    std::vector<RangedEnemy> rangedEnemies;
    std::vector<EnemyProjectile> enemyProjectiles;
    std::vector<HealthPack> healthPacks;
    std::vector<Objective> objectives;
    std::vector<BossEnemy> bosses;
    std::vector<Laser> lasers;
    std::vector<Rect> arenaWalls;
    
    static constexpr float DEACTIVATION_DISTANCE = 500.0f;
#endif
    
    // Assets
    int playerSpritesheet;
    int projectileLeftSpritesheet;
    int projectileRightSpritesheet;
    int terrainSpritesheet;
    int basicEnemySpritesheet;
    int rangedEnemySpritesheet;
    int enemyProjectileLeftSpritesheet;
    int enemyProjectileRightSpritesheet;
    int energySpritesheet;
    int healthPackSpritesheet;
    int portalSpritesheet;
    int chargerSpritesheet;
    int enclosureSpritesheet;
    int hapticSpritesheet;
    int partsSpritesheet;
    int screenSpritesheet;
    int picoSpritesheet;
    int bossIconSpritesheet;
    int titleSpritesheet;
    int bossSeanSpritesheet;
    int bossLaserSpritesheet;
    int gameOverSpritesheet;

    // HP UI animation state
    int hpUIFrame;
    float hpUIAnimTimer;
    
    // Portal animation state
    int portalFrame;
    float portalAnimTimer;

    int bossSeanFrame;
    float bossSeanAnimTimer;

    int bossLaserFrame;
    float bossLaserAnimTimer;

    int gameOverFrame;
    float gameOverAnimTimer;

    // Current level tracking (authoritative path is set in loadLevel / Pico sync as soon as the stage is loaded).
    char currentLevelName[64];
    /** Snapshot for Game Over UI / debugging (prefer deathLevelName when set at lethal death). */
    char gameOverLevelName[64];
    /** Filled from currentLevelName when recording a lethal death (before GameOver). Mirrors current after every load. */
    char deathLevelName[64];

    // Objective tracking
    bool levelObjectiveCollected;
    /** True if current level file defines at least one boss spawn (drives boss HUD). */
    bool levelHasBoss;
    /** False until player leaves portal hitbox after last transition; prevents multi-frame / spawn-in-portal double loads. */
    bool portalTransitionArmed;
    /** True after we have recorded deathLevelName for the current lethal death (cleared on load / respawn). */
    bool deathLevelCaptured;
    /** Monotonic index for Playing-state updates (debug NDJSON correlation). */
    uint32_t debugUpdateFrameIndex;

    GameState state;

    void renderMainMenu();
    void renderGameOver();
    void renderYouPassed();
    void renderPlayingWorld();
    Rect menuTitlePlaceholderRect() const;
    Rect menuStartButtonRect() const;
    Rect gameOverRetryButtonRect() const;

    void processPlayerInput(float dt);
    void processPlayerMovement(float dt);
    /** Returns true if the frame should stop (e.g. pit death handled). */
    bool resolveCollisions(float dt);
    void checkPortalCollisions();
    void updateLevelState();
    void notifyPlayerDamageHaptics();
    /** Contact / projectile / laser damage: applies HP loss, haptics, and i-frames if currently vulnerable. */
    void applyDamageToPlayerIfVulnerable(int amount);
    bool hasAliveBoss() const;
    int getObjectiveSpritesheet(ObjectiveType type) const;

#ifdef PLATFORM_PICO
    void resetAndSpawnEntitiesPico();
#endif

    void updateSummonerBoss(BossEnemy& boss, float dt);
    void updateLaserBoss(BossEnemy& boss, float dt);
    void spawnLaser(BossEnemy& boss, const Vec2& origin, float dirX, float dirY);
    void updateLasers(float dt);
    void renderLasers();
    void checkLaserCollisions();
    /** True while a laser is still running (warning line or beam). */
    bool hasActiveLaser() const;
    void rebuildArenaWallsFromActivatedBosses();
    void resolveArenaWallsForPlayer();
    /** Laser arena: if player touches the bottom wall strip, snap to a safe spot (avoids fall-through / jitter). */
    void applyLaserArenaBottomTeleport();
    void renderBossHealthBar();

    /** Pit / out-of-bounds: teleport to `level.getSpawnPoint()` (no second loadLevel — avoids fragile desktop reload). */
    void respawnPlayerAtCurrentLevelStart();

    /** Full reload of `currentLevelName` from disk (main menu Start). */
    void restartCurrentLevel();
    void restartLevelFromPath(const char* path);
    /** Game Over retry: respawn at spawn with full HP without reloading the level. */
    void retryAfterGameOver();
    void beginGameOver();
    void beginYouPassed();
};

