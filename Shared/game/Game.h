#pragma once
#include "player/Player.h"
#include "player/PlayerController.h"
#include "projectile/Projectile.h"
#include "enemy/BasicEnemy.h"
#include "enemy/RangedEnemy.h"
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
public:
    Game(IRenderer* renderer, IInput* input, IHaptics* haptics, ITimer* timer);
    
    void init();
    void update();
    void render();
    
    bool loadLevel(const char* levelName);
    Level& getLevel() { return level; }
    
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
    EntityPool<BasicEnemy, 10> basicEnemies;
    EntityPool<RangedEnemy, 10> rangedEnemies;
    EntityPool<EnemyProjectile, 30> enemyProjectiles;
    EntityPool<HealthPack, 1> healthPacks;
    EntityPool<Objective, 1> objectives;
    
    struct EntityState {
        bool isActive;
        bool wasRendered;
    };
    EntityState basicEnemyStates[25];
    EntityState rangedEnemyStates[20];
    EntityState healthPackStates[1];
    EntityState objectiveStates[1];
    
    static constexpr fixed_t ACTIVATION_DISTANCE = TO_FIXED(400.0f);
    static constexpr fixed_t DEACTIVATION_DISTANCE = TO_FIXED(500.0f);
#else
    std::vector<Projectile> projectiles;
    std::vector<BasicEnemy> basicEnemies;
    std::vector<RangedEnemy> rangedEnemies;
    std::vector<EnemyProjectile> enemyProjectiles;
    std::vector<HealthPack> healthPacks;
    std::vector<Objective> objectives;
    
    static constexpr float ACTIVATION_DISTANCE = 400.0f;
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
    
    // HP UI animation state
    int hpUIFrame;
    float hpUIAnimTimer;
    
    // Portal animation state
    int portalFrame;
    float portalAnimTimer;
    
    // Current level tracking
    char currentLevelName[64];
    
    // Objective tracking
    bool levelObjectiveCollected;
    
    void checkPortalCollisions();
    int getObjectiveSpritesheet(ObjectiveType type) const;
};

