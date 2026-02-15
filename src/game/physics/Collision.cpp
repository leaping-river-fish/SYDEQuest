#include "Collision.h"
#include "game/player/Player.h"
#include "game/level/Level.h"
#include <cmath>

void Collision::resolvePlayerTileCollision(Player& player, const Level& level) {
    player.isGrounded = false;
    
    // vertical first, then horizontal
    resolveVertical(player, level);
    resolveHorizontal(player, level);
}

// resolve vertical collisions

void Collision::resolveVertical(Player& player, const Level& level) {
    Rect collider = player.getCollider();
    int tileSize = level.getTileSize();
    
    // Get tile range
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int topTile = (int)(collider.y) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    // Check vertical collisions
    if (player.velocity.y > 0) {  // Moving down
        for (int x = leftTile; x <= rightTile; x++) {
            // Check solid tiles
            if (level.isSolid(x, bottomTile)) {
                player.position.y = bottomTile * tileSize - collider.height;
                player.velocity.y = 0;
                player.isGrounded = true;
                player.wantsToDropThrough = false;
                return;
            }
            
            // Check one-way platforms (only from above)
            if (level.isPlatform(x, bottomTile) && !player.wantsToDropThrough) {
                float platformTop = bottomTile * tileSize;
                float playerBottom = collider.y + collider.height;
                float prevPlayerBottom = playerBottom - player.velocity.y * 0.016f;  // Approx prev frame
                
                if (prevPlayerBottom <= platformTop && playerBottom >= platformTop) {
                    player.position.y = platformTop - collider.height;
                    player.velocity.y = 0;
                    player.isGrounded = true;
                    return;
                }
            }
        }
    } else if (player.velocity.y < 0) {  // Moving up
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, topTile)) {
                player.position.y = (topTile + 1) * tileSize;
                player.velocity.y = 0;
                return;
            }
        }
    }
}

// resolve horizontal collisions

void Collision::resolveHorizontal(Player& player, const Level& level) {
    Rect collider = player.getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int topTile = (int)(collider.y) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    if (player.velocity.x > 0) {  // Moving right
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(rightTile, y)) {
                player.position.x = rightTile * tileSize - collider.width;
                player.velocity.x = 0;
                return;
            }
        }
    } else if (player.velocity.x < 0) {  // Moving left
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
                player.position.x = (leftTile + 1) * tileSize;
                player.velocity.x = 0;
                return;
            }
        }
    }
}