#pragma once

class Player;
class IInput;

class PlayerController {
public:
    PlayerController();
    void update(Player& player, IInput* input, float deltaTime);
    
private:
    void handleMovement(Player& player, IInput* input);
    void handleJump(Player& player, IInput* input, float deltaTime);
    void handleDropThrough(Player& player, IInput* input, float deltaTime);
    void handleFiring(Player& player, IInput* input, float deltaTime);
    
    // Jump feel improvements
    float jumpBufferTimer;
    float coyoteTimer;
    bool wasGroundedLastFrame;
    float dropThroughTimer;
    
    // Firing
    float fireCooldownTimer;
    
    static constexpr float JUMP_BUFFER_TIME = 0.1f;  // 100ms buffer for jump input
    static constexpr float COYOTE_TIME = 0.12f;      // 120ms grace period after leaving ground
    static constexpr float DROP_THROUGH_TIME = 0.2f; // 200ms to pass through platform
    static constexpr float FIRE_COOLDOWN = 0.2f;     // 200ms between shots
};

