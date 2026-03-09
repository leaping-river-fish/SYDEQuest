#include "Game.h"
#include "core/IRenderer.h"
#include "core/IInput.h"
#include "core/IHaptics.h"
#include "core/ITimer.h"
#include <algorithm>
#include <cstring>
#include <string>

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
      hpUIFrame(0),
      hpUIAnimTimer(0.0f)
{
    currentLevelName[0] = '\0';
}

void Game::init() {
    loadLevel("../levels/Level1.csv");
    
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
}

bool Game::loadLevel(const char* levelName) {
    if (!level.loadFromFile(levelName)) {
        return false;
    }
    
    strncpy(currentLevelName, levelName, sizeof(currentLevelName) - 1);
    currentLevelName[sizeof(currentLevelName) - 1] = '\0';
    
    player.position = level.getSpawnPoint();
    player.velocity = Vec2(0, 0);
    player.isGrounded = false;
    
    projectiles.clear();
    basicEnemies.clear();
    rangedEnemies.clear();
    enemyProjectiles.clear();
    healthPacks.clear();
    
    // Spawn basic enemies from level data
    const std::vector<Vec2>& basicEnemySpawnPoints = level.getBasicEnemySpawns();
    for (const Vec2& spawnPos : basicEnemySpawnPoints) {
        basicEnemies.push_back(BasicEnemy(spawnPos, true));
    }
    
    // Spawn ranged enemies from level data
    const std::vector<Vec2>& rangedEnemySpawnPoints = level.getRangedEnemySpawns();
    for (const Vec2& spawnPos : rangedEnemySpawnPoints) {
        rangedEnemies.push_back(RangedEnemy(spawnPos, true));
    }
    
    // Spawn health packs from level data
    const std::vector<Vec2>& healthPackSpawnPoints = level.getHealthPackSpawns();
    for (const Vec2& spawnPos : healthPackSpawnPoints) {
        healthPacks.push_back(HealthPack(spawnPos));
    }
    
    camera.follow(player, level);
    
    return true;
}

