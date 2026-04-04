#include "Collision.h"
#include "../player/Player.h"
#include "../projectile/Projectile.h"
#include "../enemy/EnemyProjectile.h"
#include "../level/Level.h"
#include <algorithm>

void Collision::resolveVertical(Player& player, const Level& level, fixed_t prevY) {
    Rect collider = player.getCollider();
    int tileSize = level.getTileSize();

    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);

    fixed_t h = collider.height;
    int prevBottomTile = worldToTile(prevY + h - TO_FIXED(1.0f), tileSize);
    int prevTopTile = worldToTile(prevY, tileSize);
    int newTopTile = worldToTile(player.position.y, tileSize);

    if (player.velocity.y > 0) {
        int rowStart = std::min(prevBottomTile, bottomTile);
        int rowEnd = std::max(prevBottomTile, bottomTile);

        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isSolid(x, row)) {
#ifdef PLATFORM_PICO
                    player.position.y = TO_FIXED(row * tileSize) - h;
#else
                    player.position.y = row * tileSize - h;
#endif
                    player.velocity.y = 0;
                    player.setGrounded(true);
                    player.setWantsToDropThrough(false);
                    return;
                }
            }
        }

        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isPlatform(x, row) && !player.wantsToDropThrough()) {
#ifdef PLATFORM_PICO
                    fixed_t platformTop = TO_FIXED(row * tileSize + (tileSize - level.PLATFORM_HEIGHT));
                    fixed_t prevBottom = prevY + collider.height;
                    fixed_t currBottom = collider.y + collider.height;

                    if (prevBottom <= platformTop && currBottom >= platformTop && player.velocity.y >= 0 &&
                        !player.wantsToDropThrough()) {
                        player.position.y = platformTop - h;
                        player.velocity.y = 0;
                        player.setGrounded(true);
                        player.setWantsToDropThrough(false);
                        return;
                    }
#else
                    float platformTop = row * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                    fixed_t prevBottom = prevY + collider.height;
                    fixed_t currBottom = collider.y + collider.height;

                    if (prevBottom <= platformTop && currBottom >= platformTop && player.velocity.y >= 0 &&
                        !player.wantsToDropThrough()) {
                        player.position.y = platformTop - h;
                        player.velocity.y = 0;
                        player.setGrounded(true);
                        player.setWantsToDropThrough(false);
                        return;
                    }
#endif
                }
            }
        }
    } else if (player.velocity.y < 0) {
        // Moving up: check from previous (lower on screen) row down to new row so first hit matches travel order
        int rowStart = std::min(newTopTile, prevTopTile);
        int rowEnd = std::max(newTopTile, prevTopTile);

        for (int row = rowEnd; row >= rowStart; --row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isSolid(x, row)) {
#ifdef PLATFORM_PICO
                    player.position.y = TO_FIXED((row + 1) * tileSize);
#else
                    player.position.y = (row + 1) * tileSize;
#endif
                    player.velocity.y = 0;
                    return;
                }
            }
        }
    }

    // Resting support when vy == 0 (grounded was cleared before Y move in Game::update)
    if (player.velocity.y == 0) {
        collider = player.getCollider();
        leftTile = worldToTile(collider.x, tileSize);
        rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
        int footRow = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);

        for (int x = leftTile; x <= rightTile; ++x) {
            if (level.isSolid(x, footRow + 1)) {
                player.setGrounded(true);
                player.setWantsToDropThrough(false);
                return;
            }
        }

        // One-way platform tile is usually the same row as the foot (bottom half of cell), not footRow+1.
        for (int x = leftTile; x <= rightTile; ++x) {
            if (level.isPlatform(x, footRow) && !player.wantsToDropThrough()) {
#ifdef PLATFORM_PICO
                fixed_t platformTop =
                    TO_FIXED(footRow * tileSize + (tileSize - level.PLATFORM_HEIGHT));
                fixed_t playerBottom = collider.y + collider.height;
                fixed_t tolerance1 = TO_FIXED(1.0f);
                fixed_t tolerance4 = TO_FIXED(4.0f);
                if (playerBottom >= platformTop - tolerance1 && playerBottom <= platformTop + tolerance4) {
                    player.position.y = platformTop - collider.height;
                    player.setGrounded(true);
                    player.setWantsToDropThrough(false);
                    return;
                }
#else
                float platformTop = footRow * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                float playerBottom = collider.y + collider.height;
                if (playerBottom >= platformTop - 1.0f && playerBottom <= platformTop + 4.0f) {
                    player.position.y = platformTop - collider.height;
                    player.setGrounded(true);
                    player.setWantsToDropThrough(false);
                    return;
                }
#endif
            }
        }
    }
}

// resolve horizontal collisions

void Collision::resolveHorizontal(Player& player, const Level& level) {
    Rect collider = player.getCollider();
    int tileSize = level.getTileSize();

    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);

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

void Collision::resolveArenaWalls(Player& player, const Rect* walls, size_t count) {
    if (!walls || count == 0) {
        return;
    }
    for (int iter = 0; iter < 2; ++iter) {
        for (size_t wi = 0; wi < count; ++wi) {
            Rect r = player.getCollider();
            const Rect& w = walls[wi];
            if (!r.intersects(w)) {
                continue;
            }
            fixed_t penL = (r.x + r.width) - w.x;
            fixed_t penR = (w.x + w.width) - r.x;
            fixed_t penT = (r.y + r.height) - w.y;
            fixed_t penB = (w.y + w.height) - r.y;
            fixed_t m = penL;
            int axis = 0;
            if (penR < m) {
                m = penR;
                axis = 1;
            }
            if (penT < m) {
                m = penT;
                axis = 2;
            }
            if (penB < m) {
                m = penB;
                axis = 3;
            }
            if (m <= 0) {
                continue;
            }
            switch (axis) {
                case 0:
                    player.position.x -= m;
                    player.velocity.x = 0;
                    break;
                case 1:
                    player.position.x += m;
                    player.velocity.x = 0;
                    break;
                case 2:
                    player.position.y -= m;
                    player.velocity.y = 0;
                    break;
                case 3:
                    player.position.y += m;
                    player.velocity.y = 0;
                    break;
                default:
                    break;
            }
        }
    }
}

bool Collision::checkProjectileTileCollision(const Projectile& projectile, const Level& level) {
    Rect collider = projectile.getCollider();
    int tileSize = level.getTileSize();

    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);

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

    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);

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
