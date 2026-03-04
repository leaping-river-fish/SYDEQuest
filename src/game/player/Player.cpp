#include "Player.h"

Player::Player() 
    : position(100, 100),
      velocity(0, 0),
      isGrounded(false),
      wantsToDropThrough(false),
      wantsToFire(false),
      health(3),
      currentFrame(1),
      animationTimer(0.0f),
      facingRight(true)
{
}

void Player::update(float deltaTime) {
    // Update facing direction based on movement
    if (velocity.x > 0) {
        facingRight = true;
    } else if (velocity.x < 0) {
        facingRight = false;
    }
    
    // Determine animation state
    bool isMovingHorizontally = (velocity.x != 0);
    bool isJumpingOrFalling = (velocity.y < -50.0f || velocity.y > 50.0f);
    
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
        if (animationTimer >= FRAME_TIME) {
            animationTimer -= FRAME_TIME;
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