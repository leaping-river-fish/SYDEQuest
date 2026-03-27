#pragma once
#include "../../core/types.h"

class Player;

class Physics {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t GRAVITY = TO_FIXED(1200.0f);
    static constexpr fixed_t MAX_FALL_SPEED = TO_FIXED(600.0f);
    static constexpr fixed_t MAX_MOVE_SPEED = TO_FIXED(200.0f);
#else
    static constexpr float GRAVITY = 1200.0f;
    static constexpr float MAX_FALL_SPEED = 600.0f;
    static constexpr float MAX_MOVE_SPEED = 200.0f;
#endif
    
    void applyGravity(Player& player, float deltaTime);
    void clampVelocity(Player& player);
};

