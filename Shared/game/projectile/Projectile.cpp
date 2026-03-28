#include "Projectile.h"

Projectile::Projectile(Vec2 startPos, bool right)
    : position(startPos),
#ifdef PLATFORM_PICO
      velocity(Vec2((right ? SPEED : -SPEED), TO_FIXED(0.0f))),
#else
      velocity(Vec2((right ? SPEED : -SPEED), 0.0f)),
#endif
      lifetime(0.0f),
      shouldDestroy(false),
      movingRight(right),
      currentFrame(0),
      animationTimer(0.0f)
{
}

void Projectile::update(float deltaTime) {
    // Update position
#ifdef PLATFORM_PICO
    position.x += FIXED_MUL(velocity.x, floatToFixed(deltaTime));
    position.y += FIXED_MUL(velocity.y, floatToFixed(deltaTime));
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

Rect Projectile::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
