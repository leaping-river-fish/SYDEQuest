#include "Game.h"
#include "../core/IRenderer.h"
#include "../core/IInput.h"
#include "../core/IHaptics.h"
#include "../core/ITimer.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cctype>

#ifndef PLATFORM_PICO
    #include <string>
#endif

#ifdef PLATFORM_PICO
#include "../../Pico/assets/level1_data.h"
#include "../../Pico/assets/level2_data.h"
#include "../../Pico/assets/level3_data.h"
#endif

namespace {
// Set to true to force bosses active/visible without entering activation range (debug only).
constexpr bool kDebugForceBossActivated = false;

/** Case-insensitive substring (ASCII); needle must be lower-case. */
bool pathContainsInsensitive(const char* haystack, const char* needleLower) {
    if (!haystack || !needleLower) {
        return false;
    }
    char buf[96];
    size_t i = 0;
    for (; haystack[i] && i < sizeof(buf) - 1; ++i) {
        buf[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(haystack[i])));
    }
    buf[i] = '\0';
    return std::strstr(buf, needleLower) != nullptr;
}

bool pointInRect(int px, int py, const Rect& r) {
#ifdef PLATFORM_PICO
    const fixed_t fpx = TO_FIXED(static_cast<float>(px));
    const fixed_t fpy = TO_FIXED(static_cast<float>(py));
    return fpx >= r.x && fpx < r.x + r.width && fpy >= r.y && fpy < r.y + r.height;
#else
    return px >= static_cast<int>(r.x) && px < static_cast<int>(r.x + r.width) &&
           py >= static_cast<int>(r.y) && py < static_cast<int>(r.y + r.height);
#endif
}
}  // namespace

#ifndef PLATFORM_PICO
inline float distanceSquared(const Vec2& a, const Vec2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}
#else
/** Squared distance in fixed units (same scale as FIXED_MUL); int64 avoids int32 overflow when far apart. */
inline bool playerWithinBossActivationSq(const Vec2& playerPos, const Vec2& bossTopLeft) {
    fixed_t bcx = bossTopLeft.x + FIXED_DIV(BossEnemy::WIDTH, TO_FIXED(2.0f));
    fixed_t bcy = bossTopLeft.y + FIXED_DIV(BossEnemy::HEIGHT, TO_FIXED(2.0f));
    std::int64_t dx = static_cast<std::int64_t>(playerPos.x) - static_cast<std::int64_t>(bcx);
    std::int64_t dy = static_cast<std::int64_t>(playerPos.y) - static_cast<std::int64_t>(bcy);
    std::int64_t d2 = ((dx * dx) >> FIXED_SHIFT) + ((dy * dy) >> FIXED_SHIFT);
    std::int64_t r = static_cast<std::int64_t>(TO_FIXED(BossEnemy::BOSS_ACTIVATION_DISTANCE));
    std::int64_t r2 = (r * r) >> FIXED_SHIFT;
    return d2 < r2;
}
#endif

Game::Game(IRenderer* r, IInput* i, IHaptics* h, ITimer* t)
    : renderer(r), input(i), haptics(h), timer(t),
      camera(r->getScreenWidth(), r->getScreenHeight()),
      playerSpritesheet(-1),
      projectileLeftSpritesheet(-1),
      projectileRightSpritesheet(-1),
      terrainSpritesheet(-1),
      basicEnemySpritesheet(-1),
      rangedEnemySpritesheet(-1),
      enemyProjectileLeftSpritesheet(-1),
      enemyProjectileRightSpritesheet(-1),
      energySpritesheet(-1),
      healthPackSpritesheet(-1),
      portalSpritesheet(-1),
      chargerSpritesheet(-1),
      enclosureSpritesheet(-1),
      hapticSpritesheet(-1),
      partsSpritesheet(-1),
      screenSpritesheet(-1),
      picoSpritesheet(-1),
      bossIconSpritesheet(-1),
      titleSpritesheet(-1),
      bossSeanSpritesheet(-1),
      gameOverSpritesheet(-1),
      hpUIFrame(0),
      hpUIAnimTimer(0.0f),
      portalFrame(0),
      portalAnimTimer(0.0f),
      bossSeanFrame(0),
      bossSeanAnimTimer(0.0f),
      gameOverFrame(0),
      gameOverAnimTimer(0.0f),
      levelObjectiveCollected(false),
      levelHasBoss(false),
      state(GameState::MainMenu)
{
    currentLevelName[0] = '\0';
}

void Game::init() {
#ifndef PLATFORM_PICO
    loadLevel("../levels/Level1.csv");
#endif
    // Pico: level tiles + metadata come from embedded header; main_pico calls
    // loadFromBinaryData then syncEntitiesFromCurrentLevel() after textures load.

    playerSpritesheet = renderer->loadTexture("../assets/AlternatingWalk.png");
    projectileLeftSpritesheet = renderer->loadTexture("../assets/PencilSpinLeft.png");
    projectileRightSpritesheet = renderer->loadTexture("../assets/PencilSpinRight.png");
    terrainSpritesheet = renderer->loadTexture("../assets/FullTerrainSpriteSheet.png");
    basicEnemySpritesheet = renderer->loadTexture("../assets/Circle.png");
    rangedEnemySpritesheet = renderer->loadTexture("../assets/CalcQuiz.png");
    enemyProjectileLeftSpritesheet = renderer->loadTexture("../assets/IntegralSpinLeft.png");
    enemyProjectileRightSpritesheet = renderer->loadTexture("../assets/IntegralSpinRight.png");
    energySpritesheet = renderer->loadTexture("../assets/Energy.png");
    healthPackSpritesheet = renderer->loadTexture("../assets/EnergyDrink.png");
    portalSpritesheet = renderer->loadTexture("../assets/PortalSpriteSheet.png");
    chargerSpritesheet = renderer->loadTexture("../assets/ChargerSprite.png");
    enclosureSpritesheet = renderer->loadTexture("../assets/EnclosureSprite.png");
    hapticSpritesheet = renderer->loadTexture("../assets/HapticSprite.png");
    partsSpritesheet = renderer->loadTexture("../assets/PartsSprite.png");
    screenSpritesheet = renderer->loadTexture("../assets/ScreenSprite.png");
    picoSpritesheet = renderer->loadTexture("../assets/PicoSprite.png");
    bossIconSpritesheet = renderer->loadTexture("../assets/Boss.png");
    titleSpritesheet = renderer->loadTexture("../assets/SYDEQuest.png");
    bossSeanSpritesheet = renderer->loadTexture("../assets/SeanSpeziale.png");
    gameOverSpritesheet = renderer->loadTexture("../assets/GameOver.png");

    state = GameState::MainMenu;
}

