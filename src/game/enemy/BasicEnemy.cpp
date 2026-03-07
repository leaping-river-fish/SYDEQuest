#include "BasicEnemy.h"
#include "../level/Level.h"
#include <cmath>

BasicEnemy::BasicEnemy(Vec2 startPos, bool startMovingRight)
    : position(startPos)
    , velocity(startMovingRight ? MOVE_SPEED : -MOVE_SPEED, 0.0f)
    , health(3)
    , movingRight(startMovingRight)
    , currentFrame(0)
    , animationTimer(0.0f)
{
}

void BasicEnemy::update(float deltaTime, const Level& level) {
    applyGravity(deltaTime);
    
    checkEdgeDetection(level);
    
    position.x += velocity.x * deltaTime;
    
    checkWallCollision(level);
    
    position.y += velocity.y * deltaTime;
    
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
                float platformTop = bottomTile * tileSize + (tileSize - level.PLATFORM_HEIGHT);
                float enemyBottom = collider.y + collider.height;
                
                if (enemyBottom >= platformTop - 1.0f && enemyBottom <= platformTop + 4.0f && velocity.y >= 0) {
                    position.y = platformTop - collider.height;
                    velocity.y = 0;
                    return;
                }
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
    
    float checkX;
    if (movingRight) {
        checkX = position.x + WIDTH;
    } else {
        checkX = position.x - 1;
    }
    
    float checkY = position.y + HEIGHT + 1;
    
    int tileToDrop = (int)checkX / tileSize;
    int tileAtFeet = (int)checkY / tileSize;
    
    bool hasGroundAhead = level.isSolid(tileToDrop, tileAtFeet) || level.isPlatform(tileToDrop, tileAtFeet);
    
    if (!hasGroundAhead) {
        movingRight = !movingRight;
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
                movingRight = false;
                velocity.x = -MOVE_SPEED;
                return;
            }
        }
    } else if (velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
                position.x = (leftTile + 1) * tileSize;
                movingRight = true;
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
