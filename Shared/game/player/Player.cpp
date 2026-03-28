#include "Player.h"

Player::Player() 
    : position(100, 100),
      velocity(0, 0),
      health(3),
      invincibilityTimer(0.0f),
      currentFrame(1),
      animationTimer(0.0f),
      flags(FLAG_FACING_RIGHT)
{
}

void Player::update(float deltaTime) {
    // Update invincibility timer
    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= deltaTime;
    }
    
    // Update facing direction based on movement
    if (velocity.x > 0) {
        setFacingRight(true);
    } else if (velocity.x < 0) {
        setFacingRight(false);
    }
    
    // Determine animation state
    bool isMovingHorizontally = (velocity.x != 0);
#ifdef PLATFORM_PICO
    bool isJumpingOrFalling = (velocity.y < TO_FIXED(-50.0f) || velocity.y > TO_FIXED(50.0f));
#else
    bool isJumpingOrFalling = (velocity.y < -50.0f || velocity.y > 50.0f);
#endif
    
    if (isJumpingOrFalling && isMovingHorizontally) {
        // Jumping/falling while moving - use a mid-action frame
        currentFrame = 5;
        animationTimer = 0.0f;
    } else if (isJumpingOrFalling) {
        // Jumping/falling but not moving - use idle frame
        currentFrame = 1;
        animationTimer = 0.0f;
    } else if (isMovingHorizontally) {
        // Walking - cycle through all frames
        animationTimer += deltaTime;
        if (animationTimer >= ANIM_FRAME_DURATION_SEC) {
            animationTimer -= ANIM_FRAME_DURATION_SEC;
            currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        }
    } else {
        // Idle - use first frame
        currentFrame = 1;
        animationTimer = 0.0f;
    }
}

Rect Player::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}