#ifdef PLATFORM_PICO
void Game::resetAndSpawnEntitiesPico() {
    player.position = level.getSpawnPoint();
    player.velocity = Vec2(0.0f, 0.0f);
    player.setGrounded(false);
    player.invincibilityTimer = 0.0f;

    projectiles.clear();
    basicEnemies.clear();
    rangedEnemies.clear();
    enemyProjectiles.clear();
    healthPacks.clear();
    objectives.clear();
    bosses.clear();
    arenaWalls.clear();

    for (size_t i = 0; i < Level::MAX_HEALTH_PACKS; i++) {
        healthPackStates[i].isActive = false;
        healthPackStates[i].wasRendered = false;
    }
    for (size_t i = 0; i < Level::MAX_OBJECTIVES; i++) {
        objectiveStates[i].isActive = false;
        objectiveStates[i].wasRendered = false;
    }

    const Vec2* basicEnemySpawnPoints = level.getBasicEnemySpawns();
    uint8_t basicEnemyCount = level.getBasicEnemySpawnCount();
    for (uint8_t i = 0; i < basicEnemyCount; i++) {
        BasicEnemy* enemy = basicEnemies.allocate();
        if (enemy) {
            *enemy = BasicEnemy(basicEnemySpawnPoints[i], true);
        }
    }

    const Vec2* rangedEnemySpawnPoints = level.getRangedEnemySpawns();
    uint8_t rangedEnemyCount = level.getRangedEnemySpawnCount();
    for (uint8_t i = 0; i < rangedEnemyCount; i++) {
        RangedEnemy* enemy = rangedEnemies.allocate();
        if (enemy) {
            *enemy = RangedEnemy(rangedEnemySpawnPoints[i], true);
        }
    }

    const Vec2* healthPackSpawnPoints = level.getHealthPackSpawns();
    uint8_t healthPackCount = level.getHealthPackSpawnCount();
    for (uint8_t i = 0; i < healthPackCount; i++) {
        HealthPack* pack = healthPacks.allocate();
        if (pack) {
            *pack = HealthPack(healthPackSpawnPoints[i]);
        }
    }

    levelObjectiveCollected = false;
    const Vec2* objectiveSpawnPoints = level.getObjectiveSpawns();
    const ObjectiveType* objectiveTypes = level.getObjectiveTypes();
    uint8_t objectiveCount = level.getObjectiveSpawnCount();
    for (uint8_t i = 0; i < objectiveCount; i++) {
        Objective* obj = objectives.allocate();
        if (obj) {
            *obj = Objective(objectiveSpawnPoints[i], objectiveTypes[i]);
        }
    }

    const Vec2* bossSpawnPoints = level.getBossSpawns();
    const BossType* bossTypesData = level.getBossTypes();
    uint8_t bossCount = level.getBossSpawnCount();
    levelHasBoss = (bossCount > 0);
    for (uint8_t i = 0; i < bossCount; i++) {
        BossEnemy* b = bosses.allocate();
        if (b) {
            *b = BossEnemy(bossSpawnPoints[i], bossTypesData[i]);
        }
    }
    if (kDebugForceBossActivated) {
        for (size_t i = 0; i < bosses.size(); i++) {
            if (!bosses.isActive(i)) continue;
            bosses[i].activated = true;
        }
    }

    camera.follow(player, level);
}

void Game::syncEntitiesFromCurrentLevel(const char* levelNameForTracking) {
    if (levelNameForTracking) {
        strncpy(currentLevelName, levelNameForTracking, sizeof(currentLevelName) - 1);
        currentLevelName[sizeof(currentLevelName) - 1] = '\0';
    }
    resetAndSpawnEntitiesPico();
}
#endif

bool Game::loadLevel(const char* levelName) {
#ifdef PLATFORM_PICO
    // Embedded build: CSV paths from portals are not on a filesystem; use baked level data.
    bool ok = false;
    if (pathContainsInsensitive(levelName, "level3")) {
        ok = level.loadFromBinaryData(level3_tiles, level3_width, level3_height, &level3_metadata);
    } else if (pathContainsInsensitive(levelName, "level2")) {
        ok = level.loadFromBinaryData(level2_tiles, level2_width, level2_height, &level2_metadata);
    } else {
        ok = level.loadFromBinaryData(level1_tiles, level1_width, level1_height, &level1_metadata);
    }
    if (!ok) {
        return false;
    }
    levelHasBoss = (level.getBossSpawnCount() > 0);
#else
    if (!level.loadFromFile(levelName)) {
        return false;
    }
    levelHasBoss = !level.getBossSpawns().empty();
#endif

    strncpy(currentLevelName, levelName, sizeof(currentLevelName) - 1);
    currentLevelName[sizeof(currentLevelName) - 1] = '\0';
    
#ifdef PLATFORM_PICO
    resetAndSpawnEntitiesPico();
#else
    player.position = level.getSpawnPoint();
    player.velocity = Vec2(0.0f, 0.0f);
    player.setGrounded(false);
    
    projectiles.clear();
    basicEnemies.clear();
    rangedEnemies.clear();
    enemyProjectiles.clear();
    healthPacks.clear();
    objectives.clear();
    bosses.clear();
    arenaWalls.clear();
#endif
    
    // Spawn basic enemies from level data
#ifndef PLATFORM_PICO
    const std::vector<Vec2>& basicEnemySpawnPoints = level.getBasicEnemySpawns();
    for (const Vec2& spawnPos : basicEnemySpawnPoints) {
        basicEnemies.push_back(BasicEnemy(spawnPos, true));
    }
    
    const std::vector<Vec2>& rangedEnemySpawnPoints = level.getRangedEnemySpawns();
    for (const Vec2& spawnPos : rangedEnemySpawnPoints) {
        rangedEnemies.push_back(RangedEnemy(spawnPos, true));
    }
    
    const std::vector<Vec2>& healthPackSpawnPoints = level.getHealthPackSpawns();
    for (const Vec2& spawnPos : healthPackSpawnPoints) {
        healthPacks.push_back(HealthPack(spawnPos));
    }
    
    levelObjectiveCollected = false;
    const std::vector<std::pair<Vec2, ObjectiveType>>& objectiveSpawnPoints = level.getObjectiveSpawns();
    for (const auto& [spawnPos, type] : objectiveSpawnPoints) {
        objectives.push_back(Objective(spawnPos, type));
    }

    const std::vector<Vec2>& bossSpawnPoints = level.getBossSpawns();
    const std::vector<BossType>& bossTypesVec = level.getBossTypes();

    for (size_t i = 0; i < bossSpawnPoints.size(); i++) {
        bosses.push_back(BossEnemy(bossSpawnPoints[i], bossTypesVec[i]));
    }
    if (kDebugForceBossActivated) {
        for (auto& boss : bosses) {
            boss.activated = true;
        }
    }
#endif

#ifndef PLATFORM_PICO
    camera.follow(player, level);
#endif

    return true;
}

