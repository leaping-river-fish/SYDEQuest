#include "Game.h"
#include "core/IRenderer.h"
#include "core/IInput.h"
#include "core/IHaptics.h"
#include "core/ITimer.h"
#include <algorithm>

Game::Game(IRenderer* r, IInput* i, IHaptics* h, ITimer* t)
    : renderer(r), input(i), haptics(h), timer(t),
      camera(r->getScreenWidth(), r->getScreenHeight()),
      playerSpritesheet(-1),
      projectileLeftSpritesheet(-1),
      projectileRightSpritesheet(-1)
{
}

void Game::init() {
    // Position player at spawn point
    player.position = Vec2(100, 100);
    
    // Load spritesheets
    playerSpritesheet = renderer->loadTexture("../assets/AlternatingWalk.png");
    projectileLeftSpritesheet = renderer->loadTexture("../assets/PencilSpinLeft.png");
    projectileRightSpritesheet = renderer->loadTexture("../assets/PencilSpinRight.png");
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
    
    // Remove destroyed projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.shouldDestroy; }),
        projectiles.end()
    );
    
    // Camera follow
    camera.follow(player, level);
}

void Game::render() {
    renderer->beginFrame();
    
    int camX = camera.getOffsetX();
    int camY = camera.getOffsetY();
    
    // Draw level tiles
    for (int y = 0; y < level.getHeightInTiles(); y++) {
        for (int x = 0; x < level.getWidthInTiles(); x++) {
            int tileType = static_cast<int>(level.getTile(x, y));
            if (tileType != 0) {
                renderer->drawTile(x, y, tileType, camX, camY);
            }
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
    
    renderer->endFrame();
}

