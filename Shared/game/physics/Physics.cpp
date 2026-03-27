#include "Physics.h"
#include "../player/Player.h"
#include <algorithm>

void Physics::applyGravity(Player& player, float deltaTime) {
    if (!player.isGrounded()) {
#ifdef PLATFORM_PICO
        fixed_t dt = floatToFixed(deltaTime);
        player.velocity.y += FIXED_MUL(GRAVITY, dt);
#else
        player.velocity.y += GRAVITY * deltaTime;
#endif
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

