#include "BasicEnemy.h"
#include "../level/Level.h"
#include <algorithm>
#include <cmath>

#ifndef PICO_DEBUG_ENEMY
#define PICO_DEBUG_ENEMY 0
#endif
#if PICO_DEBUG_ENEMY
#include <cstdio>
#endif

BasicEnemy::BasicEnemy(Vec2 startPos, bool startMovingRight)
    : position(startPos)
#ifdef PLATFORM_PICO
    , velocity(Vec2((startMovingRight ? MOVE_SPEED : -MOVE_SPEED), TO_FIXED(0.0f)))
#else
    , velocity(Vec2((startMovingRight ? MOVE_SPEED : -MOVE_SPEED), 0.0f))
#endif
    , health(3)
    , currentFrame(0)
    , animationTimer(0.0f)
    , flags(startMovingRight ? FLAG_MOVING_RIGHT : 0)
{
}

void BasicEnemy::update(float deltaTime, const Level& level) {
    applyGravity(deltaTime);
    
    checkEdgeDetection(level);
    
#ifdef PLATFORM_PICO
    position.x += FIXED_MUL(velocity.x, floatToFixed(deltaTime));
#else
    position.x += velocity.x * deltaTime;
#endif
    
    checkWallCollision(level);
    
    fixed_t prevY = position.y;
#ifdef PLATFORM_PICO
    position.y += FIXED_MUL(velocity.y, floatToFixed(deltaTime));
#else
    position.y += velocity.y * deltaTime;
#endif
    
    resolveVerticalTerrain(level, prevY);
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= ANIM_FRAME_DURATION_SEC) {
        animationTimer -= ANIM_FRAME_DURATION_SEC;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }

#if PICO_DEBUG_ENEMY && defined(PLATFORM_PICO)
    {
        static int s_dbgFrame;
        s_dbgFrame++;
        if ((s_dbgFrame % 45) == 0) {
            int ts = level.getTileSize();
            int tx = worldToTile(position.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f)), ts);
            int ty = worldToTile(position.y + HEIGHT - TO_FIXED(1.0f), ts);
            printf("enemy trace: pos=(%.2f,%.2f) tile=(%d,%d) id=%d vx=%.2f vy=%.2f\n",
                   fixedToFloat(position.x), fixedToFloat(position.y), tx, ty,
                   level.getTileId(tx, ty), fixedToFloat(velocity.x), fixedToFloat(velocity.y));
        }
    }
#endif
}

bool BasicEnemy::takeDamage(int amount) {
    health -= amount;
    return health <= 0;
}

Rect BasicEnemy::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}

void BasicEnemy::checkEdgeDetection(const Level& level) {
    int tileSize = level.getTileSize();
    
#ifdef PLATFORM_PICO
    fixed_t checkX;
    if (movingRight()) {
        checkX = position.x + WIDTH;
    } else {
        checkX = position.x - TO_FIXED(1.0f);
    }
    
    fixed_t checkY = position.y + HEIGHT + TO_FIXED(1.0f);
#else
    float checkX;
    if (movingRight()) {
        checkX = position.x + WIDTH;
    } else {
        checkX = position.x - 1;
    }
    
    float checkY = position.y + HEIGHT + 1;
#endif
    
    int tileToDrop = worldToTile(checkX, tileSize);
    int tileAtFeet = worldToTile(checkY, tileSize);
    
    bool hasGroundAhead = level.isSolid(tileToDrop, tileAtFeet) || level.isPlatform(tileToDrop, tileAtFeet);
    
    if (!hasGroundAhead) {
#if PICO_DEBUG_ENEMY && defined(PLATFORM_PICO)
        printf("enemy edge flip: aheadTile=(%d,%d) id=%d solid=%d plat=%d checkX=%.2f\n",
               tileToDrop, tileAtFeet, level.getTileId(tileToDrop, tileAtFeet),
               level.isSolid(tileToDrop, tileAtFeet) ? 1 : 0,
               level.isPlatform(tileToDrop, tileAtFeet) ? 1 : 0, fixedToFloat(checkX));
#endif
        setMovingRight(!movingRight());
        velocity.x = -velocity.x;
    }
}

