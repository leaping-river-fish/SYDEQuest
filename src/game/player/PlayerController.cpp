#include "PlayerController.h"
#include "Player.h"
#include "core/IInput.h"

PlayerController::PlayerController()
    : jumpBufferTimer(0.0f),
      coyoteTimer(0.0f),
      wasGroundedLastFrame(false),
      dropThroughTimer(0.0f)
{
}

void PlayerController::update(Player& player, IInput* input, float deltaTime) {
    handleMovement(player, input);
    handleJump(player, input, deltaTime);
    handleDropThrough(player, input, deltaTime);
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
    // Update coyote time - allow jump briefly after leaving ground
    if (player.isGrounded) {
        coyoteTimer = COYOTE_TIME;
        wasGroundedLastFrame = true;
    } else {
        if (wasGroundedLastFrame) {
            // Just left the ground, start coyote timer
            wasGroundedLastFrame = false;
        }
        coyoteTimer -= deltaTime;
        if (coyoteTimer < 0.0f) {
            coyoteTimer = 0.0f;
        }
    }
    
    // Update jump buffer - remember jump input for a short time
    if (input->wasJustPressed(Button::Jump)) {
        jumpBufferTimer = JUMP_BUFFER_TIME;
    } else {
        jumpBufferTimer -= deltaTime;
        if (jumpBufferTimer < 0.0f) {
            jumpBufferTimer = 0.0f;
        }
    }
    
    // Execute jump if:
    // - Jump button was pressed recently (buffer)
    // - AND player is grounded OR just left ground (coyote time)
    bool canJump = (player.isGrounded || coyoteTimer > 0.0f);
    bool wantsToJump = jumpBufferTimer > 0.0f;
    
    if (wantsToJump && canJump) {
        player.velocity.y = Player::JUMP_VELOCITY;
        player.isGrounded = false;
        jumpBufferTimer = 0.0f;  // Consume the jump buffer
        coyoteTimer = 0.0f;       // Consume the coyote time
    }
    
}

void PlayerController::handleDropThrough(Player& player, IInput* input, float deltaTime) {
    // Drop through platforms by holding down
    if (input->isPressed(Button::Down) && player.isGrounded) {
        player.wantsToDropThrough = true;
        player.isGrounded = false;
        dropThroughTimer = DROP_THROUGH_TIME;
        // Give a small downward push to help drop through cleanly
        player.velocity.y = 50.0f;
    }
    
    // Keep drop-through active while down is held
    if (input->isPressed(Button::Down) && player.wantsToDropThrough) {
        dropThroughTimer = DROP_THROUGH_TIME;  // Keep timer refreshed
    }
    
    // Auto-reset drop-through flag after timer expires
    if (player.wantsToDropThrough) {
        dropThroughTimer -= deltaTime;
        if (dropThroughTimer <= 0.0f) {
            player.wantsToDropThrough = false;
            dropThroughTimer = 0.0f;
        }
    }
}