#include "Projectile.h"

Projectile::Projectile(Vec2 startPos, bool right)
    : position(startPos),
      velocity(right ? SPEED : -SPEED, 0),
      lifetime(0.0f),
      shouldDestroy(false),
      movingRight(right),
      currentFrame(0),
      animationTimer(0.0f)
{
}

void Projectile::update(float deltaTime) {
    // Update position
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

Rect Projectile::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
