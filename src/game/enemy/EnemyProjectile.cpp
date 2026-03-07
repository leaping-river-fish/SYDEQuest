#include "EnemyProjectile.h"
#include <cmath>

EnemyProjectile::EnemyProjectile(Vec2 startPos, Vec2 direction)
    : position(startPos),
      lifetime(0.0f),
      shouldDestroy(false),
      currentFrame(0),
      animationTimer(0.0f)
{
    // Normalize direction and apply speed
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f) {
        velocity.x = (direction.x / length) * SPEED;
        velocity.y = (direction.y / length) * SPEED;
    } else {
        velocity.x = SPEED;
        velocity.y = 0;
    }
    
    // Determine sprite direction based on velocity
    movingRight = velocity.x > 0;
}

void EnemyProjectile::update(float deltaTime) {
    // Update position (no gravity, moves in aimed direction)
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    
    // Update lifetime
    lifetime += deltaTime;
    if (lifetime >= MAX_LIFETIME) {
        shouldDestroy = true;
    }
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= FRAME_TIME) {
        animationTimer -= FRAME_TIME;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
}

Rect EnemyProjectile::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
