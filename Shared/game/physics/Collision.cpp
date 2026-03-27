#include "Collision.h"
#include "../player/Player.h"
#include "../projectile/Projectile.h"
#include "../enemy/EnemyProjectile.h"
#include "../level/Level.h"
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
    if (player.velocity.y > 0) {
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, bottomTile)) {
#ifdef PLATFORM_PICO
                player.position.y = TO_FIXED(bottomTile * tileSize) - collider.height;
#else
                player.position.y = bottomTile * tileSize - collider.height;
#endif
                player.velocity.y = 0;
                player.setGrounded(true);
                player.setWantsToDropThrough(false);
                return;
            }
            
            if (level.isPlatform(x, bottomTile) && !player.wantsToDropThrough()) {
#ifdef PLATFORM_PICO
                fixed_t platformTop = TO_FIXED(bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT));
                fixed_t playerBottom = collider.y + collider.height;
                fixed_t tolerance1 = TO_FIXED(1.0f);
                fixed_t tolerance4 = TO_FIXED(4.0f);
                
                if (playerBottom >= platformTop - tolerance1 && playerBottom <= platformTop + tolerance4 && player.velocity.y >= 0) {
                    player.position.y = platformTop - collider.height;
                    player.velocity.y = 0;
                    player.setGrounded(true);
                    player.setWantsToDropThrough(false);
                    return;
                }
#else
                float platformTop = bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                float playerBottom = collider.y + collider.height;
                
                if (playerBottom >= platformTop - 1.0f && playerBottom <= platformTop + 4.0f && player.velocity.y >= 0) {
                    player.position.y = platformTop - collider.height;
                    player.velocity.y = 0;
                    player.setGrounded(true);
                    player.setWantsToDropThrough(false);
                    return;
                }
#endif
            }
        }
    } else if (player.velocity.y < 0) {
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, topTile)) {
#ifdef PLATFORM_PICO
                player.position.y = TO_FIXED((topTile + 1) * tileSize);
#else
                player.position.y = (topTile + 1) * tileSize;
#endif
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
    
    if (player.velocity.x > 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(rightTile, y)) {
#ifdef PLATFORM_PICO
                player.position.x = TO_FIXED(rightTile * tileSize) - collider.width;
#else
                player.position.x = rightTile * tileSize - collider.width;
#endif
                player.velocity.x = 0;
                return;
            }
        }
    } else if (player.velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
#ifdef PLATFORM_PICO
                player.position.x = TO_FIXED((leftTile + 1) * tileSize);
#else
                player.position.x = (leftTile + 1) * tileSize;
#endif
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