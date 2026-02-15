#pragma once
#include "core/Types.h"

class Player {
public:
    static constexpr float WIDTH = 14.0f;
    static constexpr float HEIGHT = 22.0f;
    static constexpr float MOVE_SPEED = 150.0f;
    static constexpr float JUMP_VELOCITY = -400.0f;
    
    Vec2 position;
    Vec2 velocity;
    
    bool isGrounded;
    bool wantsToDropThrough;  // For one-way platforms
    int health;
    
    Player();
    
    void update(float deltaTime);
    Rect getCollider() const;
};