void Game::resetLevel() {
    if (currentLevelName[0] == '\0') {
        return;
    }
    player.health = 3;
    loadLevel(currentLevelName);
}

void Game::update() {
    timer->update();
    input->update();

    float dt = timer->getDeltaTime();

    if (input->wasJustPressed(Button::MenuBack)) {
        state = GameState::MainMenu;
        return;
    }

    if (state == GameState::MainMenu) {
        const Rect startBtn = menuStartButtonRect();
        int mx = 0;
        int my = 0;
        input->getMouseLogicalPosition(mx, my);
        if (input->wasJustPressed(Button::MenuConfirm) ||
            (input->wasMousePrimaryJustPressed() && pointInRect(mx, my, startBtn))) {
            state = GameState::Playing;
            resetLevel();
        }
        return;
    }
    if (state == GameState::GameOver) {
        if (gameOverSpritesheet >= 0) {
            gameOverAnimTimer += dt;
            if (gameOverAnimTimer >= 0.1f) {
                gameOverAnimTimer -= 0.1f;
                gameOverFrame = (gameOverFrame + 1) % 12;
            }
        }
        const Rect retryBtn = gameOverRetryButtonRect();
        int mx = 0;
        int my = 0;
        input->getMouseLogicalPosition(mx, my);
        if (input->wasJustPressed(Button::MenuConfirm) ||
            (input->wasMousePrimaryJustPressed() && pointInRect(mx, my, retryBtn))) {
            state = GameState::Playing;
            resetLevel();
        }
        return;
    }

    // Playing
    controller.update(player, input, dt);
    
    // Physics
    physics.applyGravity(player, dt);
    physics.clampVelocity(player);
    
    // Move X and resolve
#ifdef PLATFORM_PICO
    fixed_t dtFixed = floatToFixed(dt);
    player.position.x += FIXED_MUL(player.velocity.x, dtFixed);
#else
    player.position.x += player.velocity.x * dt;
#endif
    collision.resolveHorizontal(player, level);
    
    // Move Y and resolve (prevY enables swept vertical collision so large dt cannot tunnel solids)
    player.setGrounded(false);
#ifdef PLATFORM_PICO
    fixed_t prevY = player.position.y;
    player.position.y += FIXED_MUL(player.velocity.y, dtFixed);
#else
    fixed_t prevY = player.position.y;
    player.position.y += player.velocity.y * dt;
#endif
    collision.resolveVertical(player, level, prevY);

    // Boss activation (before arena collision); AI runs later after enemies/projectiles.
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < bosses.size(); i++) {
        if (!bosses.isActive(i)) continue;
        BossEnemy& boss = bosses[i];
        if (!boss.active || boss.activated) continue;
        if (playerWithinBossActivationSq(player.position, boss.position)) {
            boss.activated = true;
            arenaWalls.clear();
            Rect w[4];
            boss.computeArenaWalls(w);
            for (int k = 0; k < 4; k++) {
                Rect* slot = arenaWalls.allocate();
                if (slot) {
                    *slot = w[k];
                }
            }
        }
    }
#else
    {
        const float bossActR = BossEnemy::BOSS_ACTIVATION_DISTANCE;
        const float bossActR2 = bossActR * bossActR;
        for (auto& boss : bosses) {
            if (!boss.active || boss.activated) continue;
            float cx = boss.position.x + BossEnemy::WIDTH * 0.5f;
            float cy = boss.position.y + BossEnemy::HEIGHT * 0.5f;
            Vec2 bc(cx, cy);
            if (distanceSquared(player.position, bc) < bossActR2) {
                boss.activated = true;
                arenaWalls.clear();
                Rect w[4];
                boss.computeArenaWalls(w);
                for (int k = 0; k < 4; k++) {
                    arenaWalls.push_back(w[k]);
                }
            }
        }
    }
#endif

#ifdef PLATFORM_PICO
    {
        Rect arenaBuf[8];
        size_t an = 0;
        for (size_t i = 0; i < arenaWalls.size(); i++) {
            if (arenaWalls.isActive(i) && an < 8) {
                arenaBuf[an++] = arenaWalls[i];
            }
        }
        collision.resolveArenaWalls(player, arenaBuf, an);
    }
#else
    collision.resolveArenaWalls(player, arenaWalls.data(), arenaWalls.size());
#endif
    
    // Update animation/state
    player.update(dt);
    
    // Handle projectile spawning
    if (player.wantsToFire()) {
        Vec2 spawnPos = player.position;
#ifdef PLATFORM_PICO
        spawnPos.x += player.facingRight() ? Player::WIDTH : TO_FIXED(0.0f);
        spawnPos.y += FIXED_DIV(Player::HEIGHT, TO_FIXED(2.0f)) - FIXED_DIV(Projectile::HEIGHT, TO_FIXED(2.0f));
        
        Projectile* newProjectile = projectiles.allocate();
        if (newProjectile) {
            *newProjectile = Projectile(spawnPos, player.facingRight());
        }
#else
        spawnPos.x += player.facingRight() ? Player::WIDTH : 0;
        spawnPos.y += Player::HEIGHT / 2 - Projectile::HEIGHT / 2;
        projectiles.push_back(Projectile(spawnPos, player.facingRight()));
#endif
    }
    
    // Update all projectiles
    int camX = camera.getOffsetX();
    int camY = camera.getOffsetY();
    int screenWidth = renderer->getScreenWidth();
    int screenHeight = renderer->getScreenHeight();
    
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (!projectiles.isActive(i)) continue;
        
        projectiles[i].update(dt);
        
        if (collision.checkProjectileTileCollision(projectiles[i], level)) {
            projectiles[i].shouldDestroy = true;
        }
        
        fixed_t projX = projectiles[i].position.x;
        fixed_t projY = projectiles[i].position.y;
        fixed_t camXFixed = TO_FIXED(camX);
        fixed_t camYFixed = TO_FIXED(camY);
        if (projX < camXFixed || projX > camXFixed + TO_FIXED(screenWidth) ||
            projY < camYFixed || projY > camYFixed + TO_FIXED(screenHeight)) {
            projectiles[i].shouldDestroy = true;
        }
    }
