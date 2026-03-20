#include "Objective.h"
#include "core/IRenderer.h"

Objective::Objective(Vec2 pos, ObjectiveType objType) 
    : position(pos), type(objType), collected(false) {
}

void Objective::render(IRenderer* renderer, int spritesheet, int camX, int camY) const {
    if (collected) {
        return;
    }
    
    Rect dstRect = getCollider();
    dstRect.x -= camX;
    dstRect.y -= camY;
    
    if (spritesheet >= 0) {
        Rect srcRect(0, 0, 16, 16);
        renderer->drawSprite(spritesheet, srcRect, dstRect, false);
    } else {
        renderer->drawRect(dstRect, Color(255, 255, 0), true);
    }
}

Rect Objective::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}