void BasicEnemy::checkWallCollision(const Level& level) {
    Rect collider = getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);
    
    if (velocity.x > 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(rightTile, y)) {
#if PICO_DEBUG_ENEMY && defined(PLATFORM_PICO)
                printf("enemy wall R: rightTile=%d y=%d tileId=%d posX=%.2f snap->%.2f\n",
                       rightTile, y, level.getTileId(rightTile, y), fixedToFloat(position.x),
                       fixedToFloat(TO_FIXED(static_cast<float>(rightTile * tileSize)) - collider.width));
#endif
#ifdef PLATFORM_PICO
                position.x = TO_FIXED(rightTile * tileSize) - collider.width;
#else
                position.x = rightTile * tileSize - collider.width;
#endif
                setMovingRight(false);
                velocity.x = -MOVE_SPEED;
                return;
            }
        }
    } else if (velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
#if PICO_DEBUG_ENEMY && defined(PLATFORM_PICO)
                printf("enemy wall L: leftTile=%d y=%d tileId=%d posX=%.2f snap->%.2f\n",
                       leftTile, y, level.getTileId(leftTile, y), fixedToFloat(position.x),
                       fixedToFloat(TO_FIXED(static_cast<float>((leftTile + 1) * tileSize))));
#endif
#ifdef PLATFORM_PICO
                position.x = TO_FIXED((leftTile + 1) * tileSize);
#else
                position.x = (leftTile + 1) * tileSize;
#endif
                setMovingRight(true);
                velocity.x = MOVE_SPEED;
                return;
            }
        }
    }
}

void BasicEnemy::applyGravity(float deltaTime) {
#ifdef PLATFORM_PICO
    velocity.y += FIXED_MUL(GRAVITY, floatToFixed(deltaTime));
#else
    velocity.y += GRAVITY * deltaTime;
#endif
    if (velocity.y > MAX_FALL_SPEED) {
        velocity.y = MAX_FALL_SPEED;
    }
}

void BasicEnemy::resolveVerticalTerrain(const Level& level, fixed_t prevY) {
    Rect collider = getCollider();
    const int tileSize = level.getTileSize();
    const fixed_t h = collider.height;
    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);
    int prevBottomTile = worldToTile(prevY + h - TO_FIXED(1.0f), tileSize);
    int newTopTile = worldToTile(collider.y, tileSize);
    int prevTopTile = worldToTile(prevY, tileSize);

    if (velocity.y > 0) {
        int rowStart = std::min(prevBottomTile, bottomTile);
        int rowEnd = std::max(prevBottomTile, bottomTile);
        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isSolid(x, row)) {
#ifdef PLATFORM_PICO
                    position.y = TO_FIXED(row * tileSize) - h;
#else
                    position.y = row * tileSize - h;
#endif
                    velocity.y = 0;
                    return;
                }
            }
        }
        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isPlatform(x, row)) {
#ifdef PLATFORM_PICO
                    fixed_t platformTop = TO_FIXED(row * tileSize + (tileSize - level.PLATFORM_HEIGHT));
                    fixed_t prevBottom = prevY + collider.height;
                    fixed_t currBottom = collider.y + collider.height;
                    if (prevBottom <= platformTop && currBottom >= platformTop && velocity.y >= 0) {
                        position.y = platformTop - h;
                        velocity.y = 0;
                        return;
                    }
#else
                    float platformTop = row * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                    fixed_t prevBottom = prevY + collider.height;
                    fixed_t currBottom = collider.y + collider.height;
                    if (prevBottom <= platformTop && currBottom >= platformTop && velocity.y >= 0) {
                        position.y = platformTop - h;
                        velocity.y = 0;
                        return;
                    }
#endif
                }
            }
        }
    } else if (velocity.y < 0) {
        int rowStart = std::min(newTopTile, prevTopTile);
        int rowEnd = std::max(newTopTile, prevTopTile);
        for (int row = rowEnd; row >= rowStart; --row) {
            for (int x = leftTile; x <= rightTile; ++x) {
                if (level.isSolid(x, row)) {
#ifdef PLATFORM_PICO
                    position.y = TO_FIXED((row + 1) * tileSize);
#else
                    position.y = (row + 1) * tileSize;
#endif
                    velocity.y = 0;
                    return;
                }
            }
        }
    }
}