void Game::update() {
    timer->update();
    input->update();
    
    float dt = timer->getDeltaTime();
    
    // Input player actions
    controller.update(player, input, dt);
    
    // Physics
    physics.applyGravity(player, dt);
    physics.clampVelocity(player);
    
    // Move X and resolve
    player.position.x += player.velocity.x * dt;
    collision.resolveHorizontal(player, level);
    
    // Move Y and resolve
    player.isGrounded = false;
    player.position.y += player.velocity.y * dt;
    collision.resolveVertical(player, level);
    
    // Update animation/state
    player.update(dt);
    
    // Handle projectile spawning
    if (player.wantsToFire) {
        // Spawn projectile from player center, offset slightly forward
        Vec2 spawnPos = player.position;
        spawnPos.x += player.facingRight ? Player::WIDTH : 0;
        spawnPos.y += Player::HEIGHT / 2 - Projectile::HEIGHT / 2;
        
        projectiles.push_back(Projectile(spawnPos, player.facingRight));
    }
    
    // Update all projectiles
    int camX = camera.getOffsetX();
    int camY = camera.getOffsetY();
    int screenWidth = renderer->getScreenWidth();
    int screenHeight = renderer->getScreenHeight();
    
    for (auto& projectile : projectiles) {
        projectile.update(dt);
        
        // Check for tile collision
        if (collision.checkProjectileTileCollision(projectile, level)) {
            projectile.shouldDestroy = true;
        }
        
        // Check if projectile is outside camera view
        float projX = projectile.position.x;
        float projY = projectile.position.y;
        if (projX < camX || projX > camX + screenWidth ||
            projY < camY || projY > camY + screenHeight) {
            projectile.shouldDestroy = true;
        }
    }
    
    // Update all basic enemies
    for (auto& enemy : basicEnemies) {
        enemy.update(dt, level);
    }
    
    // Update all ranged enemies (pass player position and projectiles vector)
    for (auto& enemy : rangedEnemies) {
        enemy.update(dt, level, player, enemyProjectiles);
    }
    
    // Check enemy collision damage
    if (player.invincibilityTimer <= 0.0f) {
        Rect playerRect = player.getCollider();
        
        // Check basic enemy collisions
        for (const auto& enemy : basicEnemies) {
            if (playerRect.intersects(enemy.getCollider())) {
                player.health -= 1;
                player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                break;
            }
        }
        
        // Check ranged enemy collisions (only if still not invincible)
        if (player.invincibilityTimer <= 0.0f) {
            for (const auto& enemy : rangedEnemies) {
                if (playerRect.intersects(enemy.getCollider())) {
                    player.health -= 1;
                    player.invincibilityTimer = Player::INVINCIBILITY_DURATION;
                    break;
                }
            }
        }
    }
    
    // Update all enemy projectiles
    for (auto& projectile : enemyProjectiles) {
        projectile.update(dt);
        
        // Check for tile collision (solid blocks only, not platforms)
        if (collision.checkEnemyProjectileTileCollision(projectile, level)) {
            projectile.shouldDestroy = true;
        }
        
        // Check if projectile is outside camera view
        float projX = projectile.position.x;
        float projY = projectile.position.y;
        if (projX < camX || projX > camX + screenWidth ||
            projY < camY || projY > camY + screenHeight) {
            projectile.shouldDestroy = true;
        }
    }
    
    // Update all health packs
    for (auto& healthPack : healthPacks) {
        healthPack.update(dt);
    }
    
    // Update HP UI animation
    hpUIAnimTimer += dt;
    if (hpUIAnimTimer >= 0.1667f) {  // 6 FPS
        hpUIAnimTimer -= 0.1667f;
        hpUIFrame = (hpUIFrame + 1) % 12;
    }
    
    // Check player projectile vs basic enemy collisions
    for (auto& projectile : projectiles) {
        for (auto& enemy : basicEnemies) {
            if (projectile.getCollider().intersects(enemy.getCollider())) {
                projectile.shouldDestroy = true;
                enemy.takeDamage(1);
                break;
            }
        }
    }
    
    // Check player projectile vs ranged enemy collisions
    for (auto& projectile : projectiles) {
        for (auto& enemy : rangedEnemies) {
            if (projectile.getCollider().intersects(enemy.getCollider())) {
                projectile.shouldDestroy = true;
                enemy.takeDamage(1);
                break;
            }
        }
    }
    
    // Check enemy projectile vs player collisions
    for (auto& projectile : enemyProjectiles) {
        if (projectile.getCollider().intersects(player.getCollider())) {
            projectile.shouldDestroy = true;
            player.health -= 1;
        }
    }
    
    // Check health pack collisions
    for (auto& healthPack : healthPacks) {
        if (healthPack.active && healthPack.getCollider().intersects(player.getCollider())) {
            player.health += 1;
            healthPack.active = false;
        }
    }
    
    // Remove destroyed projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.shouldDestroy; }),
        projectiles.end()
    );
    
    // Remove destroyed enemy projectiles
    enemyProjectiles.erase(
        std::remove_if(enemyProjectiles.begin(), enemyProjectiles.end(),
            [](const EnemyProjectile& p) { return p.shouldDestroy; }),
        enemyProjectiles.end()
    );
    
    // Remove dead basic enemies
    basicEnemies.erase(
        std::remove_if(basicEnemies.begin(), basicEnemies.end(),
            [](const BasicEnemy& e) { return e.health <= 0; }),
        basicEnemies.end()
    );
    
    // Remove dead ranged enemies
    rangedEnemies.erase(
        std::remove_if(rangedEnemies.begin(), rangedEnemies.end(),
            [](const RangedEnemy& e) { return e.health <= 0; }),
        rangedEnemies.end()
    );
    
    // Camera follow
    camera.follow(player, level);
    
    // Check portal collisions
    checkPortalCollisions();
    
    // Check player death
    if (player.health <= 0) {
        loadLevel(currentLevelName);
        player.health = 1;
    }
}

void Game::checkPortalCollisions() {
    Rect playerRect = player.getCollider();
    const std::vector<Portal>& portals = level.getPortals();
    
    for (const Portal& portal : portals) {
        if (playerRect.intersects(portal.bounds)) {
            loadLevel(portal.targetLevel);
            break;
        }
    }
}

