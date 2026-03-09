#pragma once
#include "player/Player.h"
#include "player/PlayerController.h"
#include "projectile/Projectile.h"
#include "enemy/BasicEnemy.h"
#include "enemy/RangedEnemy.h"
#include "enemy/EnemyProjectile.h"
#include "healthpack/HealthPack.h"
#include "level/Level.h"
#include "level/Camera.h"
#include "physics/Physics.h"
#include "physics/Collision.h"
#include <vector>

class IRenderer;
class IInput;
class IHaptics;
class ITimer;

class Game {
public:
    Game(IRenderer* renderer, IInput* input, IHaptics* haptics, ITimer* timer);
    
    void init();
    void update();
    void render();
    
    bool loadLevel(const char* levelName);
    
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
    std::vector<Projectile> projectiles;
    std::vector<BasicEnemy> basicEnemies;
    std::vector<RangedEnemy> rangedEnemies;
    std::vector<EnemyProjectile> enemyProjectiles;
    std::vector<HealthPack> healthPacks;
    
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
    
    // HP UI animation state
    int hpUIFrame;
    float hpUIAnimTimer;
    
    // Current level tracking
    char currentLevelName[64];
    
    void checkPortalCollisions();
};

