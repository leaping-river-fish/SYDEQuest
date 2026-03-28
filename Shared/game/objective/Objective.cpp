#include "Objective.h"
#include "../../core/IRenderer.h"

Objective::Objective(Vec2 pos, ObjectiveType objType) 
    : position(pos), type(objType), collected(false) {
}

void Objective::render(IRenderer* renderer, int spritesheet, int camX, int camY) const {
    if (collected) {
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
        renderer->drawSpriteFrame(spritesheet, 0, 16, 16, dstRect, false);
    } else {
        renderer->drawRect(dstRect, Color(255, 255, 0), true);
    }
}

Rect Objective::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
