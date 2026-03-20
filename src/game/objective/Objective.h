#pragma once
#include "core/Types.h"

class IRenderer;

enum class ObjectiveType {
    CHARGER,
    ENCLOSURE,
    HAPTIC,
    PARTS,
    SCREEN,
    PICO
};

class Objective {
public:
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
    
    Vec2 position;
    ObjectiveType type;
    bool collected;
    
    Objective(Vec2 pos, ObjectiveType objType);
    
    void render(IRenderer* renderer, int spritesheet, int camX, int camY) const;
    Rect getCollider() const;
};
