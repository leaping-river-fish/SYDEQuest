#include "PlayerController.h"
#include "Player.h"
#include "../../core/IInput.h"

PlayerController::PlayerController()
    : jumpBufferTimer(0.0f),
      coyoteTimer(0.0f),
      wasGroundedLastFrame(false),
      dropThroughTimer(0.0f),
      fireCooldownTimer(0.0f)
{
}

void PlayerController::update(Player& player, IInput* input, float deltaTime) {
    handleMovement(player, input);
    handleJump(player, input, deltaTime);
    handleDropThrough(player, input, deltaTime);
    handleFiring(player, input, deltaTime);
}

void PlayerController::handleMovement(Player& player, IInput* input) {
    // Horizontal movement
    if (input->isPressed(Button::Left)) {
        player.velocity.x = -Player::MOVE_SPEED;
    } else if (input->isPressed(Button::Right)) {
        player.velocity.x = Player::MOVE_SPEED;
    } else {
        player.velocity.x = 0;  // Instant stop
    }
}

void PlayerController::handleJump(Player& player, IInput* input, float deltaTime) {
    if (player.isGrounded()) {
        coyoteTimer = COYOTE_TIME;
        wasGroundedLastFrame = true;
    } else {
        if (wasGroundedLastFrame) {
            wasGroundedLastFrame = false;
        }
        coyoteTimer -= deltaTime;
        if (coyoteTimer < 0.0f) {
            coyoteTimer = 0.0f;
        }
    }
    
    if (input->wasJustPressed(Button::Jump)) {
        jumpBufferTimer = JUMP_BUFFER_TIME;
    } else {
        jumpBufferTimer -= deltaTime;
        if (jumpBufferTimer < 0.0f) {
            jumpBufferTimer = 0.0f;
        }
    }
    
    bool canJump = (player.isGrounded() || coyoteTimer > 0.0f);
    bool wantsToJump = jumpBufferTimer > 0.0f;
    
    if (wantsToJump && canJump) {
        player.velocity.y = Player::JUMP_VELOCITY;
        player.setGrounded(false);
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
    }
}

void PlayerController::handleDropThrough(Player& player, IInput* input, float deltaTime) {
    if (input->isPressed(Button::Down) && player.isGrounded()) {
        player.setWantsToDropThrough(true);
        player.setGrounded(false);
        dropThroughTimer = DROP_THROUGH_TIME;
#ifdef PLATFORM_PICO
        player.velocity.y = TO_FIXED(50.0f);
#else
        player.velocity.y = 50.0f;
#endif
    }
    
    if (input->isPressed(Button::Down) && player.wantsToDropThrough()) {
        dropThroughTimer = DROP_THROUGH_TIME;
    }
    
    if (player.wantsToDropThrough()) {
        dropThroughTimer -= deltaTime;
        if (dropThroughTimer <= 0.0f) {
            player.setWantsToDropThrough(false);
            dropThroughTimer = 0.0f;
        }
    }
}

void PlayerController::handleFiring(Player& player, IInput* input, float deltaTime) {
    player.setWantsToFire(false);
    
    if (fireCooldownTimer > 0.0f) {
        fireCooldownTimer -= deltaTime;
        if (fireCooldownTimer < 0.0f) {
            fireCooldownTimer = 0.0f;
        }
    }
    
    if (input->wasJustPressed(Button::Fire)) {
        if (fireCooldownTimer <= 0.0f) {
            player.setWantsToFire(true);
            fireCooldownTimer = FIRE_COOLDOWN;
        }
    }
}