void Game::render() {
    renderer->beginFrame();
    
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
    
    // Draw basic enemies
    for (const auto& enemy : basicEnemies) {
        Rect dstRect = enemy.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        if (basicEnemySpritesheet >= 0) {
            // Calculate source rect (12 frames in 4x3 grid, 16x16 each)
            int frameX = (enemy.currentFrame % 4) * 16;
            int frameY = (enemy.currentFrame / 4) * 16;
            Rect srcRect(frameX, frameY, 16, 16);
            
            renderer->drawSprite(basicEnemySpritesheet, srcRect, dstRect, false);
        } else {
            // Fallback to red rectangle if texture fails to load
            renderer->drawRect(dstRect, Color(255, 0, 0), true);
        }
    }
    
    // Draw ranged enemies
    for (const auto& enemy : rangedEnemies) {
        Rect dstRect = enemy.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        if (rangedEnemySpritesheet >= 0) {
            // Calculate source rect (12 frames in 4x3 grid, 16x16 each)
            int frameX = (enemy.currentFrame % 4) * 16;
            int frameY = (enemy.currentFrame / 4) * 16;
            Rect srcRect(frameX, frameY, 16, 16);
            
            // Flip sprite based on facing direction
            renderer->drawSprite(rangedEnemySpritesheet, srcRect, dstRect, enemy.facingRight);
        } else {
            // Fallback to orange rectangle if texture fails to load
            renderer->drawRect(dstRect, Color(255, 128, 0), true);
        }
    }
    
    // Draw health packs
    for (const auto& healthPack : healthPacks) {
        if (healthPack.active) {
            healthPack.render(renderer, healthPackSpritesheet, camX, camY);
        }
    }
    
    // Draw enemy projectiles
    for (const auto& projectile : enemyProjectiles) {
        Rect dstRect = projectile.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        // Choose sprite sheet based on direction
        int spritesheet = projectile.movingRight ? enemyProjectileRightSpritesheet : enemyProjectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            // Calculate source rect (12 frames in 4x3 grid, 16x16 each)
            int frameX = (projectile.currentFrame % 4) * 16;
            int frameY = (projectile.currentFrame / 4) * 16;
            Rect srcRect(frameX, frameY, 16, 16);
            
            renderer->drawSprite(spritesheet, srcRect, dstRect, false);
        } else {
            // Fallback to purple rectangle if textures fail to load
            renderer->drawRect(dstRect, Color(128, 0, 255), true);
        }
    }
    
    // Draw projectiles
    for (const auto& projectile : projectiles) {
        Rect dstRect = projectile.getCollider();
        dstRect.x -= camX;
        dstRect.y -= camY;
        
        // Choose sprite sheet based on direction
        int spritesheet = projectile.movingRight ? projectileRightSpritesheet : projectileLeftSpritesheet;
        
        if (spritesheet >= 0) {
            // Calculate source rect (12 frames in 4x3 grid, 8x8 each)
            int frameX = (projectile.currentFrame % 4) * 8;
            int frameY = (projectile.currentFrame / 4) * 8;
            Rect srcRect(frameX, frameY, 8, 8);
            
            renderer->drawSprite(spritesheet, srcRect, dstRect, false);
        } else {
            // Fallback to rectangle if textures fail to load
            renderer->drawRect(dstRect, Color(255, 200, 0), true);
        }
    }
    
    // Draw player sprite
    Rect dstRect = player.getCollider();
    dstRect.x -= camX;
    dstRect.y -= camY;
    
    if (playerSpritesheet >= 0) {
        int frameX = (player.currentFrame % 4) * 16;
        int frameY = (player.currentFrame / 4) * 16;
        Rect srcRect(frameX, frameY, 16, 16);
        
        renderer->drawSprite(playerSpritesheet, srcRect, dstRect, !player.facingRight);
    } else {
        // Fallback if texture fails to load
        renderer->drawRect(dstRect, Color(255, 0, 0), true);
    }
    
    // Draw HP counter UI in top-left corner
    if (energySpritesheet >= 0) {
        int frameX = (hpUIFrame % 4) * 16;
        int frameY = (hpUIFrame / 4) * 16;
        Rect srcRect(frameX, frameY, 16, 16);
        Rect dstRect(8, 8, 16, 16);
        
        renderer->drawSprite(energySpritesheet, srcRect, dstRect, false);
    } else {
        Rect hpIconRect(8, 8, 16, 16);
        renderer->drawRect(hpIconRect, Color(255, 0, 0), true);
    }
    
    std::string hpText = std::to_string(player.health) + "x";
    renderer->drawText(hpText, 28, 10, Color(255, 255, 255));
    
    renderer->endFrame();
}

