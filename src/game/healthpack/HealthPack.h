#pragma once
#include "core/Types.h"

class IRenderer;

class HealthPack {
public:
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    static constexpr float FRAME_TIME = 0.1667f;  // 6 FPS animation
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    bool active;
    
    int currentFrame;
    float animationTimer;
    
    HealthPack(Vec2 pos);
    
    void update(float deltaTime);
    void render(IRenderer* renderer, int spritesheet, int camX, int camY) const;
    Rect getCollider() const;
};
