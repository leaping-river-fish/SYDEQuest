#pragma once
#include "core/Types.h"
#include "EnemyProjectile.h"
#include <vector>

class Level;
class Player;

class RangedEnemy {
public:
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float MOVE_SPEED = 50.0f;
    static constexpr float GRAVITY = 980.0f;
    static constexpr float MAX_FALL_SPEED = 500.0f;
    static constexpr float FRAME_TIME = 0.0833f;  // 12 FPS animation
    static constexpr int TOTAL_FRAMES = 12;
    static constexpr float SHOOT_INTERVAL = 1.0f;  // 1 second between shots
    static constexpr float DETECTION_RADIUS = 160.0f;  // 10 tiles * 16px
    static constexpr float SHOOT_PAUSE_TIME = 0.3f;  // Pause time when shooting
    
    Vec2 position;
    Vec2 velocity;
    int health;
    bool facingRight;
    
    // Animation state
    int currentFrame;
    float animationTimer;
    
    // Shooting state
    float shootTimer;
    float shootPauseTimer;
    bool isShooting;
    
    RangedEnemy(Vec2 startPos, bool startFacingRight = true);
    
    void update(float deltaTime, const Level& level, const Player& player, std::vector<EnemyProjectile>& projectiles);
    bool takeDamage(int amount);
    Rect getCollider() const;
    
private:
    void checkEdgeDetection(const Level& level);
    void checkWallCollision(const Level& level);
    void applyGravity(float deltaTime);
    bool hasLineOfSight(Vec2 targetPos, const Level& level) const;
    bool canDetectPlayer(const Player& player, const Level& level) const;
};