#else
    for (auto& projectile : projectiles) {
        projectile.update(dt);
        
        if (collision.checkProjectileTileCollision(projectile, level)) {
            projectile.shouldDestroy = true;
        }
        
        float projX = projectile.position.x;
        float projY = projectile.position.y;
        if (projX < camX || projX > camX + screenWidth ||
            projY < camY || projY > camY + screenHeight) {
            projectile.shouldDestroy = true;
        }
    }
#endif
    
    // Update basic + ranged enemies (Pico: no distance culling — see Game.h)
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < basicEnemies.count; i++) {
        if (!basicEnemies.isActive(i)) continue;
        basicEnemies[i].update(dt, level);
    }
    
    for (size_t i = 0; i < rangedEnemies.count; i++) {
        if (!rangedEnemies.isActive(i)) continue;
        rangedEnemies[i].update(dt, level, player, enemyProjectiles);
    }
#else
    for (auto& enemy : basicEnemies) {
        float distSq = distanceSquared(player.position, enemy.position);
        if (distSq < DEACTIVATION_DISTANCE * DEACTIVATION_DISTANCE) {
            enemy.update(dt, level);
        }
    }
    for (auto& enemy : rangedEnemies) {
        float distSq = distanceSquared(player.position, enemy.position);
        if (distSq < DEACTIVATION_DISTANCE * DEACTIVATION_DISTANCE) {
            enemy.update(dt, level, player, enemyProjectiles);
        }
    }
#endif

#ifdef PLATFORM_PICO
    for (size_t i = 0; i < bosses.size(); i++) {
        if (!bosses.isActive(i)) continue;
        BossEnemy& b = bosses[i];
        if (!b.active || !b.activated) continue;
        b.update(dt, level, player);
    }
#else
    for (auto& boss : bosses) {
        if (!boss.active || !boss.activated) continue;
        boss.update(dt, level, player);
    }
#endif
    
    // Check enemy collision damage
    if (player.invincibilityTimer <= 0.0f) {
        Rect playerRect = player.getCollider();
        
#ifdef PLATFORM_PICO
        for (size_t i = 0; i < basicEnemies.size(); i++) {
            if (!basicEnemies.isActive(i)) continue;
            if (playerRect.intersects(basicEnemies[i].getCollider())) {
                if (player.health > 0) {
                    player.health -= 1;
                    notifyPlayerDamageHaptics();
                }
                player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                break;
            }
        }
        
        if (player.invincibilityTimer <= 0.0f) {
            for (size_t i = 0; i < rangedEnemies.size(); i++) {
                if (!rangedEnemies.isActive(i)) continue;
                if (playerRect.intersects(rangedEnemies[i].getCollider())) {
                    if (player.health > 0) {
                        player.health -= 1;
                        notifyPlayerDamageHaptics();
                    }
                    player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                    break;
                }
            }
        }
#else
        for (const auto& enemy : basicEnemies) {
            if (playerRect.intersects(enemy.getCollider())) {
                if (player.health > 0) {
                    player.health -= 1;
                    notifyPlayerDamageHaptics();
                }
                player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                break;
            }
        }
        
        if (player.invincibilityTimer <= 0.0f) {
            for (const auto& enemy : rangedEnemies) {
                if (playerRect.intersects(enemy.getCollider())) {
                    if (player.health > 0) {
                        player.health -= 1;
                        notifyPlayerDamageHaptics();
                    }
                    player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                    break;
                }
            }
        }
#endif
    }
    
    // Update all enemy projectiles
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < enemyProjectiles.size(); i++) {
        if (!enemyProjectiles.isActive(i)) continue;
        
        enemyProjectiles[i].update(dt);
        
        if (collision.checkEnemyProjectileTileCollision(enemyProjectiles[i], level)) {
            enemyProjectiles[i].shouldDestroy = true;
        }
        
        fixed_t projX = enemyProjectiles[i].position.x;
        fixed_t projY = enemyProjectiles[i].position.y;
        fixed_t camXFixed = TO_FIXED(camX);
        fixed_t camYFixed = TO_FIXED(camY);
        if (projX < camXFixed || projX > camXFixed + TO_FIXED(screenWidth) ||
            projY < camYFixed || projY > camYFixed + TO_FIXED(screenHeight)) {
            enemyProjectiles[i].shouldDestroy = true;
        }
    }
#else
    for (auto& projectile : enemyProjectiles) {
        projectile.update(dt);
        
        if (collision.checkEnemyProjectileTileCollision(projectile, level)) {
            projectile.shouldDestroy = true;
        }
        
        float projX = projectile.position.x;
        float projY = projectile.position.y;
        if (projX < camX || projX > camX + screenWidth ||
            projY < camY || projY > camY + screenHeight) {
            projectile.shouldDestroy = true;
        }
    }
#endif
    
    // Update all health packs
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < healthPacks.size(); i++) {
        if (!healthPacks.isActive(i)) continue;
        healthPacks[i].update(dt);
    }
#else
    for (auto& healthPack : healthPacks) {
        healthPack.update(dt);
    }
#endif
    
    // Update HP UI animation (desktop only; Pico uses a static frame in render)
#ifndef PLATFORM_PICO
    hpUIAnimTimer += dt;
    if (hpUIAnimTimer >= 0.1667f) {  // 6 FPS
        hpUIAnimTimer -= 0.1667f;
        hpUIFrame = (hpUIFrame + 1) % 12;
    }
