#pragma once
#include "../../core/types.h"

class IRenderer;

class HealthPack {
public:
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(16.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(16.0f);
#else
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
#endif
    static constexpr float ANIM_FRAME_DURATION_SEC = 0.1667f;
    static constexpr int TOTAL_FRAMES = 12;
    
    Vec2 position;
    bool active;
    
    int currentFrame;
    float animationTimer;
    
    HealthPack() : position(0.0f, 0.0f), active(false), currentFrame(0), animationTimer(0.0f) {}
    HealthPack(Vec2 pos);
    
    void update(float deltaTime);
    void render(IRenderer* renderer, int spritesheet, int camX, int camY) const;
    Rect getCollider() const;
};
