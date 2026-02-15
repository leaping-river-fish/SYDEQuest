#include "PlayerController.h"
#include "Player.h"
#include "core/IInput.h"

void PlayerController::update(Player& player, IInput* input) {
    handleMovement(player, input);
    handleJump(player, input);
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

void PlayerController::handleJump(Player& player, IInput* input) {
    // Jump
    if (input->wasJustPressed(Button::Jump) && player.isGrounded) {
        player.velocity.y = Player::JUMP_VELOCITY;
        player.isGrounded = false;
    }
    
    // Drop through platforms
    if (input->isPressed(Button::Down) && input->wasJustPressed(Button::Jump)) {
        if (player.isGrounded) {
            player.wantsToDropThrough = true;
            player.isGrounded = false;
        }
    }
}