#endif
    
    // Update portal animation
    portalAnimTimer += dt;
    if (portalAnimTimer >= 0.1667f) {  // 6 FPS
        portalAnimTimer -= 0.1667f;
        portalFrame = (portalFrame + 1) % 12;
    }

    if (bossSeanSpritesheet >= 0 && levelHasBoss) {
        bool anyBossVisible = false;
#ifdef PLATFORM_PICO
        for (size_t i = 0; i < bosses.size(); i++) {
            if (!bosses.isActive(i)) {
                continue;
            }
            if (bosses[i].active && bosses[i].activated) {
                anyBossVisible = true;
                break;
            }
        }
#else
        for (const auto& b : bosses) {
            if (b.active && b.activated) {
                anyBossVisible = true;
                break;
            }
        }
#endif
        if (anyBossVisible) {
            bossSeanAnimTimer += dt;
            if (bossSeanAnimTimer >= 0.1f) {
                bossSeanAnimTimer -= 0.1f;
                bossSeanFrame = (bossSeanFrame + 1) % 12;
            }
        }
    }

    // Check player projectile vs basic enemy collisions
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (!projectiles.isActive(i)) continue;
        for (size_t j = 0; j < basicEnemies.size(); j++) {
            if (!basicEnemies.isActive(j)) continue;
            if (projectiles[i].getCollider().intersects(basicEnemies[j].getCollider())) {
                projectiles[i].shouldDestroy = true;
                basicEnemies[j].takeDamage(1);
                break;
            }
        }
    }
    
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (!projectiles.isActive(i)) continue;
        for (size_t j = 0; j < rangedEnemies.size(); j++) {
            if (!rangedEnemies.isActive(j)) continue;
            if (projectiles[i].getCollider().intersects(rangedEnemies[j].getCollider())) {
                projectiles[i].shouldDestroy = true;
                rangedEnemies[j].takeDamage(1);
                break;
            }
        }
    }
#else
    for (auto& projectile : projectiles) {
        for (auto& enemy : basicEnemies) {
            if (projectile.getCollider().intersects(enemy.getCollider())) {
                projectile.shouldDestroy = true;
                enemy.takeDamage(1);
                break;
            }
        }
    }
    
    for (auto& projectile : projectiles) {
        for (auto& enemy : rangedEnemies) {
            if (projectile.getCollider().intersects(enemy.getCollider())) {
                projectile.shouldDestroy = true;
                enemy.takeDamage(1);
                break;
            }
        }
    }
#endif

#ifdef PLATFORM_PICO
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (!projectiles.isActive(i)) continue;
        for (size_t j = 0; j < bosses.size(); j++) {
            if (!bosses.isActive(j)) continue;
            BossEnemy& b = bosses[j];
            if (!b.active || !b.activated) continue;
            if (projectiles[i].getCollider().intersects(b.getCollider())) {
                projectiles[i].shouldDestroy = true;
                b.takeDamage(1);
                break;
            }
        }
    }
#else
    for (auto& projectile : projectiles) {
        for (auto& b : bosses) {
            if (!b.active || !b.activated) continue;
            if (projectile.getCollider().intersects(b.getCollider())) {
                projectile.shouldDestroy = true;
                b.takeDamage(1);
                break;
            }
        }
    }
#endif
    
    // Check enemy projectile vs player collisions
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < enemyProjectiles.size(); i++) {
        if (!enemyProjectiles.isActive(i)) continue;
        if (enemyProjectiles[i].getCollider().intersects(player.getCollider())) {
            enemyProjectiles[i].shouldDestroy = true;
            if (player.invincibilityTimer <= 0.0f) {
                if (player.health > 0) {
                    player.health -= 1;
                    notifyPlayerDamageHaptics();
                }
                player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
            }
        }
    }
    
    for (size_t i = 0; i < healthPacks.size(); i++) {
        if (!healthPacks.isActive(i)) continue;
        if (healthPacks[i].active && healthPacks[i].getCollider().intersects(player.getCollider())) {
            player.health += 1;
            healthPacks[i].active = false;
        }
    }
    
    for (size_t i = 0; i < objectives.size(); i++) {
        if (!objectives.isActive(i)) continue;
        if (!objectives[i].collected && objectives[i].getCollider().intersects(player.getCollider())) {
            objectives[i].collected = true;
            levelObjectiveCollected = true;
        }
    }
#else
    for (auto& projectile : enemyProjectiles) {
        if (projectile.getCollider().intersects(player.getCollider())) {
            projectile.shouldDestroy = true;
            if (player.invincibilityTimer <= 0.0f) {
                if (player.health > 0) {
                    player.health -= 1;
                    notifyPlayerDamageHaptics();
                }
                player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
            }
        }
    }
    
    for (auto& healthPack : healthPacks) {
        if (healthPack.active && healthPack.getCollider().intersects(player.getCollider())) {
            player.health += 1;
            healthPack.active = false;
        }
    }
    
    for (auto& objective : objectives) {
        if (!objective.collected && objective.getCollider().intersects(player.getCollider())) {
            objective.collected = true;
            levelObjectiveCollected = true;
        }
    }
#endif
    
    // Remove destroyed/dead entities
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (projectiles.isActive(i) && projectiles[i].shouldDestroy) {
            projectiles.free(i);
        }
    }
    
    for (size_t i = 0; i < enemyProjectiles.size(); i++) {
        if (enemyProjectiles.isActive(i) && enemyProjectiles[i].shouldDestroy) {
            enemyProjectiles.free(i);
        }
    }
    
    for (size_t i = 0; i < basicEnemies.size(); i++) {
        if (basicEnemies.isActive(i) && basicEnemies[i].health <= 0) {
            basicEnemies.free(i);
        }
    }
    
    for (size_t i = 0; i < rangedEnemies.size(); i++) {
        if (rangedEnemies.isActive(i) && rangedEnemies[i].health <= 0) {
            rangedEnemies.free(i);
        }
    }
    
    for (size_t i = 0; i < bosses.size(); i++) {
        if (bosses.isActive(i) && !bosses[i].active) {
            arenaWalls.clear();
            bosses.free(i);
        }
    }
