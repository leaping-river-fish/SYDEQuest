#pragma once
#include "../../core/types.h"

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
#ifdef PLATFORM_PICO
    static constexpr fixed_t WIDTH = TO_FIXED(16.0f);
    static constexpr fixed_t HEIGHT = TO_FIXED(16.0f);
#else
    static constexpr float WIDTH = 16.0f;
    static constexpr float HEIGHT = 16.0f;
#endif
    
    Vec2 position;
    ObjectiveType type;
    bool collected;
    
    Objective() : position(0.0f, 0.0f), type(ObjectiveType::PICO), collected(false) {}
    Objective(Vec2 pos, ObjectiveType objType);
    
    void render(IRenderer* renderer, int spritesheet, int camX, int camY) const;
    Rect getCollider() const;
};
