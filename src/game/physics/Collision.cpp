#include "Collision.h"
#include "game/player/Player.h"
#include "game/projectile/Projectile.h"
#include "game/enemy/EnemyProjectile.h"
#include "game/level/Level.h"
#include <cmath>

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
                // Platforms are half-height (8 pixels), positioned at top of tile
                float platformTop = bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                float playerBottom = collider.y + collider.height;
                
                // Check if player is coming from above or already on platform
                // Allow small tolerance for floating point errors and gravity micro-adjustments
                if (playerBottom >= platformTop - 1.0f && playerBottom <= platformTop + 4.0f && player.velocity.y >= 0) {
                    player.position.y = platformTop - collider.height;
                    player.velocity.y = 0;
                    player.isGrounded = true;
                    player.wantsToDropThrough = false;  // Reset flag when landing on platform
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

bool Collision::checkProjectileTileCollision(const Projectile& projectile, const Level& level) {
    Rect collider = projectile.getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int topTile = (int)(collider.y) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    // Check if projectile hits any solid tile
    // Projectiles pass through one-way platforms (they fly horizontally)
    for (int y = topTile; y <= bottomTile; y++) {
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, y)) {
                return true;
            }
        }
    }
    
    return false;
}

bool Collision::checkEnemyProjectileTileCollision(const EnemyProjectile& projectile, const Level& level) {
    Rect collider = projectile.getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int topTile = (int)(collider.y) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    // Check if enemy projectile hits any solid tile
    // Enemy projectiles pass through platforms but collide with solid blocks
    for (int y = topTile; y <= bottomTile; y++) {
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, y)) {
                return true;
            }
        }
    }
    
    return false;
}