#else
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.shouldDestroy; }),
        projectiles.end()
    );
    
    enemyProjectiles.erase(
        std::remove_if(enemyProjectiles.begin(), enemyProjectiles.end(),
            [](const EnemyProjectile& p) { return p.shouldDestroy; }),
        enemyProjectiles.end()
    );
    
    basicEnemies.erase(
        std::remove_if(basicEnemies.begin(), basicEnemies.end(),
            [](const BasicEnemy& e) { return e.health <= 0; }),
        basicEnemies.end()
    );
    
    rangedEnemies.erase(
        std::remove_if(rangedEnemies.begin(), rangedEnemies.end(),
            [](const RangedEnemy& e) { return e.health <= 0; }),
        rangedEnemies.end()
    );
    
    {
        bool removedBoss = false;
        bosses.erase(
            std::remove_if(bosses.begin(), bosses.end(),
                [&removedBoss](BossEnemy& b) {
                    if (!b.active) {
                        removedBoss = true;
                        return true;
                    }
                    return false;
                }),
            bosses.end()
        );
        if (removedBoss) {
            arenaWalls.clear();
        }
    }
#endif
    
    // Camera follow
    camera.follow(player, level);
    
    // Check portal collisions
    checkPortalCollisions();
    
    if (player.health <= 0) {
        state = GameState::GameOver;
        gameOverFrame = 0;
        gameOverAnimTimer = 0.0f;
    }
}

void Game::notifyPlayerDamageHaptics() {
    haptics->trigger(HapticEffect::Medium, 100);
}

void Game::checkPortalCollisions() {
    Rect playerRect = player.getCollider();
    
#ifdef PLATFORM_PICO
    const Portal* portals = level.getPortals();
    uint8_t portalCount = level.getPortalCount();
    
    for (uint8_t i = 0; i < portalCount; i++) {
        if (playerRect.intersects(portals[i].bounds)) {
            bool hasObjectives = false;
            for (size_t j = 0; j < objectives.size(); j++) {
                if (objectives.isActive(j)) {
                    hasObjectives = true;
                    break;
                }
            }
            if (hasObjectives && !levelObjectiveCollected) {
                return;
            }
            if (hasAliveBoss()) {
                return;
            }
            loadLevel(portals[i].targetLevel);
            break;
        }
    }
#else
    const std::vector<Portal>& portals = level.getPortals();
    
    for (const Portal& portal : portals) {
        if (playerRect.intersects(portal.bounds)) {
            if (!objectives.empty() && !levelObjectiveCollected) {
                return;
            }
            if (hasAliveBoss()) {
                return;
            }
            loadLevel(portal.targetLevel);
            break;
        }
    }
#endif
}

bool Game::hasAliveBoss() const {
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < bosses.size(); i++) {
        if (bosses.isActive(i) && bosses[i].active) {
            return true;
        }
    }
    return false;
#else
    for (const auto& b : bosses) {
        if (b.active) {
            return true;
        }
    }
    return false;
#endif
}

void Game::renderPlayingWorld() {
    int camX = camera.getOffsetX();
    int camY = camera.getOffsetY();
    
    // Calculate visible tile range
    int screenWidth = renderer->getScreenWidth();
    int screenHeight = renderer->getScreenHeight();
    int tileSize = level.getTileSize();
    
    int startTileX = camX / tileSize;
    int startTileY = camY / tileSize;
    int endTileX = (camX + screenWidth) / tileSize + 1;
    int endTileY = (camY + screenHeight) / tileSize + 1;
    
    // Clamp to level boundaries
    startTileX = std::max(0, startTileX);
    startTileY = std::max(0, startTileY);
    endTileX = std::min(level.getWidthInTiles(), endTileX);
    endTileY = std::min(level.getHeightInTiles(), endTileY);
    
    // Draw only visible level tiles
    for (int y = startTileY; y < endTileY; y++) {
        for (int x = startTileX; x < endTileX; x++) {
            int8_t tileId = level.getTileId(x, y);
            if (tileId != -1) {
                renderer->drawTile(x, y, tileId, terrainSpritesheet, camX, camY);
            }
        }
    }

    // Draw portals with animation
#ifdef PLATFORM_PICO
    const Portal* portals = level.getPortals();
    uint8_t portalCount = level.getPortalCount();
    for (uint8_t i = 0; i < portalCount; i++) {
        Rect dstRect = portals[i].bounds;
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        
        if (portalSpritesheet >= 0) {
            renderer->drawSpriteFrame(portalSpritesheet, portalFrame, 32, 32, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(0, 255, 255), true);
        }
    }
#else
    const std::vector<Portal>& portals = level.getPortals();
    for (const Portal& portal : portals) {
        Rect dstRect = portal.bounds;
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        if (portalSpritesheet >= 0) {
            renderer->drawSpriteFrame(portalSpritesheet, portalFrame, 32, 32, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(0, 255, 255), true);
        }
    }
#endif
    
    // Draw basic enemies
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < basicEnemies.size(); i++) {
        if (!basicEnemies.isActive(i)) continue;
        
        Rect dstRect = basicEnemies[i].getCollider();
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        
        if (basicEnemySpritesheet >= 0) {
            renderer->drawSpriteFrame(basicEnemySpritesheet, basicEnemies[i].currentFrame, 16, 16, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 0, 0), true);
        }
    }
    
    for (size_t i = 0; i < rangedEnemies.size(); i++) {
        if (!rangedEnemies.isActive(i)) continue;
        
        Rect dstRect = rangedEnemies[i].getCollider();
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        
        if (rangedEnemySpritesheet >= 0) {
            renderer->drawSpriteFrame(rangedEnemySpritesheet, rangedEnemies[i].currentFrame, 16, 16, dstRect, rangedEnemies[i].facingRight());
        } else {
            renderer->drawRect(dstRect, Color(255, 128, 0), true);
        }
    }

    for (size_t i = 0; i < bosses.size(); i++) {
        if (!bosses.isActive(i)) continue;
        if (!bosses[i].active || !bosses[i].activated) continue;
        Rect dstRect = bosses[i].getCollider();
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        if (bossSeanSpritesheet >= 0 && levelHasBoss) {
            renderer->drawSpriteFrame(bossSeanSpritesheet, bossSeanFrame, 32, 32, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 0, 255), true);
        }
    }
