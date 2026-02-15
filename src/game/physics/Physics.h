#pragma once

class Player;

class Physics {
public:
    static constexpr float GRAVITY = 1200.0f;
    static constexpr float MAX_FALL_SPEED = 600.0f;
    static constexpr float MAX_MOVE_SPEED = 200.0f;
    
    void applyGravity(Player& player, float deltaTime);
    void clampVelocity(Player& player);
};

