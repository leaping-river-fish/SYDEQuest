#pragma once
#include "../../core/types.h"

class Player {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(16.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(16.0f);
    static constexpr fixed_t MOVE_SPEED = TO_FIXED(150.0f);
    static constexpr fixed_t JUMP_VELOCITY = TO_FIXED(-450.0f);
    static constexpr fixed_t INVINCIBILITY_DURATION = TO_FIXED(1.0f);
#else
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 150.0f;
    static constexpr float JUMP_VELOCITY = -450.0f;
    static constexpr float INVINCIBILITY_DURATION = 1.0f;
#endif
    static constexpr float ANIM_FRAME_DURATION_SEC = 0.0556f;
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    Vec2 velocity;
    
    uint8_t health;
    float invincibilityTimer;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    // Flags packed into bitfield
    uint8_t flags;
    static constexpr uint8_t FLAG_GROUNDED = 1 << 0;
    static constexpr uint8_t FLAG_DROP_THROUGH = 1 << 1;
    static constexpr uint8_t FLAG_WANTS_FIRE = 1 << 2;
    static constexpr uint8_t FLAG_FACING_RIGHT = 1 << 3;
    
    bool isGrounded() const { return flags & FLAG_GROUNDED; }
    void setGrounded(bool value) { 
        if (value) flags |= FLAG_GROUNDED; 
        else flags &= ~FLAG_GROUNDED; 
    }
    
    bool wantsToDropThrough() const { return flags & FLAG_DROP_THROUGH; }
    void setWantsToDropThrough(bool value) { 
        if (value) flags |= FLAG_DROP_THROUGH; 
        else flags &= ~FLAG_DROP_THROUGH; 
    }
    
    bool wantsToFire() const { return flags & FLAG_WANTS_FIRE; }
    void setWantsToFire(bool value) { 
        if (value) flags |= FLAG_WANTS_FIRE; 
        else flags &= ~FLAG_WANTS_FIRE; 
    }
    
    bool facingRight() const { return flags & FLAG_FACING_RIGHT; }
    void setFacingRight(bool value) { 
        if (value) flags |= FLAG_FACING_RIGHT; 
        else flags &= ~FLAG_FACING_RIGHT; 
    }
    
    Player();
    
    void update(float deltaTime);
    Rect getCollider() const;
};