#else
    for (const auto& enemy : basicEnemies) {
        Rect dstRect = enemy.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        if (basicEnemySpritesheet >= 0) {
            renderer->drawSpriteFrame(basicEnemySpritesheet, enemy.currentFrame, 16, 16, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 0, 0), true);
        }
    }
    
    for (const auto& enemy : rangedEnemies) {
        Rect dstRect = enemy.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        if (rangedEnemySpritesheet >= 0) {
            renderer->drawSpriteFrame(rangedEnemySpritesheet, enemy.currentFrame, 16, 16, dstRect, enemy.facingRight());
        } else {
            renderer->drawRect(dstRect, Color(255, 128, 0), true);
        }
    }

    for (const auto& boss : bosses) {
        if (!boss.active || !boss.activated) continue;
        Rect dstRect = boss.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        if (bossSeanSpritesheet >= 0 && levelHasBoss) {
            renderer->drawSpriteFrame(bossSeanSpritesheet, bossSeanFrame, 32, 32, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 0, 255), true);
        }
    }
#endif
    
    // Draw health packs
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < healthPacks.size(); i++) {
        if (!healthPacks.isActive(i)) continue;
        if (healthPacks[i].active) {
            healthPacks[i].render(renderer, healthPackSpritesheet, camX, camY);
        }
    }
    
    for (size_t i = 0; i < objectives.size(); i++) {
        if (!objectives.isActive(i)) continue;
        if (!objectives[i].collected) {
            int spritesheet = getObjectiveSpritesheet(objectives[i].type);
            objectives[i].render(renderer, spritesheet, camX, camY);
        }
    }
#else
    for (const auto& healthPack : healthPacks) {
        if (healthPack.active) {
            healthPack.render(renderer, healthPackSpritesheet, camX, camY);
        }
    }
    
    for (const auto& objective : objectives) {
        if (!objective.collected) {
            int spritesheet = getObjectiveSpritesheet(objective.type);
            objective.render(renderer, spritesheet, camX, camY);
        }
    }
#endif
    
    // Draw enemy projectiles
#ifdef PLATFORM_PICO
    for (size_t i = 0; i < enemyProjectiles.size(); i++) {
        if (!enemyProjectiles.isActive(i)) continue;
        
        Rect dstRect = enemyProjectiles[i].getCollider();
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        
        int spritesheet = enemyProjectiles[i].movingRight ? enemyProjectileRightSpritesheet : enemyProjectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            renderer->drawSpriteFrame(spritesheet, enemyProjectiles[i].currentFrame, 16, 16, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(128, 0, 255), true);
        }
    }
    
    for (size_t i = 0; i < projectiles.size(); i++) {
        if (!projectiles.isActive(i)) continue;
        
        Rect dstRect = projectiles[i].getCollider();
        dstRect.x -= TO_FIXED(camX);
        dstRect.y -= TO_FIXED(camY);
        
        int spritesheet = projectiles[i].movingRight ? projectileRightSpritesheet : projectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            renderer->drawSpriteFrame(spritesheet, projectiles[i].currentFrame, 8, 8, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 200, 0), true);
        }
    }
#else
    for (const auto& projectile : enemyProjectiles) {
        Rect dstRect = projectile.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        int spritesheet = projectile.movingRight ? enemyProjectileRightSpritesheet : enemyProjectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            renderer->drawSpriteFrame(spritesheet, projectile.currentFrame, 16, 16, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(128, 0, 255), true);
        }
    }
    
    for (const auto& projectile : projectiles) {
        Rect dstRect = projectile.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        int spritesheet = projectile.movingRight ? projectileRightSpritesheet : projectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            renderer->drawSpriteFrame(spritesheet, projectile.currentFrame, 8, 8, dstRect, false);
        } else {
            renderer->drawRect(dstRect, Color(255, 200, 0), true);
        }
    }
#endif
    
    // Draw player sprite
    Rect dstRect = player.getCollider();
#ifdef PLATFORM_PICO
    dstRect.x -= TO_FIXED(camX);
    dstRect.y -= TO_FIXED(camY);
#else
    dstRect.x -= camX;
    dstRect.y -= camY;
#endif
    
    if (playerSpritesheet >= 0) {
        renderer->drawSpriteFrame(playerSpritesheet, player.currentFrame, 16, 16, dstRect, !player.facingRight());
    } else {
        // Fallback if texture fails to load
        renderer->drawRect(dstRect, Color(255, 0, 0), true);
    }
    
    // Draw HP counter UI in top-left corner (Pico: static icon frame; redraw every frame after world
    // because beginFrame clears the full screen and tiles can overlap the HUD area.)
#ifdef PLATFORM_PICO
    constexpr int kHpUiFramePico = 0;
#endif
    if (energySpritesheet >= 0) {
        Rect hpIconDst(9.0f, 9.0f, 16.0f, 16.0f);
#ifdef PLATFORM_PICO
        renderer->drawSpriteFrame(energySpritesheet, kHpUiFramePico, 16, 16, hpIconDst, false);
#else
        renderer->drawSpriteFrame(energySpritesheet, hpUIFrame, 16, 16, hpIconDst, false);
#endif
    } else {
        Rect hpIconRect(9.0f, 9.0f, 16.0f, 16.0f);
        renderer->drawRect(hpIconRect, Color(255, 0, 0), true);
    }
    
#ifdef PLATFORM_PICO
    char hpText[8];
    snprintf(hpText, sizeof(hpText), "%dx", player.health);
    renderer->drawText(hpText, 28, 9, Color(255, 255, 255));
#else
    std::string hpText = std::to_string(player.health) + "x";
    renderer->drawText(hpText.c_str(), 28, 10, Color(255, 255, 255));
#endif
    
    // Draw objective UI if level has objectives (Pico: redraw each frame; same full-clear + overlap as HP HUD.)
