#pragma once
#include "core/Types.h"

class Level;

class Enemy {
public:
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 80.0f;
    static constexpr float GRAVITY = 980.0f;
    static constexpr float MAX_FALL_SPEED = 500.0f;
    
    Vec2 position;
    Vec2 velocity;
    int health;
    bool movingRight;
    
    Enemy(Vec2 startPos, bool startMovingRight = true);
    
    void update(float deltaTime, const Level& level);
    bool takeDamage(int amount);
    Rect getCollider() const;
    
private:
    void checkEdgeDetection(const Level& level);
    void checkWallCollision(const Level& level);
    void applyGravity(float deltaTime);
};
