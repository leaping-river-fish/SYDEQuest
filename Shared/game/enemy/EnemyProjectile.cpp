#include "EnemyProjectile.h"

#ifndef PLATFORM_PICO
    #include <cmath>
#endif

#ifdef PLATFORM_PICO
inline fixed_t fixedSqrt(fixed_t x) {
    if (x <= 0) return 0;
    
    fixed_t result = x;
    fixed_t prev;
    int iterations = 0;
    
    do {
        prev = result;
        result = (result + FIXED_DIV(x, result)) >> 1;
        iterations++;
    } while (iterations < 10 && (result != prev));
    
    return result;
}
#endif

EnemyProjectile::EnemyProjectile(Vec2 startPos, Vec2 direction)
    : position(startPos),
      lifetime(0.0f),
      shouldDestroy(false),
      currentFrame(0),
      animationTimer(0.0f)
{
#ifdef PLATFORM_PICO
    fixed_t lengthSquared = FIXED_MUL(direction.x, direction.x) + FIXED_MUL(direction.y, direction.y);
    fixed_t length = fixedSqrt(lengthSquared);
    
    if (length > 0) {
        velocity.x = FIXED_MUL(FIXED_DIV(direction.x, length), SPEED);
        velocity.y = FIXED_MUL(FIXED_DIV(direction.y, length), SPEED);
    } else {
        velocity.x = SPEED;
        velocity.y = 0;
    }
#else
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f) {
        velocity.x = (direction.x / length) * SPEED;
        velocity.y = (direction.y / length) * SPEED;
    } else {
        velocity.x = SPEED;
        velocity.y = 0;
    }
#endif
    
    movingRight = velocity.x > 0;
}

void EnemyProjectile::update(float deltaTime) {
#ifdef PLATFORM_PICO
    fixed_t dt = floatToFixed(deltaTime);
    position.x += FIXED_MUL(velocity.x, dt);
    position.y += FIXED_MUL(velocity.y, dt);
#else
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
#endif
    
    // Update lifetime
    lifetime += deltaTime;
    if (lifetime >= MAX_LIFETIME) {
        shouldDestroy = true;
    }
    
    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= ANIM_FRAME_DURATION_SEC) {
        animationTimer -= ANIM_FRAME_DURATION_SEC;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
}

Rect EnemyProjectile::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
