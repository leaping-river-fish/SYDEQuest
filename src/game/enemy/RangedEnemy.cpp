#include "RangedEnemy.h"
#include "../level/Level.h"
#include "../player/Player.h"
#include <cmath>

RangedEnemy::RangedEnemy(Vec2 startPos, bool startFacingRight)
    : position(startPos)
    , velocity(startFacingRight ? MOVE_SPEED : -MOVE_SPEED, 0.0f)
    , health(5)
    , facingRight(startFacingRight)
    , currentFrame(0)
    , animationTimer(0.0f)
    , shootTimer(0.0f)
    , shootPauseTimer(0.0f)
    , isShooting(false)
{
}

void RangedEnemy::update(float deltaTime, const Level& level, const Player& player, std::vector<EnemyProjectile>& projectiles) {
    applyGravity(deltaTime);
    
    // Update shoot timer
    shootTimer += deltaTime;
    
    // Check if player is detected
    bool playerDetected = canDetectPlayer(player, level);
    
    // Handle shooting pause
    if (isShooting) {
        shootPauseTimer -= deltaTime;
        if (shootPauseTimer <= 0.0f) {
            isShooting = false;
        }
        velocity.x = 0;
    } else if (playerDetected) {
        // Stop moving completely when player is detected
        velocity.x = 0;
        
        // Turn towards player
        Vec2 playerCenter = Vec2(
            player.position.x + Player::WIDTH / 2,
            player.position.y + Player::HEIGHT / 2
        );
        Vec2 enemyCenter = Vec2(
            position.x + WIDTH / 2,
            position.y + HEIGHT / 2
        );
        
        facingRight = playerCenter.x > enemyCenter.x;
        
        // Shoot at intervals
        if (shootTimer >= SHOOT_INTERVAL) {
            // Spawn projectile at enemy center
            Vec2 projectilePos = Vec2(
                enemyCenter.x - EnemyProjectile::WIDTH / 2,
                enemyCenter.y - EnemyProjectile::HEIGHT / 2
            );
            
            // Calculate direction vector toward player
            Vec2 direction = Vec2(
                playerCenter.x - enemyCenter.x,
                playerCenter.y - enemyCenter.y
            );
            
            projectiles.push_back(EnemyProjectile(projectilePos, direction));
            
            // Reset timers and enter shooting state
            shootTimer = 0.0f;
            shootPauseTimer = SHOOT_PAUSE_TIME;
            isShooting = true;
        }
    } else {
        // Normal patrol movement when player not detected
        checkEdgeDetection(level);
        velocity.x = facingRight ? MOVE_SPEED : -MOVE_SPEED;
    }
    
    // Move horizontally
    position.x += velocity.x * deltaTime;
    checkWallCollision(level);
    
    // Move vertically
    position.y += velocity.y * deltaTime;
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= FRAME_TIME) {
        animationTimer -= FRAME_TIME;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
    
    // Ground collision detection (same as BasicEnemy)
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

bool RangedEnemy::takeDamage(int amount) {
    health -= amount;
    return health <= 0;
}

Rect RangedEnemy::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}

void RangedEnemy::checkEdgeDetection(const Level& level) {
    int tileSize = level.getTileSize();
    
    float checkX;
    if (facingRight) {
        checkX = position.x + WIDTH;
    } else {
        checkX = position.x - 1;
    }
    
    float checkY = position.y + HEIGHT + 1;
    
    int tileToDrop = (int)checkX / tileSize;
    int tileAtFeet = (int)checkY / tileSize;
    
    bool hasGroundAhead = level.isSolid(tileToDrop, tileAtFeet) || level.isPlatform(tileToDrop, tileAtFeet);
    
    if (!hasGroundAhead) {
        facingRight = !facingRight;
    }
}

void RangedEnemy::checkWallCollision(const Level& level) {
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
                facingRight = false;
                return;
            }
        }
    } else if (velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
                position.x = (leftTile + 1) * tileSize;
                facingRight = true;
                return;
            }
        }
    }
}

void RangedEnemy::applyGravity(float deltaTime) {
    velocity.y += GRAVITY * deltaTime;
    if (velocity.y > MAX_FALL_SPEED) {
        velocity.y = MAX_FALL_SPEED;
    }
}

bool RangedEnemy::hasLineOfSight(Vec2 targetPos, const Level& level) const {
    Vec2 enemyCenter = Vec2(position.x + WIDTH / 2, position.y + HEIGHT / 2);
    
    // Bresenham's line algorithm to check for solid blocks between enemy and target
    int x0 = (int)(enemyCenter.x / level.getTileSize());
    int y0 = (int)(enemyCenter.y / level.getTileSize());
    int x1 = (int)(targetPos.x / level.getTileSize());
    int y1 = (int)(targetPos.y / level.getTileSize());
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (true) {
        // Check if this tile is solid (blocks line of sight)
        if (level.isSolid(x, y)) {
            return false;
        }
        
        // Reached target
        if (x == x1 && y == y1) {
            break;
        }
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    
    return true;
}

bool RangedEnemy::canDetectPlayer(const Player& player, const Level& level) const {
    Vec2 enemyCenter = Vec2(position.x + WIDTH / 2, position.y + HEIGHT / 2);
    Vec2 playerCenter = Vec2(
        player.position.x + Player::WIDTH / 2,
        player.position.y + Player::HEIGHT / 2
    );
    
    // Check distance
    float dx = playerCenter.x - enemyCenter.x;
    float dy = playerCenter.y - enemyCenter.y;
    float distSquared = dx * dx + dy * dy;
    float radiusSquared = DETECTION_RADIUS * DETECTION_RADIUS;
    
    if (distSquared > radiusSquared) {
        return false;
    }
    
    // Check line of sight
    return hasLineOfSight(playerCenter, level);
}
