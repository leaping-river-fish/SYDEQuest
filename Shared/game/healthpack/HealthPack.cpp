#include "HealthPack.h"
#include "../../core/IRenderer.h"

HealthPack::HealthPack(Vec2 pos)
    : position(pos), active(true), currentFrame(0), animationTimer(0.0f)
{
}

void HealthPack::update(float deltaTime) {
    if (!active) {
        return;
    }
    
    animationTimer += deltaTime;
    if (animationTimer >= ANIM_FRAME_DURATION_SEC) {
        animationTimer -= ANIM_FRAME_DURATION_SEC;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
}

void HealthPack::render(IRenderer* renderer, int spritesheet, int camX, int camY) const {
    if (!active) {
        return;
    }
    
    Rect dstRect = getCollider();
#ifdef PLATFORM_PICO
    dstRect.x -= TO_FIXED(camX);
    dstRect.y -= TO_FIXED(camY);
#else
    dstRect.x -= camX;
    dstRect.y -= camY;
#endif
    
    if (spritesheet >= 0) {
        renderer->drawSpriteFrame(spritesheet, currentFrame, 16, 16, dstRect, false);
    } else {
        renderer->drawRect(dstRect, Color(0, 255, 0), true);
    }
}

Rect HealthPack::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
