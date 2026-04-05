#include "RangedEnemy.h"
#include "../level/Level.h"
#include "../player/Player.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

#ifdef PLATFORM_PICO
    #include "../Game.h"
#else
    #include <vector>
#endif

RangedEnemy::RangedEnemy(Vec2 startPos, bool startFacingRight)
    : position(startPos)
#ifdef PLATFORM_PICO
    , velocity(Vec2((startFacingRight ? MOVE_SPEED : -MOVE_SPEED), TO_FIXED(0.0f)))
#else
    , velocity(Vec2((startFacingRight ? MOVE_SPEED : -MOVE_SPEED), 0.0f))
#endif
    , health(5)
    , currentFrame(0)
    , animationTimer(0.0f)
    , shootTimer(0.0f)
    , shootPauseTimer(0.0f)
    , flags(startFacingRight ? FLAG_FACING_RIGHT : 0)
{
}

#ifdef PLATFORM_PICO
void RangedEnemy::update(float deltaTime, const Level& level, const Player& player, EntityPool<EnemyProjectile, 30>& projectiles) {
#else
void RangedEnemy::update(float deltaTime, const Level& level, const Player& player, std::vector<EnemyProjectile>& projectiles) {
#endif
    applyGravity(deltaTime);
    
    // Update shoot timer
    shootTimer += deltaTime;
    
    // Check if player is detected
    bool playerDetected = canDetectPlayer(player, level);
    
    // Handle shooting pause
    if (isShooting()) {
        shootPauseTimer -= deltaTime;
        if (shootPauseTimer <= 0.0f) {
            setIsShooting(false);
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
        
        setFacingRight(playerCenter.x > enemyCenter.x);
        
        // Shoot at intervals
        if (shootTimer >= SHOOT_INTERVAL_SEC) {
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
            
#ifdef PLATFORM_PICO
            EnemyProjectile* proj = projectiles.allocate();
            if (proj) {
                *proj = EnemyProjectile(projectilePos, direction);
            }
#else
            projectiles.push_back(EnemyProjectile(projectilePos, direction));
#endif
            
            // Reset timers and enter shooting state
            shootTimer = 0.0f;
            shootPauseTimer = SHOOT_PAUSE_SEC;
            setIsShooting(true);
        }
    } else {
        // Normal patrol movement when player not detected
        checkEdgeDetection(level);
        velocity.x = facingRight() ? MOVE_SPEED : -MOVE_SPEED;
    }
    
    // Move horizontally
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
    
#ifdef PLATFORM_PICO
    fixed_t checkX;
    if (facingRight()) {
        checkX = position.x + WIDTH;
    } else {
        checkX = position.x - TO_FIXED(1.0f);
    }
    
    fixed_t checkY = position.y + HEIGHT + TO_FIXED(1.0f);
#else
    float checkX;
    if (facingRight()) {
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
        setFacingRight(!facingRight());
    }
}

void RangedEnemy::checkWallCollision(const Level& level) {
    Rect collider = getCollider();
    int tileSize = level.getTileSize();
    
    int leftTile = worldToTile(collider.x, tileSize);
    int rightTile = worldToTile(collider.x + collider.width - TO_FIXED(1.0f), tileSize);
    int topTile = worldToTile(collider.y, tileSize);
    int bottomTile = worldToTile(collider.y + collider.height - TO_FIXED(1.0f), tileSize);
    
    if (velocity.x > 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(rightTile, y)) {
#ifdef PLATFORM_PICO
                position.x = TO_FIXED(rightTile * tileSize) - collider.width;
#else
                position.x = rightTile * tileSize - collider.width;
#endif
                setFacingRight(false);
                return;
            }
        }
    } else if (velocity.x < 0) {
        for (int y = topTile; y <= bottomTile; y++) {
            if (level.isSolid(leftTile, y)) {
#ifdef PLATFORM_PICO
                position.x = TO_FIXED((leftTile + 1) * tileSize);
#else
                position.x = (leftTile + 1) * tileSize;
#endif
                setFacingRight(true);
                return;
            }
        }
    }
}

void RangedEnemy::applyGravity(float deltaTime) {
#ifdef PLATFORM_PICO
    velocity.y += FIXED_MUL(GRAVITY, floatToFixed(deltaTime));
#else
    velocity.y += GRAVITY * deltaTime;
#endif
    if (velocity.y > MAX_FALL_SPEED) {
        velocity.y = MAX_FALL_SPEED;
    }
}

void RangedEnemy::resolveVerticalTerrain(const Level& level, fixed_t prevY) {
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

bool RangedEnemy::hasLineOfSight(Vec2 targetPos, const Level& level) const {
    Vec2 enemyCenter = Vec2(position.x + WIDTH / 2, position.y + HEIGHT / 2);
    
    int ts = level.getTileSize();
    // Bresenham's line algorithm to check for solid blocks between enemy and target
    int x0 = worldToTile(enemyCenter.x, ts);
    int y0 = worldToTile(enemyCenter.y, ts);
    int x1 = worldToTile(targetPos.x, ts);
    int y1 = worldToTile(targetPos.y, ts);
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;

    const int maxSteps = std::abs(x1 - x0) + std::abs(y1 - y0) + 2;
    for (int step = 0; step < maxSteps; ++step) {
        if (level.isSolid(x, y)) {
            return false;
        }
        if (x == x1 && y == y1) {
            return true;
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
    return false;
}

bool RangedEnemy::canDetectPlayer(const Player& player, const Level& level) const {
#ifdef PLATFORM_PICO
    fixed_t halfWidth = WIDTH >> 1;
    fixed_t halfHeight = HEIGHT >> 1;
    fixed_t playerHalfWidth = Player::WIDTH >> 1;
    fixed_t playerHalfHeight = Player::HEIGHT >> 1;
    
    Vec2 enemyCenter = Vec2(position.x + halfWidth, position.y + halfHeight);
    Vec2 playerCenter = Vec2(
        player.position.x + playerHalfWidth,
        player.position.y + playerHalfHeight
    );
    
    // Check distance using fixed-point
    fixed_t dx = playerCenter.x - enemyCenter.x;
    fixed_t dy = playerCenter.y - enemyCenter.y;
    fixed_t distSquared = FIXED_MUL(dx, dx) + FIXED_MUL(dy, dy);
    fixed_t radiusSquared = FIXED_MUL(DETECTION_RADIUS, DETECTION_RADIUS);
    
    if (distSquared > radiusSquared) {
        return false;
    }
#else
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
#endif
    
    // Check line of sight
    return hasLineOfSight(playerCenter, level);
}
