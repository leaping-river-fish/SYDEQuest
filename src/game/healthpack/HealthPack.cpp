#include "HealthPack.h"
#include "core/IRenderer.h"

HealthPack::HealthPack(Vec2 pos)
    : position(pos), active(true), currentFrame(0), animationTimer(0.0f)
{
}

void HealthPack::update(float deltaTime) {
    if (!active) {
        return;
    }
    
    animationTimer += deltaTime;
    if (animationTimer >= FRAME_TIME) {
        animationTimer -= FRAME_TIME;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
    }
}

void HealthPack::render(IRenderer* renderer, int spritesheet, int camX, int camY) const {
    if (!active) {
        return;
    }
    
    Rect dstRect = getCollider();
    dstRect.x -= camX;
    dstRect.y -= camY;
    
    if (spritesheet >= 0) {
        int frameX = (currentFrame % 4) * 16;
        int frameY = (currentFrame / 4) * 16;
        Rect srcRect(frameX, frameY, 16, 16);
        
        renderer->drawSprite(spritesheet, srcRect, dstRect, false);
    } else {
        renderer->drawRect(dstRect, Color(0, 255, 0), true);
    }
}

Rect HealthPack::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
