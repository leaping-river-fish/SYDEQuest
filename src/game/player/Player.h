#pragma once
#include "core/Types.h"

class Player {
public:
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 150.0f;
    static constexpr float JUMP_VELOCITY = -437.0f;  // Jump height: exactly 5 tiles (80 pixels)
    static constexpr float FRAME_TIME = 0.0556f;  // 18 FPS animation
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    
    bool isGrounded;
    bool wantsToDropThrough;  // For one-way platforms
    int health;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    bool facingRight;
    
    Player();
    
    void update(float deltaTime);
    Rect getCollider() const;
};

