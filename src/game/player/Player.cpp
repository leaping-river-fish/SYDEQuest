#include "Player.h"

Player::Player() 
    : position(100, 100),
      velocity(0, 0),
      isGrounded(false),
      wantsToDropThrough(false),
      health(3)
{
}

void Player::update(float deltaTime) {
    // Apply velocity to position
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}

Rect Player::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}