#ifdef PLATFORM_PICO
    bool hasObjective = false;
    size_t objectiveIdx = 0;
    for (size_t i = 0; i < objectives.size(); i++) {
        if (objectives.isActive(i)) {
            hasObjective = true;
            objectiveIdx = i;
            break;
        }
    }
    if (hasObjective) {
        const Objective& objective = objectives[objectiveIdx];
        int spritesheet = getObjectiveSpritesheet(objective.type);
#else
    if (!objectives.empty()) {
        const Objective& objective = objectives[0];
        int spritesheet = getObjectiveSpritesheet(objective.type);
#endif
        
        // Draw objective sprite icon
        Rect iconDst(8, 28, 16, 16);
        if (spritesheet >= 0) {
            renderer->drawSpriteFrame(spritesheet, 0, 16, 16, iconDst, false);
        } else {
            renderer->drawRect(iconDst, Color(255, 255, 0), true);
        }
        
        // Draw collection status
#ifdef PLATFORM_PICO
        const char* statusText = levelObjectiveCollected ? "1/1" : "0/1";
#else
        std::string statusText = levelObjectiveCollected ? "1/1" : "0/1";
#endif
    #ifdef PLATFORM_PICO
        renderer->drawText(statusText, 28, 28, Color(255, 255, 255));
    #else
        renderer->drawText(statusText.c_str(), 28, 30, Color(255, 255, 255));
    #endif
    }

    if (levelHasBoss) {
        Rect bossIconDst(8.0f, 47.0f, 16.0f, 16.0f);
        if (bossIconSpritesheet >= 0) {
            renderer->drawSpriteFrame(bossIconSpritesheet, 0, 16, 16, bossIconDst, false);
        } else {
            renderer->drawRect(bossIconDst, Color(255, 0, 0), true);
        }
#ifdef PLATFORM_PICO
        const char* bossStatusText = hasAliveBoss() ? "0/1" : "1/1";
        renderer->drawText(bossStatusText, 28, 47, Color(255, 255, 255));
#else
        std::string bossStatusText = hasAliveBoss() ? "0/1" : "1/1";
        renderer->drawText(bossStatusText.c_str(), 28, 48, Color(255, 255, 255));
#endif
    }
}

void Game::renderMainMenu() {
    const int sw = renderer->getScreenWidth();

    Rect title = menuTitlePlaceholderRect();
    if (titleSpritesheet >= 0) {
        renderer->drawSpriteFrame(titleSpritesheet, 0, 128, 32, title, false);
    } else {
        renderer->drawRect(title, Color(180, 180, 200), true);
        renderer->drawRect(title, Color(0, 0, 0), false);
    }

    const Rect btn = menuStartButtonRect();
    renderer->drawRect(btn, Color(64, 64, 64), true);
    renderer->drawRect(btn, Color(0, 0, 0), false);

    const char* startLabel = "PLAY";
    const int w0 = renderer->measureTextWidth(startLabel);
#ifdef PLATFORM_PICO
    // Rect uses fixed_t on Pico; cast to int is wrong (raw fixed units). Match HUD cell height 8*2=16.
    const int btnTop = static_cast<int>(FROM_FIXED(btn.y));
    const int btnH = static_cast<int>(FROM_FIXED(btn.height));
    const int txtY = btnTop + (btnH - 16) / 2;
#else
    const int txtY = static_cast<int>(btn.y) + (static_cast<int>(btn.height) - 12) / 2;
#endif
    renderer->drawText(startLabel, (sw - w0) / 2, txtY, Color(255, 255, 255));
}

void Game::renderGameOver() {
    renderer->clear(Color(0, 0, 0));

    const int sw = renderer->getScreenWidth();

    const char* msg = "GAME OVER";
    const int msgW = renderer->measureTextWidth(msg);
    const int msgY = 40;
    renderer->drawText(msg, (sw - msgW) / 2, msgY, Color(255, 255, 255));

    Rect iconRect(static_cast<float>(sw / 2 - 16), static_cast<float>(msgY + 28), 32.0f, 32.0f);
    if (gameOverSpritesheet >= 0) {
        renderer->drawSpriteFrame(gameOverSpritesheet, gameOverFrame, 32, 32, iconRect, false);
    } else {
        renderer->drawRect(iconRect, Color(200, 200, 200), true);
        renderer->drawRect(iconRect, Color(0, 0, 0), false);
    }

    const Rect retryBtn = gameOverRetryButtonRect();
    renderer->drawRect(retryBtn, Color(64, 64, 64), true);
    renderer->drawRect(retryBtn, Color(0, 0, 0), false);

    const char* retryLabel = "RETRY";
    const int w1 = renderer->measureTextWidth(retryLabel);
#ifdef PLATFORM_PICO
    const int retryTop = static_cast<int>(FROM_FIXED(retryBtn.y));
    const int retryH = static_cast<int>(FROM_FIXED(retryBtn.height));
    const int txtY = retryTop + (retryH - 16) / 2;
#else
    const int txtY = static_cast<int>(retryBtn.y) + (static_cast<int>(retryBtn.height) - 12) / 2;
#endif
    renderer->drawText(retryLabel, (sw - w1) / 2, txtY, Color(255, 255, 255));
}

Rect Game::menuTitlePlaceholderRect() const {
    const int sw = renderer->getScreenWidth();
    constexpr int titleW = 128;
    constexpr int titleH = 32;
    constexpr int titleY = 60;
    const int titleX = (sw - titleW) / 2;
    return Rect(titleX, titleY, titleW, titleH);
}

Rect Game::menuStartButtonRect() const {
    const int sw = renderer->getScreenWidth();
    constexpr int titleW = 128;
    constexpr int titleH = 32;
    constexpr int titleY = 60;
    constexpr int btnW = 100;
    constexpr int btnH = 28;
    const int btnY = titleY + titleH + 16;
    const int btnX = (sw - btnW) / 2;
    return Rect(btnX, btnY, btnW, btnH);
}

Rect Game::gameOverRetryButtonRect() const {
    const int sw = renderer->getScreenWidth();
    const int msgY = 40;
    const int iconY = msgY + 28;
    constexpr int btnW = 100;
    constexpr int btnH = 28;
    const int btnY = iconY + 32 + 16;
    const int btnX = (sw - btnW) / 2;
    return Rect(btnX, btnY, btnW, btnH);
}

void Game::render() {
    renderer->beginFrame();
    switch (state) {
    case GameState::MainMenu:
        renderMainMenu();
        break;
    case GameState::GameOver:
        renderGameOver();
        break;
    case GameState::Playing:
        renderPlayingWorld();
        break;
    }
    renderer->endFrame();
}

int Game::getObjectiveSpritesheet(ObjectiveType type) const {
    switch (type) {
        case ObjectiveType::CHARGER: return chargerSpritesheet;
        case ObjectiveType::ENCLOSURE: return enclosureSpritesheet;
        case ObjectiveType::HAPTIC: return hapticSpritesheet;
        case ObjectiveType::PARTS: return partsSpritesheet;
        case ObjectiveType::SCREEN: return screenSpritesheet;
        case ObjectiveType::PICO: return picoSpritesheet;
        default: return -1;
    }
}
