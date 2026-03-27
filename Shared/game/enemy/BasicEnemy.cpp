#include "BasicEnemy.h"
#include "../level/Level.h"
#include <cmath>

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
    
#ifdef PLATFORM_PICO
    position.y += FIXED_MUL(velocity.y, floatToFixed(deltaTime));
#else
    position.y += velocity.y * deltaTime;
#endif
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= FRAME_TIME) {
        animationTimer -= FRAME_TIME;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
    
    Rect collider = getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    if (velocity.y > 0) {
        for (int x = leftTile; x <= rightTile; x++) {
            if (level.isSolid(x, bottomTile)) {
                position.y = bottomTile * tileSize - collider.height;
                velocity.y = 0;
                return;
            }
            
            if (level.isPlatform(x, bottomTile)) {
#ifdef PLATFORM_PICO
                fixed_t platformTop = TO_FIXED(bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT));
                fixed_t enemyBottom = collider.y + collider.height;
                
                if (enemyBottom >= platformTop - TO_FIXED(1.0f) && enemyBottom <= platformTop + TO_FIXED(4.0f) && velocity.y >= 0) {
                    position.y = platformTop - collider.height;
                    velocity.y = 0;
                    return;
                }
#else
                float platformTop = bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                float enemyBottom = collider.y + collider.height;
                
                if (enemyBottom >= platformTop - 1.0f && enemyBottom <= platformTop + 4.0f && velocity.y >= 0) {
                    position.y = platformTop - collider.height;
                    velocity.y = 0;
                    return;
                }
#endif
            }
        }
    }
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
    
    int tileToDrop = (int)checkX / tileSize;
    int tileAtFeet = (int)checkY / tileSize;
    
    bool hasGroundAhead = level.isSolid(tileToDrop, tileAtFeet) || level.isPlatform(tileToDrop, tileAtFeet);
    
    if (!hasGroundAhead) {
        setMovingRight(!movingRight());
        velocity.x = -velocity.x;
    }
}

void BasicEnemy::checkWallCollision(const Level& level) {
    Rect collider = getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = (int)(collider.x) / tileSize;
    int rightTile = (int)(collider.x + collider.width - 1) / tileSize;
    int topTile = (int)(collider.y) / tileSize;
    int bottomTile = (int)(collider.y + collider.height - 1) / tileSize;
    
    if (velocity.x > 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(rightTile, y)) {
                position.x = rightTile * tileSize - collider.width;
                setMovingRight(false);
                velocity.x = -MOVE_SPEED;
                return;
            }
        }
    } else if (velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
                position.x = (leftTile + 1) * tileSize;
                setMovingRight(true);
                velocity.x = MOVE_SPEED;
                return;
            }
        }
    }
}

void BasicEnemy::applyGravity(float deltaTime) {
    velocity.y += GRAVITY * deltaTime;
    if (velocity.y > MAX_FALL_SPEED) {
        velocity.y = MAX_FALL_SPEED;
    }
}
