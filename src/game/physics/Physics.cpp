#include "Physics.h"
#include "game/player/Player.h"
#include <algorithm>

void Physics::applyGravity(Player& player, float deltaTime) {
    if (!player.isGrounded) {
        player.velocity.y += GRAVITY * deltaTime;
    }
}

void Physics::clampVelocity(Player& player) {
    // Clamp horizontal speed
    if (player.velocity.x > MAX_MOVE_SPEED) {
        player.velocity.x = MAX_MOVE_SPEED;
    }
    if (player.velocity.x < -MAX_MOVE_SPEED) {
        player.velocity.x = -MAX_MOVE_SPEED;
    }
    
    // Clamp fall speed
    if (player.velocity.y > MAX_FALL_SPEED) {
        player.velocity.y = MAX_FALL_SPEED;
    }
}

