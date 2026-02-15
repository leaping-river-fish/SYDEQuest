#include "Game.h"
#include "core/IRenderer.h"
#include "core/IInput.h"
#include "core/IHaptics.h"
#include "core/ITimer.h"

Game::Game(IRenderer* r, IInput* i, IHaptics* h, ITimer* t)
    : renderer(r), input(i), haptics(h), timer(t),
      camera(r->getScreenWidth(), r->getScreenHeight())
{
}

void Game::init() {
    // Position player at spawn point
    player.position = Vec2(100, 100);
}

void Game::update() {
    timer->update();
    input->update();
    
    float dt = timer->getDeltaTime();
    
    // Input player actions
    controller.update(player, input);
    
    // Physics
    physics.applyGravity(player, dt);
    physics.clampVelocity(player);
    
    // Update player position
    player.update(dt);
    
    // Collision resolution
    collision.resolvePlayerTileCollision(player, level);
    
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
    
    // Draw player
    Rect playerRect = player.getCollider();
    playerRect.x -= camX;
    playerRect.y -= camY;
    renderer->drawRect(playerRect, Color(255, 0, 0), true);  // Red player
    
    renderer->endFrame();
}

