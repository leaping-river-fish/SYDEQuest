#include "Game.h"
#include "core/IRenderer.h"
#include "core/IInput.h"
#include "core/IHaptics.h"
#include "core/ITimer.h"

Game::Game(IRenderer* r, IInput* i, IHaptics* h, ITimer* t)
    : renderer(r), input(i), haptics(h), timer(t),
      camera(r->getScreenWidth(), r->getScreenHeight()),
      playerSpritesheet(-1)
{
}

void Game::init() {
    // Position player at spawn point
    player.position = Vec2(100, 100);
    
    // Load player spritesheet
    playerSpritesheet = renderer->loadTexture("../assets/AlternatingWalk.png");
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

