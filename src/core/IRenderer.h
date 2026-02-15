#pragma once
#include "Types.h"

class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void clear(Color color) = 0;
    
    virtual void drawRect(const Rect& rect, Color color, bool filled) = 0;
    virtual void drawTile(int tileX, int tileY, int tileType,
                          int cameraOffsetX, int cameraOffsetY) = 0;
    
    virtual int getScreenWidth() const = 0;
    virtual int getScreenHeight() const = 0;
};

